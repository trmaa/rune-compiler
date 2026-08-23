	.data
a:
	.long	0
b:
	.long	0
str0:
	.string	"two args needed\n"
str1:
	.string	"a bigger\n"
str2:
	.string	"b bigger\n"
str3:
	.string	"%d\n"
str4:
	.string	"%d\n"

	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	mov	8(%ebp), %eax
	push	%eax
	mov	$3, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setl	%al
	movzbl	%al, %eax
	test	%eax, %eax
	je	L0

	push	$str0
	call	printf
	add	$4, %esp

	mov	$1, %eax

L0:

	mov	12(%ebp), %eax
	add	$4, %eax
	mov	(%eax), %eax
	push	%eax
	call	atoi
	add	$4, %esp

	mov	%eax, a

	mov	12(%ebp), %eax
	add	$8, %eax
	mov	(%eax), %eax
	push	%eax
	call	atoi
	add	$4, %esp

	mov	%eax, b

	mov	a, %eax
	push	%eax
	mov	b, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setg	%al
	movzbl	%al, %eax
	test	%eax, %eax
	je	L2

	push	$str1
	call	printf
	add	$4, %esp

	jmp	L3
L2:
	push	$str2
	call	printf
	add	$4, %esp

L3:

	sub	$4, %esp
	mov	$0, -4(%ebp)

L4:
	mov	-4(%ebp), %eax
	push	%eax
	mov	$10, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setl	%al
	movzbl	%al, %eax
	test	%eax, %eax
	je	L5

	incl	-4(%ebp)
	mov	-4(%ebp), %eax
	push	%eax
	push	$str3
	call	printf
	add	$8, %esp

	jmp	L4
L5:

	mov	$0, %eax
	mov	%eax, -4(%ebp)

L6:
	mov	-4(%ebp), %eax
	push	%eax
	mov	$10, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setl	%al
	movzbl	%al, %eax
	test	%eax, %eax
	je	L7
	mov	-4(%ebp), %eax
	push	%eax
	push	$str4
	call	printf
	add	$8, %esp

	incl	-4(%ebp)

	jmp	L6
L7:

	mov	%ebp, %esp
	pop	%ebp
	ret
