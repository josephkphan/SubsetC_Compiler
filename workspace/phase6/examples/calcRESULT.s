--Function::generate()
_lexan:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$lexan.size, %esp
--Block::generate()
--If::generate()
--Equal::generate()
--Identifier::generate()
--Number::generate()
	movl	_c, %eax
	cmpl	$0, %eax
	sete	%al
	movzbl	%al, %eax
--Expression::generate()
	movl	%eax, -4(%ebp)
	movl	-4(%ebp), %eax
	cmpl	$0, %eax
	je	.L0
--Assignment::generate()
--Expression::generate()
--Identifier::generate()
--Call::generate()
--Expression::generate()
	call	_getchar
	movl	, %eax
	movl	%eax, _c
.L0:
.L1:
--While::generate()
.L2:
--LogicalAnd::generate()
--Call::generate()
--Expression::generate()
--Identifier::generate()
	movl	_c, %eax
	movl	%eax, 0(%esp)
	call	_isspace
movl	,%eax
cmpl	$0,%eax
jne	.L3
--NotEqual::generate()
--Identifier::generate()
--Number::generate()
	movl	_c, %eax
	cmpl	$10, %eax
	setne	%al
	movzbl	%al, %eax
--Expression::generate()
	movl	%eax, -8(%ebp)
movl	-8(%ebp),%eax
cmpl	$0,%eax
.L3:
--Expression::generate()
	setne	%al
	movzbl	%al,%eax
	movl	%eax,-12(%ebp)
	movl	-12(%ebp), %eax
	cmpl	$0, %eax
	je	.L4
--Assignment::generate()
--Expression::generate()
--Identifier::generate()
--Call::generate()
--Expression::generate()
	call	_getchar
	movl	, %eax
	movl	%eax, _c
	jmp	.L2
.L4:
--If::generate()
--Not::generate()
--Call::generate()
--Expression::generate()
--Identifier::generate()
	movl	_c, %eax
	movl	%eax, 0(%esp)
	call	_isdigit
	movl	, %eax
	cmpl	$0, %eax
	sete	%al
	movzbl	%al, %eax
--Expression::generate()
	movl	%eax, -16(%ebp)
	movl	-16(%ebp), %eax
	cmpl	$0, %eax
	je	.L5
--Block::generate()
--Assignment::generate()
--Expression::generate()
--Identifier::generate()
--Identifier::generate()
	movl	_c, %eax
	movl	%eax, -8(%ebp)
--Assignment::generate()
--Expression::generate()
--Identifier::generate()
--Number::generate()
	movl	$0, %eax
	movl	%eax, _c
--Return::generate()
--Identifier::generate()
	movl	-8(%ebp), %eax
