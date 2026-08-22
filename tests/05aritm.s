	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	sub	$4, %esp
	mov	$60, -4(%ebp)

	sub	$4, %esp
	mov	$12, -8(%ebp)

	sub	$4, %esp
	mov	-4(%ebp), %eax
	push	%eax
	mov	-8(%ebp), %eax
	push	%eax
	mov	$2, %eax
	mov	%eax, %ebx
	pop	%eax
	xor	%edx, %edx
	div	%ebx
	mov	%eax, %ebx
	pop	%eax
	add	%ebx, %eax
	mov	%eax, -12(%ebp)

	mov	-12(%ebp), %eax

	mov	%ebp, %esp
	pop	%ebp
	ret
