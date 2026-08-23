	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	sub	$256, %esp

	lea	-256(%ebp), %edi
	add	$10, %edi
	mov	$0x61, %eax
	movb	%al, (%edi)

	lea	-256(%ebp), %eax
	push	%eax
	mov	$30, %eax
	mov	%eax, %ebx
	pop	%eax
	add	%ebx, %eax
	mov	%eax, %edi
	mov	$0x62, %eax
	movb	%al, (%edi)

	lea	-256(%ebp), %edi
	add	$255, %edi
	mov	$0, %eax
	movb	%al, (%edi)

	lea	-256(%ebp), %eax

	mov	%ebp, %esp
	pop	%ebp
	ret
