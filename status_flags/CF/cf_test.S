/*	1. 两数相加，计算的结果最高位向上进一位时，eflags.CF=1
 *  2. 两数相减，最高位不足减，向前借位，eflags.CF=1
 *
 *  CF标志只针对无符号数，即当你把程序中的数字当做无符号数时，
 *  那么根据计算结果决定程序走向时，需要关注CF标志。
 *
 */	
	.file 	"af_test.S"
	.code32

.section .data
data1:
	.int 0xffffffff 

.section .text
.global _start
_start:
	/* 		1111 1111 1111 1111 1111 1111 1111 1111
     *	+   0000 0000 0000 0000 0000 0000 0000 0001
     * --------------------------------------------
     *     10000 0000 0000 0000 0000 0000 0000 0000
     * 0xffffffff 当成有符号数的话，-1， 加上1，等于0，没有溢出，eflags.OF=0
	 *
	 * 0xffffffff 当成有符号数则是4294967295U， 加1，等于0，溢出，最高位进位，eflags.CF=1
	 *
	 * 顺带eflags.ZF=1 eflags.AF=1 eflags.PF=1
	 *
     */
	movl $-1, %eax
	xor %eax, %eax 	/* [ PF ZF IF ] */
	movl data1, %eax
	addl $1, %eax	/* [ CF PF AF ZF IF ] */
	movl %eax, data1

	clc				/* [ PF AF ZF IF ] */
	/*
     * 		0111 1111 1111 1111
     * +	0000 0000 0000 0010
     * ------------------------
     *      1000 0000 0000 0001                 
     *
     * 0x7fff 当有有符号数的话，32767，加上2，本应等于32769，结果却是-1，溢出，eflags.OF=1
     * 0x7fff 当有无符号数，32767，加上2，等于32769，无进位，eflags.CF=0
     *
     * 顺带eflags.AF=1 eflags.SF=1
     */
	movw $0x7fff, %ax
	addw $2, %ax	/* [ AF SF IF OF ] */

	xorl %eax, %eax /* [ PF ZF IF ] */
	cmc /* [ CF PF ZF IF ] */
	clc /* [ PF ZF IF ] */
	/*
     *		1111 0000
     * +    0111 1000
     * --------------
     *     10110 1000         
     *
     * 0xf0 0x78 当成有符号数，分别为-16， 120，相加应该等于104，结果正确，没有溢出，eflags.OF=0
     * 0xf0 0x78 当成无符号数，分别为240，120，本应该等于360，现在结果却是104，溢出，有进位，eflags.CF=1
     *
     */
	movb $0xf0, %al	
	addb $0x78, %al /* [ CF IF ] */

	/* -128 + (-128) = 1 0000 0000 CF=1 PF=1 ZF=1 OF=1 */
	/* 		1000 0000
	 * +	1000 0000
     *----------------	
	 *	   10000 0000
	 *	signed: -128 + 128 = -256 OF=1
     *unsigned: 128 + 128 = 256 CF=1
	 */
	xorl %eax, %eax /* [ PF ZF IF ]  */
	/*
     *		1000 0000
     * +	1000 0000
     * --------------
     *	   10000 0000 
     *
     * 0x80 当成有符号数，为-128，-128 + (-128) = -256，现在结果却是0，溢出，eflags.OF=1
     * 0x80 当成无符号数，为128, 128 + 128 = 256，现在结果却是0，溢出，有进位，eflags.CF=1
     *
     * 顺带eflags.ZF=1 eflags.PF=1
     *
     */
	movb $0x80, %al
	movb $0x80, %ah
	addb %ah, %al 	/* [ CF PF ZF IF OF ] */

	/* 两数相加 */
	movl $0x80000000, %eax
	movl $0x80000000, %ebx
	addl %ebx, %ebx	/* %ebx = %eax + %ebx [ CF PF ZF IF OF ] */
	/* 两数相减 */
	/*
     * 	 (1)0000 0000 0000 0000 0000 0000 0000 0000
     * -    0000 0000 0000 0000 0000 0000 0000 0001
     * --------------------------------------------
     *      1111 1111 1111 1111 1111 1111 1111 1111 
     *
     * 有符号，0 - 1 = -1，正确，eflags.OF=0
     * 无符号，0 - 1 = -1，溢出，eflags.CF=1（最高位借位）
     *
     * 顺带eflags.PF=1 eflags.AF=1 eflags.SF=1
     *
     */
	movl $0, %eax
	movl $1, %ebx
	subl %ebx, %eax	/* %eax = %eax - %ebx [ CF PF AF SF IF ] */

	movl $0x000f003f, %esi
	movl %ecx, %esi


	movl $1, %eax
	movl $0, %ebx	/* return 0 to shell */
	int $0x80 
