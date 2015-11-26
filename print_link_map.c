#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <link.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

/*
 * 本程序通过两种方法获得GOT首地址，然后强转GOT[1]为struct link_map*
 * 链表打印
 * http://blog.csdn.net/lzshlzsh/article/details/6066628
 * Linux下库函数动态链接过程分析－结合glibc-2.11源码
 */

static void print_link_map(ElfW(Addr) *got_addr)
{
	const struct link_map *link_map, *lnk_tmp;
	int i;
		
	for (i = 0; i < 3; i++) {
		printf("GOT[%d] = 0x%08x\n", i, got_addr[i]);
	}

	link_map = (const struct link_map *)got_addr[1];
	for (lnk_tmp = link_map; lnk_tmp; lnk_tmp = lnk_tmp->l_next) {
		printf("\t%s: 0x%08x\n", lnk_tmp->l_name, lnk_tmp->l_addr);
	}
	printf("\n***************************************************\n\n");
}

static int print_shared_objects_name_and_address(struct dl_phdr_info *info, size_t size, void *data);

int main(int argc, const char *argv[])
{
	int fd; 
	struct stat stat = { 0 };
	char *file_mmbase = NULL;
	ElfW(Ehdr) *ehdr = NULL;
	ElfW(Phdr) *phdr = NULL;
	ElfW(Shdr) *shdr = NULL;
	char *strtab = NULL;
	int i;
#if 0
	ElfW(Addr) image_base = 0U;
#endif
	ElfW(Addr) *pgot = NULL; 			/* GOT表首地址 */
	ElfW(Word) dynamic_size = 0U; 
	ElfW(Dyn) *dynamic_base = NULL;		/* 也可以直接使用link.h中提供的extern ElfW(Dyn) _DYNAMIC[];*/

	/* open file */
	if ((fd = open(argv[0], O_RDONLY)) < 0) {
		perror("open error:");		
		exit(errno);
	} 

	/* get file stat info */
	if (fstat(fd, &stat) < 0) {
		perror("fstat error:");
		close(fd);
		exit(errno);
	}

	/* mmap file into memory */
	file_mmbase = (char *)mmap(0, stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (file_mmbase == MAP_FAILED) {
		perror("mmap error: ");
		close(fd);
		exit(errno);
	}
	close(fd);

	ehdr = (ElfW(Ehdr) *)file_mmbase;
	phdr = (ElfW(Phdr) *)(file_mmbase + ehdr->e_phoff);
	shdr = (ElfW(Shdr) *)(file_mmbase + ehdr->e_shoff);
	strtab = (char *)(file_mmbase + shdr[ehdr->e_shstrndx].sh_offset);

	/*
	 * get the progress image's base address, for executable file, this
	 * address is the same with the static address
	 */
#if 0
	for (i = 0; i < ehdr->e_phnum; i++) {
		if (PT_LOAD == phdr[i].p_type) {
			image_base = phdr[i].p_vaddr; 	/* 获得程序加载地址基地址 */
			break;
		}
	}
#endif

	for (i = 0; i < ehdr->e_shnum; i++) {
		if (strcmp(".got.plt", strtab + shdr[i].sh_name) == 0) {
			pgot = (ElfW(Addr) *)shdr[i].sh_addr; 		/* 获得.got.plt的加载地址 */
			break;
		}
	}
	if (pgot == NULL) {
		/* try section .got */
		for (i = 0; i < ehdr->e_shnum; i++) {
			if (strcmp(".got", strtab + shdr[i].sh_name) == 0) {
				pgot = (ElfW(Addr) *)shdr[i].sh_addr; 	/* 获得.got.plt的加载地址 */
				break;
			}
		}
	}	

	assert(pgot);

	printf("one way to get got address(from .got.plt or .got ...)\n");

	print_link_map(pgot);
///////////////////////////////////////////////////////////////////////////////////
	pgot = NULL;

	/* another way to get GOT address */
	/* get .dynamic section */
	for (i = 0; i < ehdr->e_shnum; i++) {
		if (strcmp(".dynamic", strtab + shdr[i].sh_name) == 0) {
			dynamic_size = shdr[i].sh_size;
			dynamic_base = (ElfW(Dyn) *)shdr[i].sh_addr;
			break;	
		}
	}

#if 0
	for (i = 0; i < dynamic_size/ sizeof(ElfW(Dyn)); i++) {
		if (DT_PLTGOT == dynamic_base[i].d_tag) {
			pgot = (ElfW(Addr) *)dynamic_base[i].d_un.d_ptr;
			break;
		}
	}
#else
#if 0
	for (i = 0; i < dynamic_size/ sizeof(ElfW(Dyn)); i++) {
		if (DT_PLTGOT == _DYNAMIC[i].d_tag) {
			pgot = (ElfW(Addr) *)_DYNAMIC[i].d_un.d_ptr; 
			break;
		}
	}
#else
	ElfW(Dyn) *dyn;
	for (dyn = _DYNAMIC; dyn->d_tag != DT_NULL; ++dyn) {
		if (dyn->d_tag == DT_PLTGOT) {
			pgot = (ElfW(Addr) *)dyn->d_un.d_ptr; 
			break;
		}
	}	
#endif
#endif


	assert(pgot);

	printf("another way to get got address(from .dynamic ...)\n");
	print_link_map(pgot);

	if (munmap(file_mmbase, stat.st_size) == -1) {
		perror("munmap error:");
		exit(errno);
	}

#if 0
extern const Elf32_Addr _GLOBAL_OFFSET_TABLE_[];
	printf("%p\n", _GLOBAL_OFFSET_TABLE_);
	print_link_map((ElfW(Addr) *)_GLOBAL_OFFSET_TABLE_[1]);
#endif
	
	dl_iterate_phdr(print_shared_objects_name_and_address, NULL);
	exit(EXIT_SUCCESS);
}

static int print_shared_objects_name_and_address(struct dl_phdr_info *info, size_t size, void *data)
{
//	ElfW(Half) i;
//	for (i = 0; i < info->dlpi_phnum; i++) {
		printf("%s: %p\n", info->dlpi_name, (void *)info->dlpi_addr);
//	} 

	return 0;
}
