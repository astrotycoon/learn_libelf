/*
 * 1. 两个正数相加，如果结果为负数，就产生了溢出。
 *
 * 2. 两个负数相加，如果结果为正数，就产生了溢出。
 *
 * 3. 不同符号的两个数相加，不会产生溢出。
 *
 */	
	.file "of_test.S"
	.code32

	.text
.global _start
_start:
	clc
	/*
     * 		0111 1111
     * +    0000 0001
     * --------------
     *      1000 0000     
     *
     * 有符号数，127+1=128现在却是-128,结果不正确，发生溢出，eflags.OF=1
     * 无符号数，127+1=128，正确，eflags.CF=0
	 * 
     * 顺带eflags.SF=1, eflags.AF=1
     *
     */
	movb $0x7f, %al
	addb $1, %al 	/* 正数 + 正数 = 负数，就产生了溢出 [ AF SF IF OF ] */

	xor %eax, %eax
	
	clc
	/*
     *		1000 0101 1000 1111
     * -	0111 1000 0110 1001
     * ------------------------
     *      0000 1101 0010 0110
     *
     * 有符号，分别为
     *
     *
     *
     */
	movw $0x858f, %ax
	subw $0x7869, %ax 	/* %ax = %ax - 0x7869 */

	clc
	movb $0x7f, %al
	addb $2, %al

	xor %eax, %eax

	clc
	movw $0x7fff, %ax
	addw $1, %ax

	xor %eax, %eax

	clc
	movw $0x7fff, %ax
	addw $2, %ax

	xor %eax, %eax

	clc
	movl $0x7fffffff, %eax
	addl $1, %eax

	xor %eax, %eax
	
	clc 
	movl $0x7fffffff, %eax
	addl $2, %eax

	

	xor %eax, %eax

	clc
	movb $0xfc, %al
	addb $0xf8, %al

	clc

	xor %eax, %eax
	movw $1, %ax
	neg %ax
	cwd

	clc
	xor %eax, %eax

	/* 两正数相加 */
	/*
	 *		0111 1111 1111 1111 1111 1111 1111 1110
     * +	0000 0000 0000 0000 0000 0000 0000 0001
     * --------------------------------------------
     *      0111 1111 1111 1111 1111 1111 1111 1111
     *
     * 有符号数， 2147483646 + 1 = 2147483647，正确，eflags.OF=0
     * 无符号数， 2147483646 + 1 = 2147483647，正确，eflags.CF=0
	 *
     * by the way, eflags.PF=1(最低字节中1的个数是偶数)
     */
	movl $0x7ffffffe, %eax 
	addl $1, %eax	/* %eax = %eax + 1 [ PF IF ] */

	clc
	xorl %eax, %eax	

	/*
     *		0111 1111 1111 1111 1111 1111 1111 1111
     * +	0000 0000 0000 0000 0000 0000 0000 0001
     * --------------------------------------------
     *      1000 0000 0000 0000 0000 0000 0000 0000
     *
     *   signed，2147483647 + 1 = 2147483648，但是结果却是-2147483648，溢出，eflags.OF=1
     * unsigned, 2147483647 + 1 = 2147483648，正确，eflags.CF=0 
     *
     * by the way, eflags.AF=1 eflags.SF=1 
     */
	movl $0x7fffffff, %eax
	addl $1, %eax	/* %eax = %eax + 1 */

	clc
	xorl %eax, %eax

	/* 两负数相加 */
	/*
     *		1111 1111 1111 1111 1111 1111 1111 1110
     * +	1111 1111 1111 1111 1111 1111 1111 1111
     * --------------------------------------------
     *	 (1)1111 1111 1111 1111 1111 1111 1111 1101
     *
     *  signed，-2 + -1 = -3，正确，eflags.OF=0, 最高位有进位, eflags.CF=1
     *unsigned, 4294967294 + 4294967295 = 8589934589，但是结果（4294967293）不对， 溢出，eflags.CF=1 
     *
     * by the way, eflags.AF=1, eflags.SF=1
     */
	movl $0xfffffffe, %eax 
	addl $0xffffffff, %eax	/* %eax = %eax + (-1) [ CF AF SF IF ] */

	clc 
	xorl %eax, %eax

	/* 
     * 		1111 1111 1111 1111 1111 1111 1111 1111
     * + 	1000 0000 0000 0000 0000 0000 0000 0000
     * --------------------------------------------
     *	 (1)0111 1111 1111 1111 1111 1111 1111 1111
     *
     *  signed, -1 + -2147483648 = -2147483649才对， 但是结果却是2147483647，溢出，eflags.OF=1
     *unsigned, 4294967295 + 2147483648 = 6442450943才对，但是结果却是2147483647，有进位, eflags.CF=1
     *
     * by the way, eflags.PF=1
     *
     */
	movl $0xffffffff, %eax
	addl $0x80000000, %eax	/* %eax = %eax + 0x80000000 [ CF PF IF OF ] */


	clc
	xorl %eax, %eax
	
	/* 一正一负相加，结果不会溢出 */
	/*
	 * 		1111 1111 1111 1111 1111 1111 1111 1111
	 * +	0000 0000 0000 0000 0000 0000 0000 0001
	 * --------------------------------------------
	 *   (1)0000 0000 0000 0000 0000 0000 0000 0000
	 *
	 *  signed, -1 + 1 = 0， 正确, eflags.OF=0
	 *unsigned, 4294967295 + 1 = 4294967296才对，结果却是0，溢出，有进位，eflags.CF=1
	 * 
	 * by the way, eflags.AF=1 eflags.PF=1 eflags.ZF=1
     */
	movl $0xffffffff, %eax
	addl $1, %eax	/* %eax = %eax + 1 [ CF PF AF ZF IF ] */

	clc
	xorl %eax, %eax
	/*
     *
     * 		1000 0000 0000 0000 0000 0000 0000 0000
     *	+	1000 0000 0000 0000 0000 0000 0000 0000
     *	-------------------------------------------
     *	 (1)0000 0000 0000 0000 0000 0000 0000 0000
     *
     *	signed, (-2147483648) + (-2147483648) = (-4294967296)才对，结果却是0，溢出，eflags.OF=1 
     *unsigned, 2147483648 + 2147483648 = 4294967296才对， 结果却是0，溢出， 有进位，eflags.CF=1 
     *
     * by the way, eflags.ZF=1, eflags.PF=1
     *
     */
	movl $0x80000000, %eax	/* [ PF ZF IF ] */
	addl %eax, %eax	/* [ CF PF ZF IF OF ] */
	addl %eax, %eax	/* [ PF ZF IF ] */

	clc
	movb $0x50, %al
	subb $0x30, %al

	clc
	xorl %eax, %eax
	
	movw $20000, %ax
	movw $4, %bx
	mul %bx	/* [ CF IF OF ] */

	clc
	xorl %eax, %eax

	/*
     * 80 - 48 = 32
     * 
     * 用80 -48并没有进位
     * 但是用80加上-48的补码就有，怎么回事?
     *
     */

	movb $0x50, %al
	subb $0x30, %al

	clc
	xorl %eax, %eax
	
	movb $0x50, %al
	addb $0xd0, %al


	clc 
	xorl %eax, %eax
	
	movb $0x50, %al
	addb $0x50, %al

	movl $1, %eax
	movl $0, %ebx
	int $0x80

