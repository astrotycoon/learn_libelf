#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int _get_file_size(const char *file_name);

int main(int argc, const char *argv[])
{
	if (argc != 2) {
		fprintf(stderr, "Usage: %s + file_name.\n", argv[0]);
		exit(EXIT_FAILURE);
	}	

	printf("the size of file %s is %d.\n", argv[1], _get_file_size(argv[1]));

	return EXIT_SUCCESS;
}

int _get_file_size(const char *file_name)
{
	if (!file_name)	return -1;
	int file_size;

#if 0
	FILE *fp;

	if (!(fp = fopen(file_name, "rb"))) {
		errx(EXIT_FAILURE, "open file %s error.\n", file_name);
	}

	// set the file position indicator to the end of file 
	if (fseek(fp, 0L, SEEK_END) != 0) {
		errx(EXIT_FAILURE, "fseek() failed.\n");
	}	
	// now get the file size through ftell 	
	if ((file_size = ftell(fp)) == -1L) {
		errx(EXIT_FAILURE, "ftell() failed.\n");
	}
#endif

#if 0
	struct stat file_info;
		
	if (stat(file_name, &file_info) == -1) {
		errx(EXIT_FAILURE, "stat() failed.\n");
	}
	file_size = file_info.st_size;
#endif

	int fd;
	if ((fd = open(file_name, O_RDONLY)) == -1) {
		errx(EXIT_FAILURE, "open %s error.\n", file_name);
	}
	if ((file_size = lseek(fd, (off_t)0, SEEK_END)) == (off_t)-1) {
		errx(EXIT_FAILURE, "lseek() failed.\n");
	}

	return file_size;	
}
