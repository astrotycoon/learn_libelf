extern int bar(void) __attribute__((weak));

int foo(void) 
{
	return bar();
}
