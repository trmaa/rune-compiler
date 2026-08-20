	.data
g:
	.long	10
gp:
	.long	g

str0:
	.string	"%d\n"
str1:
	.string	"%d\n"

	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	sub	$4, %esp
	mov	$20, -4(%ebp)

	sub	$4, %esp
	lea	-4(%ebp), %eax
	mov	%eax, -8(%ebp)

	sub	$4, %esp
	lea	g, %eax
	mov	%eax, -12(%ebp)

	sub	$4, %esp
	lea	gp, %eax
	mov	%eax, -16(%ebp)

	sub	$4, %esp
	mov	-16(%ebp), %eax
	mov	(%eax), %eax
	mov	%eax, -20(%ebp)

	sub	$4, %esp
	mov	-8(%ebp), %eax
	mov	(%eax), %eax
	mov	%eax, -24(%ebp)

	sub	$4, %esp
	mov	$gp, -28(%ebp)

	mov	-16(%ebp), %eax
	add	$1, %eax
	push	%eax
	push	$str0
	call	printf
	add	$8, %esp

	lea	-8(%ebp), %eax
	push	4(%eax)
	push	$str1
	call	printf
	add	$8, %esp

	mov	%ebp, %esp
	pop	%ebp
	ret
