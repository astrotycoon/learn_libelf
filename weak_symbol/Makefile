all:
	gcc -o foo.o -c foo.c -fPIC
	gcc -o bar.o -c bar.c -fPIC
	ld -shared -o test1.so bar.o foo.o
	readelf --dyn-syms test1.so | grep -E "(foo|bar)"
	ar cr libbar.a bar.o
	ld -shared -o test2.so libbar.a foo.o
	readelf --dyn-syms test2.so | grep -E "(foo|bar)"
	gcc -o main.o -c main.c
	gcc -o main libbar.a main.o
	readelf --dyn-syms main | grep -E "(foo|bar)"

clean:
	rm -rf *.o *.so *.a main
