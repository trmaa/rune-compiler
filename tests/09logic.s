	.data
str0:
	.string	"bad: t&&f\n"
str1:
	.string	"ok t&&f is false\n"
str2:
	.string	"ok t||f is true\n"
str3:
	.string	"ok !f is true\n"
str4:
	.string	"bad one\n"
str5:
	.string	"ok two\n"
str6:
	.string	"bad many\n"
str7:
	.string	"ok all cmps\n"
str8:
	.string	"bad sc\n"
str9:
	.string	"sc x=%d\n"
str10:
	.string	"ok sc or\n"
str11:
	.string	"sc y=%d\n"
str12:
	.string	"sum=%d\n"
str13:
	.string	"total=%d\n"
str14:
	.string	"acc=%d\n"

	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	sub	$4, %esp
	mov	$1, -4(%ebp)

	sub	$4, %esp
	mov	$0, -8(%ebp)

	mov	-4(%ebp), %eax
	test	%eax, %eax
	je	L2
	mov	-8(%ebp), %eax
	test	%eax, %eax
	setne	%al
	movzbl	%al, %eax
	jmp	L3
L2:
	xor	%eax, %eax
L3:
	test	%eax, %eax
	je	L0

	push	$str0
	call	printf
	add	$4, %esp

	jmp	L1
L0:
	push	$str1
	call	printf
	add	$4, %esp

L1:

	mov	-4(%ebp), %eax
	test	%eax, %eax
	jne	L6
	mov	-8(%ebp), %eax
	test	%eax, %eax
	setne	%al
	movzbl	%al, %eax
	jmp	L7
L6:
	mov	$1, %eax
L7:
	test	%eax, %eax
	je	L4

	push	$str2
	call	printf
	add	$4, %esp

L4:

	mov	-8(%ebp), %eax
	test	%eax, %eax
	sete	%al
	movzbl	%al, %eax
	test	%eax, %eax
	je	L8

	push	$str3
	call	printf
	add	$4, %esp

L8:

	sub	$4, %esp
	mov	$2, -12(%ebp)

	mov	-12(%ebp), %eax
	push	%eax
	mov	$1, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	sete	%al
	movzbl	%al, %eax
	test	%eax, %eax
	je	L10

	push	$str4
	call	printf
	add	$4, %esp

	jmp	L11
L10:
	mov	-12(%ebp), %eax
	push	%eax
	mov	$2, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	sete	%al
	movzbl	%al, %eax
	test	%eax, %eax
	je	L12

	push	$str5
	call	printf
	add	$4, %esp

	jmp	L13
L12:
	push	$str6
	call	printf
	add	$4, %esp

L13:

L11:

	mov	$1, %eax
	push	%eax
	mov	$2, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setl	%al
	movzbl	%al, %eax
	test	%eax, %eax
	je	L16
	mov	$2, %eax
	push	%eax
	mov	$2, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setle	%al
	movzbl	%al, %eax
	test	%eax, %eax
	setne	%al
	movzbl	%al, %eax
	jmp	L17
L16:
	xor	%eax, %eax
L17:
	test	%eax, %eax
	je	L18
	mov	$3, %eax
	push	%eax
	mov	$2, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setg	%al
	movzbl	%al, %eax
	test	%eax, %eax
	setne	%al
	movzbl	%al, %eax
	jmp	L19
L18:
	xor	%eax, %eax
L19:
	test	%eax, %eax
	je	L20
	mov	$3, %eax
	push	%eax
	mov	$3, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setge	%al
	movzbl	%al, %eax
	test	%eax, %eax
	setne	%al
	movzbl	%al, %eax
	jmp	L21
L20:
	xor	%eax, %eax
L21:
	test	%eax, %eax
	je	L22
	mov	$1, %eax
	push	%eax
	mov	$2, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setne	%al
	movzbl	%al, %eax
	test	%eax, %eax
	setne	%al
	movzbl	%al, %eax
	jmp	L23
L22:
	xor	%eax, %eax
L23:
	test	%eax, %eax
	je	L24
	mov	$1, %eax
	push	%eax
	mov	$1, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	sete	%al
	movzbl	%al, %eax
	test	%eax, %eax
	setne	%al
	movzbl	%al, %eax
	jmp	L25
L24:
	xor	%eax, %eax
