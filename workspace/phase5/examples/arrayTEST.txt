	.globl	main
main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$4, %esp
	movl	$10, %eax
	movl	%eax, -4(%ebp)
	pushl	-4(%ebp)
	call	init_array
	addl	$4, %esp
	pushl	-4(%ebp)
	call	print_array
	addl	$4, %esp
	movl	%ebp, %esp
	popl	%ebp
	ret


	.data
	.comm	a, 40, 4
