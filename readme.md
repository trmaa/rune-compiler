# rune compiler (RC)

Lightwaight programming language for x86-i386 computers and above.

Resembles assembly but has loops, variables, and pointers.

See examples at tests/.

## Done for now

- Tokenizer

## How to

### Build

```console
$ make # compile source into ./rc
$ make debug f=file.ru # see all debug logs of rc after compiling file.ru
$ make install # install the compiler
```

### Usage

```console
$ ./rc -help
Usage: rc [options] [file.ru]
Options:
        -h print this text.
        -v show the version.
```

## Specs

There are two data types: setw, and byte.

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

```rust
// file.ru

setw glob = 67;
setw arr[10];

pub fn main {
	setw res = glob;

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

There are a similar example at tests/.

Then, to assemble and link just:

```console
$ as --32 file.s -o file.o
$ cc -m32 -no-pie file.o
$ ./a.out
```

...
