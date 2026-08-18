# rune compiler (RC)

It is a small compiler to create binaries in very old computers.

## Specs

There are two data types: word, and byte.

Input: .ru file, Output: x86-i386 AT&T .s file.

Variables outside of any scope, and strings are stored in .data.

Variables in a scope are stored in this way:

```asm
scope:
	push	%ebp
	mov	%esp, %ebp

	sub	$stack-size, %esp

	mov	$var0, -4(%ebp)
	mov	$var1, -8(%ebp)
	mov	$var2, -12(%ebp)
	...

	mov	%ebp, %esp
	pop	%ebp
	ret

```

Function parameters are passed inside the stack of the called function:

```asm
caller:
	...
	push	$arg1
	push	$arg0
	call	called
	...

called:
	# arg0 in %esp
	# arg1 in %esp + 4

```

The outputed assembly code uses libc, so this code:

```c
// file.z

word glob = 67;
word arr[10];

pub fn main {
	word res = glob;

	foo();

	ret res;
}

fn foo {
	printf("g: %d\n", glob);
}
```

should output this:

```asm
# file.s

	.data
glob:
	.long 67
arr:
	.zero 40

str0:
	.string "g: %d\n"

	.text
	.globl	main
main:
	push	%ebp
	mov	%esp, %ebp

	sub	$4, %esp

	mov	glob, %eax
	mov	%eax, -4(%ebp)

	call	foo

	mov	-4(%ebp), %eax

	mov	%ebp, %esp
	pop	%ebp
	ret

foo:
	push	glob
	push	$str0
	call	printf
	add	$8, %esp

	ret
```

There is a similar example at examples/.

Then, to assemble and link just:

```console
$ as --32 file.s -o file.o
$ cc -m32 -no-pie file.o
$ ./a.out
```

...
