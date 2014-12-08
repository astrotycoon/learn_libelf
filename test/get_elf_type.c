#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <fcntl.h>
#include <libelf.h>
#include <unistd.h>

int main(int argc, const char *argv[])
{
	int fd = 0;
	Elf *pelf = NULL;		
	const char *buf = NULL;
	Elf_Kind ek;

	if (argc != 2) 
		errx(EXIT_FAILURE, "usage: %s file-name", argv[0]);

	//	+-------------------------+						 +----------------------+						+-----------------------------+
	//  | application v1		  | -------------------> | libelf v1 v2		 	| ------------------->	|	Elf Object v2	 		  |
	//	| 程序中打印信息默认使用v1| <------------------	 | 转换两个版本			| <------------------ 	| 即目标文件组成遵守的那个版本|
	//	+-------------------------+						 +----------------------+						+-----------------------------+
	if (elf_version(EV_CURRENT) == EV_NONE) 
		errx(EXIT_FAILURE, "ELF library initialization failed: %s", elf_errmsg(-1));

	if ((fd = open(argv[1], O_RDONLY, 0)) < 0)
		errx(EXIT_FAILURE, "open %s failed", argv[1]);
	
	if (!(pelf = elf_begin(fd, ELF_C_READ, NULL)))
		errx(EXIT_FAILURE, "elf_begin() failed: %s", elf_errmsg(-1));

	switch (ek = elf_kind(pelf)) {
		case ELF_K_AR:
			buf = "ar(1) archive";
			break;
		case ELF_K_ELF:
			buf = "elf object";
			break;
		case ELF_K_NONE:
			buf = "data";
			break;
		default:
			buf = "unrecognized";	
	}

	(void)printf("%s: %s\n", argv[1], buf);
	(void)elf_end(pelf);
	(void)close(fd);

	exit(EXIT_SUCCESS);
}
