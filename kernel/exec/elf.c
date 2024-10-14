#include <arch.h>
#include "dev/log.h"
#include "exec/module.h"
#include "fcntl.h"
#include "fs/fs.h"
#include "mem/mem.h"
#include "mem/paging.h"
#include "proc/process.h"
#include "symbols.h"
#include <assert.h>
#include <exec/elf.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util.h>

uint8_t k_elf_check(void* elf) {
    elf_ident* ident = elf;
    if(ident->magic != ELF_MAGIC      || 
        ident->osabi != ELFOSABI_NONE || 
        ident->data != ELFDATA2LSB    ||
        ident->version != EV_CURRENT  ||
        ident->class == ELF_CLASSNONE) {
        return 0;
    }   
    return ident->class;
}


static shdr64* __k_elf_section(elf64* elf, size_t index) {
    assert(index < elf->e_shnum);
    return (void*) elf + elf->e_shoff + index * elf->e_shentsize;
}

#define __k_elf_shstrtab(elf) __k_elf_section(elf, elf->e_shstrndx)

static void* __k_elf_section_data(elf64* file, shdr64* section, size_t offset) {
    return (void*) file + section->sh_offset + offset;
}

static void* __k_elf_section_index(elf64* file, shdr64* section, size_t index) {
    return __k_elf_section_data(file, section, index * section->sh_entsize);
}

static const char* __k_elf_section_name(elf64* file, shdr64* section) {
    shdr64* shstrtab = __k_elf_shstrtab(file);
    return __k_elf_section_data(file, shstrtab, section->sh_name);
}

static shdr64* __k_elf_find_section(elf64* elf, const char* target) {
    for(size_t i = 0; i < elf->e_shnum; i++) {
        shdr64* section = __k_elf_section(elf, i);
        if(!strcmp(__k_elf_section_name(elf, section), target)) {
            return section;
        }
    }
    return NULL;
}

static shdr64* __k_elf_find_section_by_type(elf64* elf, uint32_t type) {
    for(size_t i = 0; i < elf->e_shnum; i++) {
        shdr64* section = __k_elf_section(elf, i);
        if(section->sh_type == type) {
            return section;
        }
    }
    return NULL;
}

#define __k_elf_symtab(elf) __k_elf_find_section_by_type(elf, SHT_SYMTAB)

#define __k_elf_section_entries(sect) ((sect)->sh_size / (sect)->sh_entsize)

static sym64* __k_elf_find_symbol(elf64* elf, const char* symbol) {
    shdr64* symtab  = __k_elf_symtab(elf);
    shdr64* strtab  = __k_elf_section(elf, symtab->sh_link);
    for(size_t i = 0; i < __k_elf_section_entries(symtab); i++) {
        sym64* s    = __k_elf_section_index(elf, symtab, i);
        const char* name = __k_elf_section_data(elf, strtab, s->st_name); 
        if(!strcmp(name, symbol)) {
            return s;
        }
    }
    return NULL;
}

static uintptr_t __k_elf_symval(elf64* file, uintptr_t table, size_t index) {
    shdr64* symtab = __k_elf_section(file, table);
    sym64*  sym    = __k_elf_section_index(file, symtab, index);
    if(sym->st_shndx == SHN_UNDEF) {
        shdr64* strtab  = __k_elf_section(file, symtab->sh_link);
        const char* name = __k_elf_section_data(file, strtab, sym->st_name); 
        symbol* kernel_symbol = k_lookup_symbol(name);
        if(!kernel_symbol) {
            if(ELF64_ST_BIND(sym->st_info) & STB_WEAK) {
                return 0;
            } else {
                k_error("Unresolved external symbol: %s", name);
                return 0xFFFFffffFFFFffff;
            }
        } else {
            return kernel_symbol->address;
        }
    } else if(sym->st_shndx == SHN_ABS) {
        return sym->st_value;
    } else {
        shdr64* sect = __k_elf_section(file, sym->st_shndx);
        return (uintptr_t) __k_elf_section_data(file, sect, sym->st_value); 
    }
}

