#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <libelf.h>
#include <gelf.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdint.h>

char *get_sym_type(int type)
{
	switch (type) {
		case STT_NOTYPE:
			return "NOTYPE";
		case STT_OBJECT:
			return "OBJECT";
		case STT_FUNC:
			return "FUNC";
		case STT_SECTION:
			return "SECTION";
		case STT_FILE:
			return "FILE";
		case STT_COMMON:
			return "COMMON";
		case STT_TLS:
			return "TLS";
	}
	return NULL;
}

char *get_sym_bind(int bind)
{
	switch (bind) {
		case STB_LOCAL:
			return "LOCAL";
		case STB_GLOBAL:
			return "GLOBAL";
		case STB_WEAK:
			return "WEAK";
	}
	return NULL;
}

char *get_sym_visibility(int vis)
{
	switch (vis) {
		case STV_DEFAULT:
			return "DEFAULT";
		case STV_INTERNAL:
			return "INTERNAL";
		case STV_HIDDEN:
			return "HIDDEN";
		case STV_PROTECTED:	
			return "PROTECTED";
	}
	return NULL;
}

char *get_sym_ndx(int ndx, char *buf)
{
	if (ndx == SHN_UNDEF) {
		return "UNDEF";
	} else if (ndx == SHN_ABS) {
		return "ABS";
	} else if (ndx == SHN_COMMON) {
		return "COMMON";
	} else {
		sprintf(buf, "%d", ndx);
		return buf; 
	}	
	return NULL;
}

int main(int argc, const char *argv[])
{
	int filedes, n = 0;
	Elf *pelf = NULL;
	Elf_Scn *pelf_scn = NULL;
	GElf_Shdr shdr;
	Elf_Data *pdata = NULL;
	GElf_Sym sym;
	
	if (argc != 2) {
		errx(EXIT_FAILURE, "Usgae: %s file-name.\n", argv[0]);
	}

	if (elf_version(EV_CURRENT) == EV_NONE) {
		errx(EXIT_FAILURE, "ELF library initialization failed: %s\n", elf_errmsg(-1));
	}

	if ((filedes = open(argv[1], O_RDONLY, 0)) < 0) {
		errx(EXIT_FAILURE, "open file %s error.\n", argv[1]);
	}
		
	if ((pelf = elf_begin(filedes, ELF_C_READ, NULL)) == NULL) {
		close(filedes);
		errx(EXIT_FAILURE, "elf_begin failed: %s\n", elf_errmsg(-1));
	}

	if (elf_kind(pelf) != ELF_K_ELF) {
		elf_end(pelf);
		close(filedes);
		errx(EXIT_FAILURE, "%s is not a ELF file.\n", argv[1]);
	}

	while ((pelf_scn = elf_nextscn(pelf, pelf_scn)) != NULL) {
		if (gelf_getshdr(pelf_scn, &shdr) != &shdr) {
			elf_end(pelf);	close(filedes);
			errx(EXIT_FAILURE, "gelf_getshdr failed: %s\n", elf_errmsg(-1));
		}
		/* 判断节的类型 */
		if (shdr.sh_type == SHT_SYMTAB || shdr.sh_type == SHT_DYNSYM) {
			size_t str_sec_idx = shdr.sh_link; 	/* 相关联的字符串表的节头表索引值 */
			int sym_num = shdr.sh_size / shdr.sh_entsize; 	/* 符号表表项个数 */	
			
			if (shdr.sh_type == SHT_SYMTAB)
				printf("Symbol table '.symtab' contains %d entries:\n", sym_num);	
			else 
				printf("Symbol table '.dynsym' contains %d entries:\n", sym_num);	
			printf("\tNum:   	Value 	Size	Type   	Bind  	Vis     Ndx	Name\n");

			char buf[32] = { 0 };
			pdata = elf_getdata(pelf_scn, pdata);
			while (n < sym_num) {
				if (gelf_getsym(pdata, n, &sym) != &sym) {
					elf_end(pelf);	close(filedes);
					errx(EXIT_FAILURE, "gelf_getsym failed: %s\n", elf_errmsg(-1));
				}
				printf("\t%02d: %08x	%04d	%s	%s	%s	%s	%s\n", n, (uint32_t)sym.st_value, (uint32_t)sym.st_size, 
															get_sym_type(GELF_ST_TYPE(sym.st_info)), 
															get_sym_bind(GELF_ST_BIND(sym.st_info)), 
															get_sym_visibility(ELF64_ST_VISIBILITY(sym.st_other)), 
															get_sym_ndx(sym.st_shndx, buf), 
															elf_strptr(pelf, str_sec_idx, (size_t)sym.st_name));
				n++;
			}
		}
   	}

	close(filedes);
	exit(EXIT_SUCCESS);
}
