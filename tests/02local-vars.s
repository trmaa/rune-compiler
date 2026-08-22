	.data
arr0:
	.byte	97, 2
str0:
	.string	"str"

	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	sub	$4, %esp
	mov	$10, -4(%ebp)

	sub	$4, %esp
	lea	-4(%ebp), %eax
	mov	%eax, -8(%ebp)

	sub	$4, %esp
	lea	arr0, %eax
	mov	%eax, -12(%ebp)

	sub	$4, %esp
	movb	$0x1b, -16(%ebp)

	sub	$4, %esp
	lea	str0, %eax
	movb	%al, -20(%ebp)

	sub	$4, %esp
	call	foo

	mov	%eax, -24(%ebp)

	mov	%ebp, %esp
	pop	%ebp
	ret
