	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	mov	$4, %esp
	mov	$60, -4(%ebp)

	mov	$4, %esp
	mov	$12, -8(%ebp)

	mov	$4, %esp
	mov	-8(%ebp), %eax
	xor	%edx, %edx
	mov	$2, %ebx
	div	%ebx
	mov	%eax, -12(%ebp)
	mov	-4(%ebp), %eax
	add	%eax, -12(%ebp)

	mov	-12(%ebp), %eax

	mov	%ebp, %esp
	pop	%ebp
	ret
