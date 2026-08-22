# rune compiler (RC)

Lightwaight programming language for x86-i386 computers and above.

Resembles assembly but has loops, variables, and pointers.

See examples at tests/.

![](logo.png)

## Done for now

- Tokenizer
- Global variables (.data section)
- Float literals (3.14) and floats in words
- Functions, calls and string/array literals (.text section)
- Local variables, arithmetic expressions (with precedence) and return statements
- Control flow: `if`/`else`, `while`, and `for init, cond, step`
- Comparisons (`== != < > <= >=`), boolean logic with short-circuit (`&& || !`)
  and bitwise operators (`& | ^ ~ << >>`) in every expression
- Syntax highlighting on vim

## To do

- Structs
- Integrate gcc preprocessor, assembler, and linker by forking and execing

## How to

### Build

```console
$ make # compile source into ./rc
$ make install # install the compiler and manpages
$ make install-syntax # install syntax for vim
```

### Usage

```console
$ rc -help
Usage: rc [options] [file.rn]
Options:
        -h print this text.
        -v show the version.
$ rc file.rn # compiles file.rn into file.s
$ cc -m32 -no-pie file.s # assembles and links the binarie with libc and outputs a program
$ ./a.out # run the program
$ make debug f=file.rn # see all debug logs of rc after compiling file.rn
```

Alternatively, if you don't need libc, just:

```console
$ rc file.rn
$ as --32 file.s -o file.o # assembles
$ ld -m elf_i386 -no-pie file.s # link
$ ./a.out
```

You can use macros, defines, includes, and enums by using the GCC preprocessor too.

```console
$ cc -E -P -x c file.rn -o file-prep.rn
$ rc file-prep.rn
$ cc -m32 -no-pie file-prep.s
$ ./a.out
```

## Specs

There are two data types: word, and byte (plus the pointers to each, of size word- 32 bits-). A word can also hold a float (32 bits).

### Pointers

Declare them with stars after the type: `let *p` is a pointer
to word, `leb *bp` a pointer to byte, and each extra star adds
a level (`leb **pp` is a pointer to a pointer to byte).

- Address-of with `&var`, dereference with `*p`, indexing with
  `p[i]` (stride 1 on byte pointers, 4 on word pointers).
  `&arr[i]` is the base plus a constant offset.
- Local arrays reserve their full size on the stack: `leb buf[256]`
  is a 256-byte buffer and its name decays to the address of
  element 0, so it works like a pointer to that buffer (`buf[i]`,
  `*(buf + n)`, passing it to functions, returning it). With an
  initializer (`leb s[8] = "hi"`) the dimensions are ignored and
  the address of the literal is stored, as before.
- Stores through pointers: `*p = v` and `p[i] = v`. Byte
  pointers store one byte (movb), word pointers four.
- Compound assignment works everywhere: `x += 1`, `*p += 2`,
  `p[i] -= 3`, with `+= -= *= /= &= |= ^=`.
- Pointer arithmetic is raw: `p + 1` adds exactly 1, like
  assembly. Bytes load as unsigned (zero extended).

### Control flow and expressions

Bodies are a block between braces or the single statement
that follows. No parentheses around conditions.

```
if a > b          while i < n       for i = 0, i < n, i++
	printf(..)    	printf(..)    	printf(..)
else
	printf(..)
```

Expressions follow C precedence: `|| && | ^ & == != < > <= >=
<< >> + - * / %` plus unary `! ~ - * &`. Calls are
expressions too, so `let x = foo(2) + bar(3)` works. `&&` and
`||` short-circuit. `i++`/`++i` work as statements, in call
arguments and in the `for` step.

Input: .rn file, Output: x86-i386 AT&T .s file.

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
// file.rn

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
...
