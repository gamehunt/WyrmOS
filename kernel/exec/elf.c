#include "dev/log.h"
#include "exec/module.h"
#include "symbols.h"
#include <assert.h>
#include <exec/elf.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int k_elf_exec(void* file, int argc, const char** argv, const char** envp) {
	// TODO
	return -1;
}

EXPORT(k_elf_check)
EXPORT(k_elf_load_module)
EXPORT(k_elf_exec)