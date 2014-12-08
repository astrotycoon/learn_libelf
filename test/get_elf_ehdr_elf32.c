#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdint.h>
#include <unistd.h>

int main(int argc, const char *argv[])
{
	int i, fd;
	Elf *pelf = NULL;
	char *id, bytes[5];
	Elf32_Ehdr *pehdr = NULL; 
	
	if (argc != 2)
		errx(EXIT_FAILURE, "usage: %s file-name", argv[0]);

	if (elf_version(EV_CURRENT) == EV_NONE)
		errx(EXIT_FAILURE, "ELF library initializztion "
			"failed: %s", elf_errmsg(-1));

	if ((fd = open(argv[1], O_RDONLY, 0)) < 0)
		errx(EXIT_FAILURE, "open \"%s\" failed", argv[1]);

	if (!(pelf = elf_begin(fd, ELF_C_READ, NULL)))
//	if (!(pelf = elf_begin(fd, ELF_C_FDDONE, NULL)))
		errx(EXIT_FAILURE, "elf_begin() failed: %s", elf_errmsg(-1));

	if (elf_kind(pelf) != ELF_K_ELF)
		errx(EXIT_FAILURE, "\"%s\" is not an ELF object.", argv[1]);

	// get the elf header
	if ((pehdr = elf32_getehdr(pelf)) == NULL)
		errx(EXIT_FAILURE, "getehdr() failed: %s.", elf_errmsg(-1));

	// get elf class ()
	if ((i = gelf_getclass(pelf)) == ELFCLASSNONE)
		errx(EXIT_FAILURE, "getclass() failed: %s.", elf_errmsg(-1));
	// print the elf class
	printf("%s: %d-bit ELF object\n", argv[1], 
								(i == ELFCLASS32) ? 32 : 64);
	// get e_ident
	if ((id = elf_getident(pelf, NULL)) == NULL)
		errx(EXIT_FAILURE, "getident() failed: %s.", elf_errmsg(-1));
	// print e_ident
	printf("e_ident[0..%1d]:", EI_ABIVERSION);
	for (i = 0; i <= EI_ABIVERSION; i++) {
		printf("0x%02x ", id[i]);
	}
	printf("\n");
	for (i = 0; i <= EI_ABIVERSION; i++) {
		switch (i) {
			case 0:
				printf("0x%02x ", id[i]);
				break;
			case 1 ... 3:
				printf("%c ", id[i]);
				break;
			case 4:
				printf("%d-bit ", id[i] == ELFCLASS32 ? 32 : 64);
				break;
			case 5:
				printf("%s ", id[i] == ELFDATA2LSB ? "LSB" : "MSB");
				break;
			case 6:
				printf("%s ", id[i] == EV_CURRENT ? "EV_CURRENT" : "EV_NONE");
				break;
			case 7:
				printf("%s ", id[i] == ELFOSABI_NONE ? "ELFOSABI_NONE" : " ");
				break;
			case 8: 
				printf("%d ", 0);
				break;
		}
	}
	printf("\n");
#define PRINT_FMT 	"    %-20s 0x%jx\n"
#define PRINT_FIELD(N) do { (void)printf(PRINT_FMT, #N, (uintmax_t)pehdr->N); } while (0);

/*
typedef struct {
        unsigned char e_ident[EI_NIDENT];
        Elf32_Half e_type;
        Elf32_Half e_machine;
        Elf32_Word e_version;
        Elf32_Addr e_entry;
        Elf32_Off e_phoff;
        Elf32_Off e_shoff;
        Elf32_Word e_flags;
        Elf32_Half e_ehsize;
        Elf32_Half e_phentsize;
        Elf32_Half e_phnum;
        Elf32_Half e_shentsize;
        Elf32_Half e_shnum;
        Elf32_Half e_shstrndx;
    } Elf32_Ehdr;
*/

	PRINT_FIELD(e_type);	
	PRINT_FIELD(e_machine);	
	PRINT_FIELD(e_version);	
	PRINT_FIELD(e_entry);	
	PRINT_FIELD(e_phoff);	
	PRINT_FIELD(e_shoff);	
	PRINT_FIELD(e_flags);	
	PRINT_FIELD(e_ehsize);	
	PRINT_FIELD(e_phentsize);	
	PRINT_FIELD(e_shentsize);	
	PRINT_FIELD(e_phnum);	
	PRINT_FIELD(e_shnum);
	PRINT_FIELD(e_shstrndx);
	printf("\n");
	
	size_t n = 0;
	// get shnum
	if (elf_getshdrnum(pelf, &n) != 0)
		errx(EXIT_FAILURE, "getshdrnum() failed: %s.",
				elf_errmsg(-1));
	printf(PRINT_FMT, "(shnum)", (uintmax_t)n);

	// get phnum
	if (elf_getphdrnum(pelf, &n) != 0)
		errx(EXIT_FAILURE, "getphdrnum() failed: %s.",
				elf_errmsg(-1));
	printf(PRINT_FMT, "(phnum)", (uintmax_t)n);

	// get shstrndx
	if (elf_getshdrstrndx(pelf, &n) != 0)
		errx(EXIT_FAILURE, "getshdrstrndx() failed: %s.",
				elf_errmsg(-1));
	printf(PRINT_FMT, "(shstrndx)", (uintmax_t)n);
	elf_end(pelf);	
	exit(EXIT_SUCCESS);
}
