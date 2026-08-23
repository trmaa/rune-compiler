	.text
	.globl	_start
_start:
	mov	0(%esp), %eax
	lea	4(%esp), %ebx

	push	%ebx
	push	%eax
	call	main
	add	$8, %esp

	push	%eax
	call	exit
	hlt
