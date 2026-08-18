	.text
	.globl	_start
_start:
	mov	0(%esp), %eax
	lea	4(%esp), %ebx

	push	%ebx
	push	%eax
	call	main
	add	$8, %esp

	mov	%eax, %ebx
	mov	$1, %eax
	int	$0x80

	hlt
