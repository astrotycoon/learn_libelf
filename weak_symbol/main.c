extern int bar(void) __attribute__((weak));

int main(int argc, const char *argv[])
{
	return bar();
}
