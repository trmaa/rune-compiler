	.data
str0:
	.string "the prog name is %s\n"

	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	mov	12(%ebp), %eax
	push	(%eax)
	push	$str0
	call	printf
	add	$8, %esp

	push	$-11
	push	$70
	push	$10
	call	sum3
	add	$12, %esp

	sub	$4, %esp
	mov	%eax, -4(%ebp)

	mov	-4(%ebp), %eax

	mov	%ebp, %esp
	pop	%ebp
	ret

sum3:
	push	%ebp
	mov	%esp, %ebp

	push	12(%ebp)
	push	8(%ebp)
	call	sum2
	add	$8, %esp

	push	16(%ebp)
	push	%eax
	call	sum2
	add	$8, %esp

	mov	%eax, %eax

	mov	%ebp, %esp
	pop	%ebp
	ret

sum2:
	push	%ebp
	mov	%esp, %ebp

	mov	8(%ebp), %eax
	add	12(%ebp), %eax

	mov	%ebp, %esp
	pop	%ebp
	ret

