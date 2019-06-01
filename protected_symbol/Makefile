all:
	gcc -fPIC foo.c -c -o foo.o
	gcc -fPIC foofunc.c -c -o foofunc.o
	gcc -o libfoo.so -shared foo.o foofunc.o
	gcc -o main main.c -L. -lfoo

clean:
	rm -rf *.o *.so main
