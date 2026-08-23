	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp


		mov	$1, %eax
		mov	$0, %ebx
		int	$0x80
	

	mov	%ebp, %esp
	pop	%ebp
	ret
