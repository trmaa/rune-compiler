	.data
str0:
	.string	"str"
arr0:
	.byte	'a', 2

	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	sub	$4, %esp
	mov	$10, -4(%ebp)

	sub	$4, %esp
	lea	-4(%ebp), -8(%ebp)

	sub	$4, %esp
	lea	$arr0, %eax
	mov	%eax, -12(%ebp)

	sub	$4, %esp
	movb	$0x1b, -16(%ebp)

	sub	$4, %esp
	lea	$str0, %eax
	mov	%eax, -20(%ebp)

	call	foo
	sub	$4, %esp
	mov	%eax, -24(%ebp)

	mov	%ebp, %esp
	pop	%ebp
	ret
