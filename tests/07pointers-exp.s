	.data
g:
	.long	10
gp:
	.long	g
cells:
	.zero	16
bytes:
	.zero	4
str0:
	.string	"%d %d %d %c%c\n"
str1:
	.string	"%d\n"

	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	sub	$4, %esp
	mov	$20, -4(%ebp)

	sub	$4, %esp
	lea	-4(%ebp), %eax
	mov	%eax, -8(%ebp)

	sub	$4, %esp

	lea	g, %eax
	mov	%eax, -12(%ebp)

	sub	$4, %esp
	lea	gp, %eax
	mov	%eax, -16(%ebp)

	sub	$4, %esp
	mov	-16(%ebp), %eax
	mov	(%eax), %eax
	mov	%eax, -20(%ebp)

	sub	$4, %esp
	mov	-8(%ebp), %eax
	mov	(%eax), %eax
	mov	%eax, -24(%ebp)

	sub	$4, %esp
	push	%eax
	mov	gp, %ecx
	movzbl	(%ecx), %eax
	pop	%eax
	mov	%eax, -28(%ebp)

	sub	$4, %esp
	lea	cells, %eax
	mov	%eax, -32(%ebp)

	sub	$4, %esp
	lea	bytes, %eax
	mov	%eax, -36(%ebp)

	mov	-8(%ebp), %edi
	mov	$25, %eax
	mov	%eax, (%edi)

	mov	gp, %edi
	mov	$1, %ebx
	movzbl	(%edi), %eax
	add	%ebx, %eax
	movb	%al, (%edi)

	mov	-32(%ebp), %edi
	add	$8, %edi
	mov	$7, %eax
	mov	%eax, (%edi)

	mov	-36(%ebp), %edi
	mov	$0x41, %eax
	movb	%al, (%edi)

	mov	-36(%ebp), %edi
	add	$1, %edi
	mov	$0x42, %eax
	movb	%al, (%edi)

	mov	-36(%ebp), %ecx
	movzbl	1(%ecx), %eax
	push	%eax
	mov	-36(%ebp), %eax
	movzbl	(%eax), %eax
	push	%eax
	mov	-32(%ebp), %ecx
	mov	8(%ecx), %eax
	push	%eax
	mov	gp, %eax
	movzbl	(%eax), %eax
	push	%eax
	mov	-8(%ebp), %eax
	mov	(%eax), %eax
	push	%eax
	push	$str0
	call	printf
	add	$24, %esp

	mov	-8(%ebp), %eax
	mov	%eax, %ecx
	mov	$1, %eax
	add	%ecx, %eax
	mov	(%eax), %eax
	push	%eax
	push	$str1
	call	printf
	add	$8, %esp

	mov	%ebp, %esp
	pop	%ebp
	ret
