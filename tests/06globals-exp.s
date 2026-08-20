	.data
g:
	.long	40

	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	sub	$4, %esp
	mov	g, %eax
	mov	%eax, -4(%ebp)

	sub	$4, %esp
	mov	g, %eax
	xor	%edx, %edx
	mov	$2, %ebx
	div	%ebx
	mov	%eax, -8(%ebp)

	mov	g, %eax
	add	-4(%ebp), %eax
	sub	-8(%ebp), %eax

	mov	%ebp, %esp
	pop	%ebp
	ret
