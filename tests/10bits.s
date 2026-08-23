	.data
str0:
	.string	"and=%d\n"
str1:
	.string	"or=%d\n"
str2:
	.string	"xor=%d\n"
str3:
	.string	"not=%d\n"
str4:
	.string	"shl=%d\n"
str5:
	.string	"shr=%d\n"
str6:
	.string	"eq=%d ne=%d\n"
str7:
	.string	"lt=%d le=%d gt=%d ge=%d\n"
str8:
	.string	"prec=%d\n"
str9:
	.string	"shiftprec=%d\n"
str10:
	.string	"cand=%d\n"
str11:
	.string	"cor=%d\n"
str12:
	.string	"cxor=%d\n"
str13:
	.string	"notnot=%d\n"
str14:
	.string	"neg=%d\n"
str15:
	.string	"negvar=%d\n"
str16:
	.string	"ok truthy\n"
str17:
	.string	"ok nor\n"

	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	sub	$4, %esp
	mov	$240, -4(%ebp)

	sub	$4, %esp
	mov	$15, -8(%ebp)

	mov	-4(%ebp), %eax
	push	%eax
	mov	-8(%ebp), %eax
	mov	%eax, %ebx
	pop	%eax
	and	%ebx, %eax
	push	%eax
	push	$str0
	call	printf
	add	$8, %esp

	mov	-4(%ebp), %eax
	push	%eax
	mov	-8(%ebp), %eax
	mov	%eax, %ebx
	pop	%eax
	or	%ebx, %eax
	push	%eax
	push	$str1
	call	printf
	add	$8, %esp

	mov	-4(%ebp), %eax
	push	%eax
	mov	-8(%ebp), %eax
	mov	%eax, %ebx
	pop	%eax
	xor	%ebx, %eax
	push	%eax
	push	$str2
	call	printf
	add	$8, %esp

	mov	-4(%ebp), %eax
	not	%eax
	push	%eax
	push	$str3
	call	printf
	add	$8, %esp

	mov	$1, %eax
	push	%eax
	mov	$4, %eax
	mov	%eax, %ecx
	pop	%eax
	shl	%cl, %eax
	push	%eax
	push	$str4
	call	printf
	add	$8, %esp

	mov	$-16, %eax
	push	%eax
	mov	$2, %eax
	mov	%eax, %ecx
	pop	%eax
	sar	%cl, %eax
	push	%eax
	push	$str5
	call	printf
	add	$8, %esp

	mov	$3, %eax
	push	%eax
	mov	$3, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setne	%al
	movzbl	%al, %eax
	push	%eax
	mov	$3, %eax
	push	%eax
	mov	$3, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	sete	%al
	movzbl	%al, %eax
	push	%eax
	push	$str6
	call	printf
	add	$12, %esp

	mov	$2, %eax
	push	%eax
	mov	$3, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setge	%al
	movzbl	%al, %eax
	push	%eax
	mov	$3, %eax
	push	%eax
	mov	$2, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setg	%al
	movzbl	%al, %eax
	push	%eax
	mov	$2, %eax
	push	%eax
	mov	$2, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setle	%al
	movzbl	%al, %eax
	push	%eax
	mov	$1, %eax
	push	%eax
	mov	$2, %eax
	mov	%eax, %ebx
	pop	%eax
	cmp	%ebx, %eax
	setl	%al
	movzbl	%al, %eax
	push	%eax
	push	$str7
	call	printf
	add	$20, %esp

	mov	$1, %eax
	push	%eax
	mov	$2, %eax
	push	%eax
	mov	$2, %eax
	push	%eax
	mov	$3, %eax
	mov	%eax, %ebx
	pop	%eax
	and	%ebx, %eax
	mov	%eax, %ebx
	pop	%eax
	xor	%ebx, %eax
	mov	%eax, %ebx
	pop	%eax
	or	%ebx, %eax
	push	%eax
	push	$str8
	call	printf
	add	$8, %esp

	mov	$1, %eax
	push	%eax
	mov	$2, %eax
	push	%eax
	mov	$1, %eax
	mov	%eax, %ebx
	pop	%eax
	add	%ebx, %eax
	mov	%eax, %ecx
	pop	%eax
	shl	%cl, %eax
	push	%eax
	push	$str9
	call	printf
	add	$8, %esp

	sub	$4, %esp
	mov	$12, -12(%ebp)

	mov	$10, %eax
	mov	%eax, %ebx
	mov	-12(%ebp), %eax
	and	%ebx, %eax
	mov	%eax, -12(%ebp)

	mov	-12(%ebp), %eax
	push	%eax
	push	$str10
	call	printf
	add	$8, %esp

	mov	$1, %eax
	mov	%eax, %ebx
	mov	-12(%ebp), %eax
	or	%ebx, %eax
	mov	%eax, -12(%ebp)

	mov	-12(%ebp), %eax
	push	%eax
	push	$str11
	call	printf
	add	$8, %esp

	mov	$15, %eax
	mov	%eax, %ebx
	mov	-12(%ebp), %eax
	xor	%ebx, %eax
	mov	%eax, -12(%ebp)

	mov	-12(%ebp), %eax
	push	%eax
	push	$str12
	call	printf
	add	$8, %esp

	mov	$5, %eax
	test	%eax, %eax
	sete	%al
	movzbl	%al, %eax
	test	%eax, %eax
	sete	%al
	movzbl	%al, %eax
	push	%eax
	push	$str13
	call	printf
	add	$8, %esp

	mov	$-7, %eax
	push	%eax
	push	$str14
	call	printf
	add	$8, %esp

	sub	$4, %esp
	mov	$9, -16(%ebp)

	mov	-16(%ebp), %eax
	neg	%eax
	push	%eax
	push	$str15
	call	printf
	add	$8, %esp

	mov	$5, %eax
	test	%eax, %eax
	je	L2
	mov	$-3, %eax
	test	%eax, %eax
	setne	%al
	movzbl	%al, %eax
	jmp	L3
L2:
	xor	%eax, %eax
L3:
	test	%eax, %eax
	je	L0

	push	$str16
	call	printf
	add	$4, %esp

L0:

	mov	$0, %eax
	test	%eax, %eax
	jne	L6
	mov	$0, %eax
	test	%eax, %eax
	setne	%al
	movzbl	%al, %eax
	jmp	L7
L6:
	mov	$1, %eax
L7:
	test	%eax, %eax
	sete	%al
	movzbl	%al, %eax
	test	%eax, %eax
	je	L4

	push	$str17
	call	printf
	add	$4, %esp

L4:

	mov	$0, %eax

	mov	%ebp, %esp
	pop	%ebp
	ret
