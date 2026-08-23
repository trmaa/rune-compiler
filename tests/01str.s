	.data
str0:
	.string	"Hello\n"
arr0:
	.long	1, 2

	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	push	$str0
	call	printf
	add	$4, %esp

	push	$arr0
	call	foo
	add	$4, %esp

	mov	%ebp, %esp
	pop	%ebp
	ret
