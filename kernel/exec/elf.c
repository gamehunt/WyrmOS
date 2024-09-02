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

static shdr64* __k_elf_find_section(elf64* elf, const char* section) {
	shdr64*	sheader  = (void*) elf + elf->e_shoff;
	shdr64* shstrtab = &sheader[elf->e_shstrndx];
	for(size_t i = 0; i < elf->e_shnum; i++) {
		if(!strcmp((void*) elf + shstrtab->sh_offset + sheader[i].sh_name, section)) {
			return &sheader[i];
		}
	}
	return NULL;
}

static sym64* __k_elf_find_symbol(elf64* elf, const char* symbol) {
	shdr64*	sheader = (void*) elf + elf->e_shoff;
	shdr64* symtab  = __k_elf_find_section(elf, ".symtab");
	for(size_t i = 0; i < symtab->sh_size / symtab->sh_entsize; i++) {
		sym64* s    = (void*) elf + symtab->sh_offset + i * symtab->sh_entsize;
		char*  name = (void*) elf + sheader[symtab->sh_link].sh_offset + s->st_name; 
		if(!strcmp(name, symbol)) {
			return s;
		}
	}
	return NULL;
}

static uintptr_t __k_elf_symval(elf64* file, uintptr_t table, uintptr_t index) {
	shdr64* symtab = (void*) file + file->e_shoff + table * file->e_shentsize;
	sym64*  sym = (void*) file + symtab->sh_offset + index * symtab->sh_entsize;
	if(sym->st_shndx == SHN_UNDEF) {
		shdr64* strtab = (void*) file + file->e_shoff + symtab->sh_link * file->e_shentsize;	
		const char* name = (void*) file + strtab->sh_offset + sym->st_name;

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
		shdr64* sect = (void*) file + file->e_shoff + sym->st_shndx * file->e_shentsize;
		return (uintptr_t) file + sect->sh_offset + sym->st_value; 
	}
}

static uint8_t __k_elf_relocate(elf64* file, shdr64* sect, void* rel) {
	shdr64*    target = (void*) file + file->e_shoff + file->e_shentsize * sect->sh_info;
	rela64*    rela   = (rela64*) rel;
	uintptr_t* addr   = (void*) file + target->sh_offset + rela->r_offset;
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
		shdr64* section = elf + header->e_shoff + i * header->e_shentsize;
		if(section->sh_type == SHT_NOBITS) {
			if(!section->sh_size) {
				continue;
			}
			if(section->sh_flags & SHF_ALLOC) {
				void* mem = malloc(section->sh_size);
				memset(mem, 0, section->sh_size);
				section->sh_offset = mem - elf;
				k_debug("Allocated memory for section: %d", section->sh_size);
			}
		} else if (section->sh_type == SHT_REL || section->sh_type == SHT_RELA) {
			k_debug("Started reloc.");
			for(size_t j = 0; j < section->sh_size / section->sh_entsize; j++) {
				rela64* rel = elf + section->sh_offset + j * section->sh_entsize;
				if(!__k_elf_relocate(header, section, rel)) {
					k_error("Relocation failed.");
					return NULL;
				}
			}
			k_debug("Finished reloc.");
		}
	}

	k_debug("Sections loaded.");

	sym64* modinfo = __k_elf_find_symbol(elf, "__module_info");
	struct module_info* module = NULL;

	if(modinfo) {
		shdr64* shdr = elf + header->e_shoff + header->e_shentsize * modinfo->st_shndx;
		module = elf + shdr->sh_offset + modinfo->st_value;
	}

	return module;
}

EXPORT(k_elf_check)
EXPORT(k_elf_load_module)
