	.data
a:
	.zero 40
b:
	.long 30
c:
	.zero 1
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
	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp
