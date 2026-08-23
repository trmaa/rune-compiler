	.data
str0:
	.string	"Hola!"

	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	mov	$4, %eax
	push	%eax
	mov	$1, %eax
	push	%eax
	push	$str0
	mov	$5, %eax
	push	%eax
	pop	%edx
	pop	%ecx
	pop	%ebx
	pop	%eax
	int	$0x80

	mov	%ebp, %esp
	pop	%ebp
	ret
