#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <fcntl.h>
#include <gelf.h>
#include <stdint.h>
#include <unistd.h>

int main(int argc, const char *argv[])
{
	int fd;
	Elf *pelf = NULL;
	char *name, *p, pc[4 * sizeof(char)];
	Elf_Scn *scn;
	Elf_Data *data;
	GElf_Shdr shdr;
	size_t n, shstrndx, sz;

	if (argc != 2) 
		errx(EXIT_FAILURE, "usage: %s file-name", argv[0]);
	
	if (elf_version(EV_CURRENT) == EV_NONE)
		errx(EXIT_FAILURE, "ELF library initialization "
			"failed: %s", elf_errmsg(-1));
	
	if ((fd = open(argv[1], O_RDONLY, 0)) < 0)
		errx(EXIT_FAILURE, "open \"%s\" failed", argv[1]);

	if ((pelf = elf_begin(fd, ELF_C_READ, NULL)) == NULL)
		errx(EXIT_FAILURE, "elf_begin() failed: %s.", elf_errmsg(-1));

	if (elf_kind(pelf) != ELF_K_ELF)
		errx(EXIT_FAILURE, "%s is not an ELF object.", argv[1]);

	if (elf_getshdrstrndx(pelf, &shstrndx) != 0)
		errx(EXIT_FAILURE, "elf_getshdrstrndx() failed: %s.", elf_errmsg(-1));

	scn = NULL;
#if 0	// gelf
	while ((scn = elf_nextscn(pelf, scn)) != NULL) {
		if (gelf_getshdr(scn, &shdr) != &shdr)
			errx(EXIT_FAILURE, "getshdr() failed: %s.", elf_errmsg(-1));

		if ((name = elf_strptr(pelf, shstrndx, shdr.sh_name)) == NULL)
			errx(EXIT_FAILURE, "elf_strptr() failed: %s.", elf_errmsg(-1));

		printf("Section %-4.4jd %s\n", (uintmax_t)elf_ndxscn(scn), name);
	}
#endif

#if 1
	while ((scn = elf_nextscn(pelf, scn)) != NULL) {
		Elf32_Shdr *pshdr = NULL;
		if ((pshdr = elf32_getshdr(scn)) == NULL)
			errx(EXIT_FAILURE, "elf32_getshdr failed: %s.", elf_errmsg(-1));
		
		if ((name = elf_strptr(pelf, shstrndx, pshdr->sh_name)) == NULL)
			errx(EXIT_FAILURE, "elf_strptr() failed: %s.", elf_errmsg(-1));

		printf("Section %-4.4jd %s\n", (uintmax_t)elf_ndxscn(scn), name);
	}
#endif

#if 0
	int idx;
	GElf_Ehdr gehdr = { 0 };
	if ((gelf_getehdr(pelf, &gehdr)) != &gehdr) 
		errx(EXIT_FAILURE, "gelf_getehdr failed: %s\n", elf_errmsg(-1));
	for (idx = 0; idx < gehdr.e_shnum; idx++) {
		Elf32_Shdr *pshdr = NULL;
		if ((scn = elf_getscn(pelf, idx)) == NULL)
			errx(EXIT_FAILURE, "elf_getshdr failed: %s.\n", elf_errmsg(-1));

		if ((pshdr = elf32_getshdr(scn)) == NULL)
			errx(EXIT_FAILURE, "elf32_getshdr faile: %s\n", elf_errmsg(-1));

		if ((name = elf_strptr(pelf, shstrndx, pshdr->sh_name)) == NULL)
			errx(EXIT_FAILURE, "elf_strptr() failed: %s.", elf_errmsg(-1));

		printf("Section %-4.4jd %s\n", (uintmax_t)elf_ndxscn(scn), name);
	}
#endif

////////////////////////////////////////////////////
	if ((scn = elf_getscn(pelf, shstrndx)) == NULL)
		errx(EXIT_FAILURE, "getscn() failed: %s.", elf_errmsg(-1));
	
	if (gelf_getshdr(scn, &shdr) != &shdr)
		errx(EXIT_FAILURE, "getshdr(shstrndx) failed: %s.", elf_errmsg(-1));
	printf(".shstrtab: size = %jd\n", (uintmax_t)shdr.sh_size);

	data = NULL; n = 0;
	while (n < shdr.sh_size 
		&& (data = elf_getdata(scn, data)) != NULL) {
		p = (char *)data->d_buf;
		while (p < ((char *)data->d_buf + data->d_size)) {
			printf("%c ", *p);
			n++; p++;
		}
	}
	
	putchar('\n');
	elf_end(pelf);
	close(fd);
	return EXIT_SUCCESS;
}
