#include <stdio.h>
#include "foo.h" 

int global1 = 1;
int global2 = 2;

int main(int argc, const char *argv[])
{
	printf("globa1 + global2 = %d\n", foo());		
	return 0;
}
