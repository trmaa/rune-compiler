	.text
	.globl	exit
exit:
	mov	4(%esp), %ebx
	mov	$1, %eax
	int	$0x80
