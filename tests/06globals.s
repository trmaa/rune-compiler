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
	push	%eax
	mov	$2, %eax
	mov	%eax, %ebx
	pop	%eax
	xor	%edx, %edx
	div	%ebx
	mov	%eax, -8(%ebp)

	mov	g, %eax
	push	%eax
	mov	-4(%ebp), %eax
	mov	%eax, %ebx
	pop	%eax
	add	%ebx, %eax
	push	%eax
	mov	-8(%ebp), %eax
	mov	%eax, %ebx
	pop	%eax
	sub	%ebx, %eax

	mov	%ebp, %esp
	pop	%ebp
	ret