static uint8_t __k_elf_relocate(elf64* file, shdr64* sect, rela64* rela) {
    shdr64*    target = __k_elf_section(file, sect->sh_info);
    uintptr_t* addr   = __k_elf_section_data(file, target, rela->r_offset);
    uintptr_t  symval  = 0;
    if(ELF64_R_SYM(rela->r_info) != SHN_UNDEF) {
        symval = __k_elf_symval(file, sect->sh_link, ELF64_R_SYM(rela->r_info));
        if(symval == 0xFFFFffffFFFFffff) {
            return 0;
        }
    }
    switch(ELF64_R_TYPE(rela->r_info)) {
        case R_X86_64_NONE:
            break;
        case R_X86_64_64:
            *addr = symval + rela->r_addend;
            break;
        case R_X86_64_PC32:
            *(uint32_t*)addr = symval + rela->r_addend - (uintptr_t) addr;
            break;
        case R_X86_64_32:
            *(uint32_t*)addr = symval + rela->r_addend; 
            break;
        default:
            k_error("Unknown relocation type: %d", ELF64_R_TYPE(rela->r_info));
            return 0;
    }
    return 1;
}

struct module_info* k_elf_load_module(void* elf) {
    uint8_t version = k_elf_check(elf);
    if(version != ELF_CLASS64) {
        k_error("Invalid e_ident.");
        return NULL;
    }

    elf64* header = elf;

    if(header->e_type != ET_REL) {
        k_error("Not relocatable.");
        return NULL;
    }

    for(int i = 0; i < header->e_shnum; i++) {
        shdr64* section = __k_elf_section(elf, i);
        if(section->sh_type == SHT_NOBITS) {
            if(!section->sh_size) {
                continue;
            }
            if(section->sh_flags & SHF_ALLOC) {
                void* mem = malloc(section->sh_size);
                memset(mem, 0, section->sh_size);
                section->sh_offset = (uintptr_t) mem - (uintptr_t) elf;
                k_debug("Allocated memory for section: %d", section->sh_size);
            }
        } else if (section->sh_type == SHT_REL || section->sh_type == SHT_RELA) {
            for(size_t j = 0; j < __k_elf_section_entries(section); j++) {
                rela64 rel;
                if(section->sh_type == SHT_RELA) {
                    rel = *(rela64*)(__k_elf_section_index(elf, section, j));
                } else {
                    rel64* _rel = __k_elf_section_index(elf, section, j);
                    rel.r_offset = _rel->r_offset;
                    rel.r_info   = _rel->r_info;
                    rel.r_addend = 0;
                }
                if(!__k_elf_relocate(header, section, &rel)) {
                    k_error("Relocation failed.");
                    return NULL;
                }
            }
        }
    }

    k_debug("Sections loaded.");

    sym64* modinfo = __k_elf_find_symbol(elf, "__module_info");
    struct module_info* module = NULL;

    if(modinfo) {
        shdr64* shdr = __k_elf_section(elf, modinfo->st_shndx);
        module = __k_elf_section_data(elf, shdr, modinfo->st_value);
    }

    return module;
}

