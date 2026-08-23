	.data
a:
	.long	0
	.globl	b
b:
	.zero	200
c:
	.byte	1, 2
str:
	.long	str0
str0:
	.string	"Hello"
	.globl	d
d:
	.long	a
	.globl	e
e:
	.byte	10
