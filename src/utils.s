	.text
# void swap(int *a, int *b)
	.globl	swap
swap:
	push	%ebx

	mov	8(%esp), %eax
	mov	12(%esp), %ebx

	mov	(%eax), %ecx
	mov	(%ebx), %edx

	mov	%edx, (%eax)
	mov	%ecx, (%ebx)

	pop	%ebx

	ret
