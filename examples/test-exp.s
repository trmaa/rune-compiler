	.data
a:
	.zero 40
b:
	.long 30
c:
	.zero 0

str0:
	.string "d: %d\n"

	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	sub	$92, %esp

	mov	b, %eax
	mov	%eax, -4(%ebp)
	mov	$1, -8(%ebp)
	mov	$2, -12(%ebp)

	push	-4(%ebp)
	push	$str0
	call	printf
	add	$8, %esp

	mov	$69, %eax

	mov	%ebp, %esp
	pop	%ebp
	ret
