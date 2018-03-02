	.globl	towers
towers:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$0, %esp
	pushl	16(%ebp)
	pushl	20(%ebp)
	pushl	12(%ebp)
	pushl	8(%ebp)
	call	call_towers
	addl	$16, %esp
	pushl	16(%ebp)
	pushl	12(%ebp)
	call	print_move
	addl	$8, %esp
	pushl	12(%ebp)
	pushl	16(%ebp)
	pushl	20(%ebp)
	pushl	8(%ebp)
	call	call_towers
	addl	$16, %esp
	movl	%ebp, %esp
	popl	%ebp
	ret


	.globl	main
main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$4, %esp
	movl	$3, %eax
	movl	%eax, -4(%ebp)
	pushl	-4(%ebp)
	call	print
	addl	$4, %esp
	pushl	$3
	pushl	$2
	pushl	$1
	pushl	-4(%ebp)
	call	towers
	addl	$16, %esp
	movl	%ebp, %esp
	popl	%ebp
	ret


