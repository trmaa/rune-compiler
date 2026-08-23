	.data
str0:
	.string	"the prog name is %s\n"

	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	mov	12(%ebp), %eax
	mov	(%eax), %eax
	push	%eax
	push	$str0
	call	printf
	add	$8, %esp

	sub	$4, %esp
	mov	$-11, %eax
	push	%eax
	mov	$70, %eax
	push	%eax
	mov	$10, %eax
	push	%eax
	call	sum3
	add	$12, %esp

	mov	%eax, -4(%ebp)

	mov	-4(%ebp), %eax

	mov	%ebp, %esp
	pop	%ebp
	ret
sum3:
	push	%ebp
	mov	%esp, %ebp

	mov	16(%ebp), %eax
	push	%eax
	mov	12(%ebp), %eax
	push	%eax
	mov	8(%ebp), %eax
	push	%eax
	call	sum2
	add	$8, %esp

	push	%eax
	call	sum2
	add	$8, %esp


	mov	%ebp, %esp
	pop	%ebp
	ret
	.globl	sum2
sum2:
	push	%ebp
	mov	%esp, %ebp

	mov	8(%ebp), %eax
	push	%eax
	mov	12(%ebp), %eax
	mov	%eax, %ebx
	pop	%eax
	add	%ebx, %eax

	mov	%ebp, %esp
	pop	%ebp
	ret
