	.file "sf_test.S"
	.code32
	.text
.globl _start
_start:
	nop
	movl $0x7fffffff, %eax
	addl $1, %eax	
	
	movl $1, %eax
	movl $0, %ebx
	int $0x80
