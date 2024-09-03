#ifndef __K_EXEC_ELF_H
#define __K_EXEC_ELF_H 1

#define ELF_MAGIC 0x464c457f

#define ELF_CLASSNONE 0
#define ELF_CLASS32   1
#define ELF_CLASS64   2

#define ELFDATANONE   0
#define ELFDATA2LSB   1
#define ELFDATA2MSB   2

#define EV_NONE    0
#define EV_CURRENT 1

#define ELFOSABI_NONE 0

#define ET_NONE	0
#define ET_REL	1
#define ET_EXEC	2
#define ET_DYN	3
#define ET_CORE	4

#define EM_NONE   0x00
#define EM_386    0x03
#define EM_860    0x07
#define EM_960    0x13
#define EM_X86_64 0x3E

#include <stdint.h>
#include <exec/module.h>

typedef struct {
	uint32_t magic;
	uint8_t  class;
	uint8_t  data;
	uint8_t  version;
	uint8_t  osabi;
	uint8_t  osabi_version;
	uint8_t  pad[7];
} elf_ident;

typedef struct {
	elf_ident e_ident;
	uint16_t  e_type;
	uint16_t  e_machine;
	uint32_t  e_version;
	uint32_t  e_entry;
	uint32_t  e_phoff;
	uint32_t  e_shoff;
	uint32_t  e_flags;
	uint16_t  e_hsize;
	uint16_t  e_phentsize;
	uint16_t  e_phnum;
	uint16_t  e_shentsize;
	uint16_t  e_shnum;
	uint16_t  e_shstrndx;
} elf32;

typedef struct {
	elf_ident e_ident;
	uint16_t  e_type;
	uint16_t  e_machine;
	uint32_t  e_version;
	uint64_t  e_entry;
	uint64_t  e_phoff;
	uint64_t  e_shoff;
	uint32_t  e_flags;
	uint16_t  e_hsize;
	uint16_t  e_phentsize;
	uint16_t  e_phnum;
	uint16_t  e_shentsize;
	uint16_t  e_shnum;
	uint16_t  e_shstrndx;
} elf64;

#define PT_NULL     0
#define PT_LOAD     1
#define PT_DYNAMIC	2
#define PT_INTERP	3
#define PT_NOTE	    4
#define PT_SHLIB    5
#define PT_PHDR	    6
#define PT_TLS	    7

typedef struct {
	uint32_t  p_type;
	uint32_t  p_offset;
	uint32_t  p_vaddr;
	uint32_t  p_paddr;
	uint32_t  p_filesz;
	uint32_t  p_memsz;
	uint32_t  p_flags;
	uint32_t  p_align;
} phdr32;

typedef struct {
	uint32_t  p_type;
	uint32_t  p_flags;
	uint64_t  p_offset;
	uint64_t  p_vaddr;
	uint64_t  p_paddr;
	uint64_t  p_filesz;
	uint64_t  p_memsz;
	uint64_t  p_align;
} phdr64;

#define SHT_NULL	 0
#define SHT_PROGBITS 1
#define SHT_SYMTAB	 2
#define SHT_STRTAB	 3
#define SHT_RELA	 4
#define SHT_HASH	 5
#define SHT_DYNAMIC	 6
#define SHT_NOTE	 7
#define SHT_NOBITS	 8
#define SHT_REL	     9
#define SHT_SHLIB	10
#define SHT_DYNSYM	11
#define SHT_INIT_ARRAY    14
#define SHT_FINI_ARRAY    15
#define SHT_PREINIT_ARRAY 16
#define SHT_GROUP         17 
#define SHT_SYMTAB_SHNDX  18

#define SHF_WRITE	         0x1
#define SHF_ALLOC	         0x2	
#define SHF_EXECINSTR	     0x4	
#define SHF_MERGE	         0x10	
#define SHF_STRINGS	         0x20 
#define SHF_INFO_LINK	     0x40 
#define SHF_LINK_ORDER	     0x80	
#define SHF_OS_NONCONFORMING 0x100
#define SHF_GROUP            0x200
#define SHF_TLS	             0x400
#define SHF_MASKOS           0x0FF00000
#define SHF_MASKPROC         0xF0000000
 