#undef  k_debug
#define k_debug(fmt, ...) 
extern void __attribute__((noreturn)) __usr_jmp(uintptr_t entry, uintptr_t stack);
int k_elf_exec(const char* path, int argc, const char** argv, char** envp) {
    fs_node* exec = k_fs_open(path, O_RDONLY);
    if(!exec) {
        k_error("No such file: %s", path);
        return -1;
    }

    void* elf = malloc(exec->size);
    size_t r = k_fs_read(exec, 0, exec->size, elf);
    k_fs_close(exec);
    if(r < exec->size) {
        k_error("Failed to read: %s", path);
        free(elf);
        return -1;
    }

    uint8_t version = k_elf_check(elf);
    if(version != ELF_CLASS64) {
        k_error("Invalid e_ident: %d", version);
        return -1;
    }

    elf64* header = elf;
    if(header->e_type != ET_EXEC) {
        k_error("Not executable.");
        return -1;
    }

    int envc = 0;
    if(envp) {
        while(*(envp + envc)) {
            envc++;
        }
    }

    char** _argv_copy = malloc(sizeof(char*) * (argc + 1));
    char** _envp_copy = malloc(sizeof(char*) * (envc + 1));

    for(int i = 0; i < argc; i++) {
        _argv_copy[i] = strdup(argv[i]);
    }
    _argv_copy[argc] = NULL;

    for(int i = 0; i < envc; i++) {
        _envp_copy[i] = strdup(envp[i]);
    }
    _envp_copy[envc] = NULL;

    union page* cur = current_core->current_process->ctx.pml;
    current_core->current_process->ctx.pml = k_mem_paging_clone_pml(NULL);
    k_mem_paging_set_pml(current_core->current_process->ctx.pml);
    k_mem_paging_free_pml(cur);

    phdr64*   phdr = elf + header->e_phoff;
    uintptr_t exec_end = 0;
    for(size_t i = 0; i < header->e_phnum; i++) {
        switch(phdr->p_type) {
            case PT_LOAD:
            k_debug("Allocating section: %#.16lx - %#.16lx", phdr->p_vaddr, phdr->p_vaddr + phdr->p_memsz);
            uintptr_t start = 0;
            size_t pages = 0;
            if(phdr->p_vaddr % PAGE_SIZE) {
                start = phdr->p_vaddr & ~(PAGE_SIZE - 1);
                pages = PAGES(phdr->p_memsz + (phdr->p_vaddr & (PAGE_SIZE - 1)));
            } else {
                start = phdr->p_vaddr;
                pages = PAGES(phdr->p_memsz);
            }
            uintptr_t end = start + pages * PAGE_SIZE;
            k_debug("Aligning to: %#.16lx - %#.16lx", start, end);
            k_mem_paging_map_pages_ex(start, pages, 0, PM_FL_USER | PM_FL_WRITABLE); 
            if(end > exec_end) {
                exec_end = end;
            }
            memset((void*) phdr->p_vaddr, 0, phdr->p_memsz);
            memcpy((void*) phdr->p_vaddr, elf + phdr->p_offset, phdr->p_filesz);
            break;
        }
        phdr = ((void*) phdr) + header->e_phentsize;
    }

    free(elf);

    k_mem_paging_map_pages_ex(exec_end, PAGES(USER_STACK_SIZE), 0, PM_FL_USER | PM_FL_WRITABLE);

    k_debug("Allocated %dB user stack at %#.16lx", USER_STACK_SIZE, exec_end);
    k_debug("Entry: %#.16lx", header->e_entry);

    uintptr_t user_stack = exec_end + USER_STACK_SIZE;

    const char** argv_pointer;
    char* tmp[argc + 1];
    tmp[argc] = NULL;
    for(int i = argc - 1; i >= 0; i--) {
        PUSH(user_stack, char, '\0');
        for(int j = strlen(_argv_copy[i]) - 1; j >= 0; j--) {
            PUSH(user_stack, char, _argv_copy[i][j]);
        }
        tmp[i] = (char*) user_stack;
    }
    for(int i = argc; i >= 0; i--) {
        PUSH(user_stack, char*, tmp[i]);
    }
    argv_pointer = (const char**) user_stack;

    char** envp_pointer;

    char* tmp_env[envc + 1];
    tmp_env[envc] = NULL;
    if(envc) {
        for(int i = envc - 1; i >= 0; i--) {
            PUSH(user_stack, char, '\0');
            for(int j = strlen(_envp_copy[i]) - 1; j >= 0; j--) {
                PUSH(user_stack, char, _envp_copy[i][j]);
            }
            tmp_env[i] = (char*) user_stack;
        }
    }
    for(int i = envc; i >= 0; i--) {
        PUSH(user_stack, char*, tmp_env[i]);
    }
    envp_pointer = (char**) user_stack;

    for(int i = 0 ; i < argc; i++) {
        free(_argv_copy[i]);
    }
    free(_argv_copy);
    for(int i = 0; i < envc; i++) {
        free(_envp_copy[i]);
    }
    free(_envp_copy);

    user_stack = ALIGN(user_stack, 16);
    
    k_mem_set_kernel_stack((uintptr_t) current_core->current_process->ctx.kernel_stack);
    arch_user_jmp_exec(argc, argv_pointer, envp_pointer, header->e_entry, user_stack);

    return -1;
}
