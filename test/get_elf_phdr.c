#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <fcntl.h>
#include <gelf.h>
#include <unistd.h>
#include <stdint.h>

static void print_ptype(size_t pt);

int main(int argc, const char *argv[])
{
	int i, fd;
	Elf *pelf = NULL;
	char *id, bytes[5];
	size_t n = 0;
	Elf32_Phdr *phdr32 = NULL; 
	GElf_Phdr gphdr = { 0 };	

	if (argc != 2)	
		errx(EXIT_FAILURE, "usage: %s file-name", argv[0]);

	if (elf_version(EV_CURRENT) == EV_NONE)
		errx(EXIT_FAILURE, "ELF library initialization "
				"failed: %s", elf_errmsg(-1));

	if ((fd = open(argv[1], O_RDONLY, 0)) < 0)
		errx(EXIT_FAILURE, "open \"%s\" failed", argv[1]);

	if ((pelf = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
		errx(EXIT_FAILURE, "elf_begin() failed: %s",
				elf_errmsg(-1));

	if (elf_kind(pelf) != ELF_K_ELF)
		errx(EXIT_FAILURE, "\"%s\" is not an ELF object.",
				argv[1]);

	// get the num of program header table
	if (elf_getphdrnum(pelf, &n) != 0)
		errx(EXIT_FAILURE, "elf_getphdrnum() failed: %s.",
				elf_errmsg(-1));
	
#if 0
	// 32bit
	if ((phdr32 = elf32_getphdr(pelf)) == NULL)
		errx(EXIT_FAILURE, "elf32_getphdr() failed: %s.",
				elf_errmsg(-1));
	for (i = 0; i < n; i++) {
		Elf32_Phdr phdr = phdr32[i];
		printf("PHDR %d:\n", i);
#define PRINT_FMT	"    %-20s 0x%jx"
#define PRINT_FIELD(N)	do { printf(PRINT_FMT, #N, (uintmax_t)phdr.N); } while (0);
#define NL() do { printf("\n"); } while (0);

		PRINT_FIELD(p_type);
		print_ptype(phdr.p_type); 				NL();
		PRINT_FIELD(p_offset);					NL();
		PRINT_FIELD(p_vaddr);					NL();
		PRINT_FIELD(p_paddr); 					NL();
		PRINT_FIELD(p_filesz);					NL();
		PRINT_FIELD(p_memsz);					NL();
		PRINT_FIELD(p_flags);					
		printf(" [");
		if (phdr.p_flags & PF_X)
			printf(" execute");
		if (phdr.p_flags & PF_R)
			printf(" read");
		if (phdr.p_flags & PF_W)
			printf(" write");
		printf(" ]");			NL();
		PRINT_FIELD(p_align);	NL();
	}
#endif

#if 1
	for (i = 0; i < n; i++) {
		// get every entry fist 
		if (gelf_getphdr(pelf, i, &gphdr) != &gphdr)	
			errx(EXIT_FAILURE, "gelf_getphdr() failed: %s.",
													elf_errmsg(-1));
		// now print every entry
		printf("PHDR %d:\n", i);
#define PRINT_FMT	"    %-20s 0x%jx"
#define PRINT_FIELD(N)	do { printf(PRINT_FMT, #N, (uintmax_t)gphdr.N); } while (0);
#define NL() do { printf("\n"); } while (0);

		PRINT_FIELD(p_type);
		print_ptype(gphdr.p_type); 				NL();
		PRINT_FIELD(p_offset);					NL();
		PRINT_FIELD(p_vaddr);					NL();
		PRINT_FIELD(p_paddr); 					NL();
		PRINT_FIELD(p_filesz);					NL();
		PRINT_FIELD(p_memsz);					NL();
		PRINT_FIELD(p_flags);					;
		printf(" [");
		if (gphdr.p_flags & PF_X)
			printf(" execute");
		if (gphdr.p_flags & PF_R)
			printf(" read");
		if (gphdr.p_flags & PF_W)
			printf(" write");
		printf(" ]");			NL();
		PRINT_FIELD(p_align);	NL();
	}
#endif

	elf_end(pelf);
	close(fd);

	exit(EXIT_SUCCESS);
}

static void print_ptype(size_t pt)
{
	char *s = NULL;
#define C(V)	case PT_##V: s = #V; break
	switch (pt) {
		C(NULL);	C(LOAD);		C(DYNAMIC);
		C(INTERP);	C(NOTE);		C(SHLIB);
		C(PHDR);	C(TLS);			C(SUNWSTACK);
		C(LOSUNW);  C(GNU_STACK);	C(GNU_RELRO);	
		C(GNU_EH_FRAME);
	default:
		s = "unknown";
		break;
	}
	printf(" \"%s\"", s);
#undef C
}
