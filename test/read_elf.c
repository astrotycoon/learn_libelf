#include <err.h>
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *get_file_type(Elf32_Half e_type)
{
	if (e_type == ET_REL)
		return "REL (Relocatable file)";
	else if (e_type == ET_EXEC)
		return "EXEC (Executable file)";
	else if (e_type == ET_DYN)
		return "DYN (Shared object file)";
}

const char *get_program_header_type(Elf32_Word p_type)
{
	if (p_type == PT_LOAD)
		return "LLOAD";
	else if (p_type == PT_DYNAMIC)
		return "NAMIC";
	else if (p_type == PT_INTERP)
		return "NTERP";
	else if (p_type == PT_NOTE)
		return "NNOTE";
	else if (p_type == PT_SHLIB)
		return "SHLIB";
	else if (p_type == PT_PHDR)
		return "PPHDR";
	else if (p_type == PT_GNU_EH_FRAME)
		return "FRAME"; 
	else if (p_type == PT_GNU_STACK) 
		return "STACK";
	else if (p_type == PT_GNU_RELRO)
		return "RELRO";
}

const char *get_segment_flag(Elf32_Off p_flag)
{
	if (p_flag == PF_X)
		return "  E";
	else if (p_flag == PF_W)
		return " W ";
	else if (p_flag == PF_R)
		return "R  ";
	else if (p_flag == PF_R | PF_W)
		return "RW ";
	else if (p_flag == PF_R | PF_X)
		return "R E";
	else if (p_flag == PF_W | PF_X)
		return " WE";
}

int main(int argc, const char *argv[])
{
	Elf32_Ehdr ehdr = { 0 };
	Elf32_Phdr phdr = { 0 };
	Elf32_Shdr shdr = { 0 };
	FILE *fp = NULL;
	int i;

	if (argc != 2) 	
		errx(EXIT_FAILURE, "usage: %s file-name", argv[0]);
	// open the file first	
	if (!(fp = fopen(argv[1], "rb")))
		errx(EXIT_FAILURE, "open \"%s\" failed", argv[1]);
	// now read the excutable header
	fread(&ehdr, sizeof(Elf32_Ehdr), 1, fp);
	// print the ehdr
	printf("ELF Header:\n", ehdr.e_ident);
	printf("  Magic:  ");
	for (i = 0; i < EI_NIDENT; i++) {
		printf("%02x ", ehdr.e_ident[i]);
	}
	printf("\n  Class:\t\t\t\t%s\n", ehdr.e_ident[EI_CLASS] == ELFCLASS32 ? "ELF32": "ELF64");
	printf("  Data:\t\t\t\t\t%s\n", ehdr.e_ident[EI_DATA] == ELFDATA2LSB ? "2's complement, little endian" 
																		: "2's complement, big endian");
//	printf("  Version:\t\t\t\t%s\n", ehdr.e_ident[EI_VERSION] == EV_CURRENT ? "1 (current)" : "");
	printf("  Version:\t\t\t\t%s\n", ehdr.e_version == EV_CURRENT ? "1 (current)" : "");
	printf("  OS/ABI:\t\t\t\t%s\n", ehdr.e_ident[EI_OSABI] == ELFOSABI_NONE ? "UNIX - System V" : "");
	printf("  ABI Version:\t\t\t\t%d\n", ehdr.e_ident[EI_ABIVERSION]);
	printf("  Type:\t\t\t\t\t%s\n", get_file_type(ehdr.e_type));
	printf("  Machine:\t\t\t\t%s\n", ehdr.e_machine == EM_386 ? "Intel 80386" : "");
	printf("  Version:\t\t\t\t0x%x\n", ehdr.e_version);
	printf("  Entry point address:\t\t\t0x%08x\n", ehdr.e_entry);
	printf("  Start of program headers:\t\t%d (bytes into file)\n", ehdr.e_phoff);
	printf("  Start of section headers:\t\t%d (bytes into file)\n", ehdr.e_shoff);
	printf("  Flags:\t\t\t\t0x%d\n", ehdr.e_flags);
	printf("  Size of this header:\t\t\t%d (bytes)\n", ehdr.e_ehsize);
	printf("  Size of program headers:\t\t%d (bytes)\n", ehdr.e_phentsize);
	printf("  Number of program headers:\t\t%d\n", ehdr.e_phnum);
	printf("  Size of section headers:\t\t%d (bytes)\n", ehdr.e_shentsize);
	printf("  Number of section headers:\t\t%d\n", ehdr.e_shnum);
	printf("  Section header string table index:\t%d\n\n", ehdr.e_shstrndx);

	printf("  Type\t\t\tOffset\t\tVirtAddr\tPhysAddr\tFileSiz\t\tMemSiz\t\tFlg\tAlign\n");
	// get program header
	for(i = 0; i < ehdr.e_phnum; i++) {
		memset(&phdr, 0, sizeof(Elf32_Phdr));	
		fread(&phdr, sizeof(Elf32_Phdr), 1, fp);
		printf("  %s\t\t\t0x%08x\t0x%08x\t0x%08x\t0x%08x\t0x%08x\t%s\t%d\n", get_program_header_type(phdr.p_type), phdr.p_offset,
				phdr.p_vaddr, phdr.p_paddr, phdr.p_filesz, phdr.p_memsz, get_segment_flag(phdr.p_flags), phdr.p_align);
	}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// print name of every section
	char buf[1024] = { 0 };
	if (fseek(fp, (long)(ehdr.e_shoff + sizeof(Elf32_Shdr) * ehdr.e_shstrndx), SEEK_SET) != -1) {
		fread(&shdr, sizeof(Elf32_Shdr), 1, fp);
		printf("0x%x\n", shdr.sh_offset);
		if (fseek(fp, (long)(shdr.sh_offset), SEEK_SET) != -1) {
			fread(buf, shdr.sh_size, 1, fp);
		}
	}
	for (i = 0; i < ehdr.e_shnum; i++) {
		if (fseek(fp, (long)(ehdr.e_shoff + sizeof(Elf32_Shdr) * i), SEEK_SET) != -1) {
			fread(&shdr, sizeof(Elf32_Shdr), 1, fp);
			printf("%02d: %s\n", i, buf + shdr.sh_name);
		}
	}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	fclose(fp);
	exit(EXIT_SUCCESS);;
}