#define PF_X        0x1	        
#define PF_W        0x2	        
#define PF_R        0x4	        
#define PF_MASKOS	0x0ff00000
#define PF_MASKPROC	0xf0000000 

#define STB_LOCAL	0		/* Local symbol */
#define STB_GLOBAL	1		/* Global symbol */
#define STB_WEAK	2		/* Weak symbol */
#define	STB_NUM		3		/* Number of defined types.  */
#define STB_LOOS	10		/* Start of OS-specific */
#define STB_GNU_UNIQUE	10		/* Unique symbol.  */
#define STB_HIOS	12		/* End of OS-specific */
#define STB_LOPROC	13		/* Start of processor-specific */
#define STB_HIPROC	15		/* End of processor-specific */

typedef struct {
	uint32_t sh_name;
	uint32_t sh_type;
	uint32_t sh_flags;
	uint32_t sh_addr;
	uint32_t sh_offset;
	uint32_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint32_t sh_addralign;
	uint32_t sh_entsize;
} shdr32;

typedef struct {
	uint32_t sh_name;
	uint32_t sh_type;
	uint64_t sh_flags;
	uint64_t sh_addr;
	uint64_t sh_offset;
	uint64_t sh_size;
	uint32_t sh_link;
	uint32_t sh_info;
	uint64_t sh_addralign;
	uint64_t sh_entsize;
} shdr64;

typedef struct {
  	uint32_t    st_name;
  	uint8_t     st_info;	
  	uint8_t     st_other;	
  	uint16_t    st_shndx;	
  	uint64_t    st_value;	
  	uint64_t	st_size;	
} sym64;

#define ELF32_ST_BIND(val)		(((unsigned char) (val)) >> 4)
#define ELF32_ST_TYPE(val)		((val) & 0xf)
#define ELF32_ST_INFO(bind, type)	(((bind) << 4) + ((type) & 0xf))

#define ELF64_ST_BIND(val)		ELF32_ST_BIND (val)
#define ELF64_ST_TYPE(val)		ELF32_ST_TYPE (val)
#define ELF64_ST_INFO(bind, type)	ELF32_ST_INFO ((bind), (type))

#define ELF32_R_SYM(i)   ((i)>>8)
#define ELF32_R_TYPE(i)   ((unsigned char) (i))
#define ELF32_R_INFO(s,t) (((s)<<8)+(unsigned char)(t)) 

#define ELF64_R_SYM(info)        ((info)>>32)
#define ELF64_R_TYPE(info)       ((uint16_t)(info))
#define ELF64_R_INFO(sym, type)  (((uint64_t)(sym)<<32)+(uint64_t)(type))

#define R_X86_64_NONE      0
#define R_X86_64_64        1
#define R_X86_64_PC32      2
#define R_X86_64_GOT32     3
#define R_X86_64_PLT32     4
#define R_X86_64_COPY      5
#define R_X86_64_GLOB_DAT  6
#define R_X86_64_JUMP_SLOT 7
#define R_X86_64_RELATIVE  8
#define R_X86_64_GOTPCREL  9
#define R_X86_64_32       10
#define R_X86_64_32S      11
#define R_X86_64_16       12
#define R_X86_64_PC16     13
#define R_X86_64_8        14
#define R_X86_64_PC8      15
#define R_X86_64_PC64     24
#define R_X86_64_GOTOFF64 25
#define R_X86_64_GOTPC32  26
#define R_X86_64_SIZE32   32
#define R_X86_64_SIZE64   33

#define SHN_UNDEF     0x0000
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC    0xff00
#define SHN_BEFORE    0xff00
#define SHN_AFTER     0xff01
#define SHN_HIPROC    0xff1f
#define SHN_ABS       0xfff1
#define SHN_COMMON    0xfff2
#define SHN_HIRESERVE 0xffff

typedef struct
{
   uint64_t r_offset;
   uint64_t r_info;	
} rel64;

typedef struct
{
   uint64_t r_offset;
   uint64_t r_info;	
   int64_t  r_addend;
} rela64;

uint8_t             k_elf_check(void* elf);
struct module_info* k_elf_load_module(void* elf);
void                k_elf_exec(void* file);

#endif