L25:
	test	%eax, %eax
	je	L14

	push	$str7
	call	printf
	add	$4, %esp

L14:

	sub	$4, %esp
	mov	$0, -16(%ebp)

	mov	-8(%ebp), %eax
	test	%eax, %eax
	je	L28
	incl	-16(%ebp)
	mov	-16(%ebp), %eax
	test	%eax, %eax
	setne	%al
	movzbl	%al, %eax
	jmp	L29
L28:
	xor	%eax, %eax
L29:
	test	%eax, %eax
	je	L26

	push	$str8
	call	printf
	add	$4, %esp

L26:

	mov	-16(%ebp), %eax
	push	%eax
	push	$str9
	call	printf
	add	$8, %esp

	sub	$4, %esp
	mov	$0, -20(%ebp)

	mov	-4(%ebp), %eax
	test	%eax, %eax
	jne	L32
	incl	-20(%ebp)
	mov	-20(%ebp), %eax
	test	%eax, %eax
	setne	%al
	movzbl	%al, %eax
	jmp	L33
L32:
	mov	$1, %eax
L33:
	test	%eax, %eax
	je	L30

	push	$str10
	call	printf
	add	$4, %esp

L30:

	mov	-20(%ebp), %eax
	push	%eax
	push	$str11
	call	printf
	add	$8, %esp

	sub	$4, %esp
	mov	$0, -24(%ebp)

	sub	$4, %esp
	mov	$0, -28(%ebp)

L34:
	mov	-24(%ebp), %eax
	push	%eax
	mov	$10, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setl	%al
	movzbl	%al, %eax
	test	%eax, %eax
	je	L35

	mov	-24(%ebp), %eax
	mov	%eax, %ebx
	mov	-28(%ebp), %eax
	add	%ebx, %eax
	mov	%eax, -28(%ebp)

	incl	-24(%ebp)

	jmp	L34
L35:

	mov	-28(%ebp), %eax
	push	%eax
	push	$str12
	call	printf
	add	$8, %esp

	sub	$4, %esp

	sub	$4, %esp
	mov	$0, -36(%ebp)

	mov	$0, %eax
	mov	%eax, -32(%ebp)

L36:
	mov	-32(%ebp), %eax
	push	%eax
	mov	$5, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setl	%al
	movzbl	%al, %eax
	test	%eax, %eax
	je	L37
	mov	-32(%ebp), %eax
	push	%eax
	mov	$2, %eax
	mov	%eax, %ebx
	pop	%eax
	mul	%ebx
	mov	%eax, %ebx
	mov	-36(%ebp), %eax
	add	%ebx, %eax
	mov	%eax, -36(%ebp)

	incl	-32(%ebp)

	jmp	L36
L37:

	mov	-36(%ebp), %eax
	push	%eax
	push	$str13
	call	printf
	add	$8, %esp

	sub	$4, %esp

	sub	$4, %esp

	sub	$4, %esp
	mov	$0, -48(%ebp)

	mov	$1, %eax
	mov	%eax, -40(%ebp)

L38:
	mov	-40(%ebp), %eax
	push	%eax
	mov	$3, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setle	%al
	movzbl	%al, %eax
	test	%eax, %eax
	je	L39
	mov	$1, %eax
	mov	%eax, -44(%ebp)

L40:
	mov	-44(%ebp), %eax
	push	%eax
	mov	$3, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setle	%al
	movzbl	%al, %eax
	test	%eax, %eax
	je	L41
	mov	-40(%ebp), %eax
	push	%eax
	mov	-44(%ebp), %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	sete	%al
	movzbl	%al, %eax
	test	%eax, %eax
	je	L42

	mov	-40(%ebp), %eax
	push	%eax
	mov	-44(%ebp), %eax
	mov	%eax, %ebx
	pop	%eax
	mul	%ebx
	mov	%eax, %ebx
	mov	-48(%ebp), %eax
	add	%ebx, %eax
	mov	%eax, -48(%ebp)

L42:

	incl	-44(%ebp)

	jmp	L40
L41:

	incl	-40(%ebp)

	jmp	L38
L39:

	mov	-48(%ebp), %eax
	push	%eax
	push	$str14
	call	printf
	add	$8, %esp

	mov	$0, %eax

	mov	%ebp, %esp
	pop	%ebp
	ret
