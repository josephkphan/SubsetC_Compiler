	.globl	main
main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$0, %esp
	pushl	$83
	call	putchar
	addl	$4, %esp
	pushl	$101
	call	putchar
	addl	$4, %esp
	pushl	$103
	call	putchar
	addl	$4, %esp
	pushl	$109
	call	putchar
	addl	$4, %esp
	pushl	$101
	call	putchar
	addl	$4, %esp
	pushl	$110
	call	putchar
	addl	$4, %esp
	pushl	$116
	call	putchar
	addl	$4, %esp
	pushl	$97
	call	putchar
	addl	$4, %esp
	pushl	$116
	call	putchar
	addl	$4, %esp
	pushl	$105
	call	putchar
	addl	$4, %esp
	pushl	$111
	call	putchar
	addl	$4, %esp
	pushl	$110
	call	putchar
	addl	$4, %esp
	pushl	$32
	call	putchar
	addl	$4, %esp
	pushl	$102
	call	putchar
	addl	$4, %esp
	pushl	$97
	call	putchar
	addl	$4, %esp
	pushl	$117
	call	putchar
	addl	$4, %esp
	pushl	$108
	call	putchar
	addl	$4, %esp
	pushl	$116
	call	putchar
	addl	$4, %esp
	pushl	$10
	call	putchar
	addl	$4, %esp
	movl	%ebp, %esp
	popl	%ebp
	ret


