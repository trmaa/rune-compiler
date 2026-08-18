	.data
	.text
a:
	.zero 40
b:
	.long 30
c:
	.zero 1
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp
	sub	$108, %esp
	mov	b, %eax
	push	%eax
	mov	$70, %eax
	pop	%ebx
	add	%ebx, %eax
	mov	%eax, -4(%ebp)
	mov	$1, -92(%ebp)
	mov	$2, -88(%ebp)
	push	-4(%ebp)
	push	$str0
	call	printf
	add	$8, %esp
	mov	$0, %eax
	mov	%eax, -96(%ebp)
.L0:
	mov	-96(%ebp), %eax
	push	%eax
	mov	$2, %eax
	pop	%ebx
	cmp	%eax, %ebx
	setl	%al
	movzbl	%al, %eax
	cmp	$0, %eax
	je	.L1
	push	$0
	push	-96(%ebp)
	push	$0
	push	-92(%ebp)
	push	-96(%ebp)
	push	$str1
	call	printf
	add	$24, %esp
	mov	-96(%ebp), %eax
	push	%eax
	mov	$1, %eax
	pop	%ebx
	add	%ebx, %eax
	mov	%eax, -96(%ebp)
	jmp	.L0
.L1:
	mov	$10, %eax
	mov	%eax, -100(%ebp)
	mov	-100(%ebp), %eax
	push	%eax
	mov	$5, %eax
	pop	%ebx
	cmp	%eax, %ebx
	setg	%al
	movzbl	%al, %eax
	cmp	$0, %eax
	je	.L2
	push	$str2
	call	printf
	add	$4, %esp
	jmp	.L3
.L2:
	push	$str3
	call	printf
	add	$4, %esp
.L3:
	mov	$0, %eax
	mov	%eax, -104(%ebp)
.L4:
	mov	-104(%ebp), %eax
	push	%eax
	mov	$3, %eax
	pop	%ebx
	cmp	%eax, %ebx
	setl	%al
	movzbl	%al, %eax
	cmp	$0, %eax
	je	.L5
	push	-104(%ebp)
	push	$str4
	call	printf
	add	$8, %esp
	mov	-104(%ebp), %eax
	push	%eax
	mov	$1, %eax
	pop	%ebx
	add	%ebx, %eax
	mov	%eax, -104(%ebp)
	jmp	.L4
.L5:
	mov	$0, %eax
	mov	%eax, -108(%ebp)
.L6:
	mov	-108(%ebp), %eax
	push	%eax
	mov	$3, %eax
	pop	%ebx
	cmp	%eax, %ebx
	setl	%al
	movzbl	%al, %eax
	cmp	$0, %eax
	je	.L7
	push	-108(%ebp)
	push	$str5
	call	printf
	add	$8, %esp
	mov	-108(%ebp), %eax
	push	%eax
	mov	$1, %eax
	pop	%ebx
	add	%ebx, %eax
	mov	%eax, -108(%ebp)
	jmp	.L6
.L7:
	mov	$69, %eax
	mov	%ebp, %esp
	pop	%ebp
	ret
str0:
	.string "d: %d\n"
str1:
	.string "f[%d]: %d\n"
str2:
	.string "10 is greater than 5\n"
str3:
	.string "10 is not greater than 5\n"
str4:
	.string "i: %d\n"
str5:
	.string "j: %d\n"
