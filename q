[1mdiff --git a/examples/comp b/examples/comp[m
[1mindex 8cb9289..d1e332b 100755[m
[1m--- a/examples/comp[m
[1m+++ b/examples/comp[m
[36m@@ -1,7 +1,4 @@[m
 #! /usr/bin/env bash[m
 [m
[31m-as --32 test-exp.s -o test-exp.o[m
[31m-cc -m32 -no-pie test-exp.o -o test-exp[m
[31m-[m
 as --32 test.s -o test.o[m
 cc -m32 -no-pie test.o -o test[m
[1mdiff --git a/examples/test b/examples/test[m
[1mindex 471ebfd..3f52a9c 100755[m
Binary files a/examples/test and b/examples/test differ
[1mdiff --git a/examples/test-exp b/examples/test-exp[m
[1mdeleted file mode 100755[m
[1mindex f59b8cc..0000000[m
Binary files a/examples/test-exp and /dev/null differ
[1mdiff --git a/examples/test-exp.o b/examples/test-exp.o[m
[1mdeleted file mode 100644[m
[1mindex d259f02..0000000[m
Binary files a/examples/test-exp.o and /dev/null differ
[1mdiff --git a/examples/test-exp.s b/examples/test-exp.s[m
[1mdeleted file mode 100644[m
[1mindex 7f745c6..0000000[m
[1m--- a/examples/test-exp.s[m
[1m+++ /dev/null[m
[36m@@ -1,34 +0,0 @@[m
[31m-	.data[m
[31m-a:[m
[31m-	.zero 40[m
[31m-b:[m
[31m-	.long 30[m
[31m-c:[m
[31m-	.zero 0[m
[31m-[m
[31m-str0:[m
[31m-	.string "d: %d\n"[m
[31m-[m
[31m-	.text[m
[31m-	.globl	main[m
[31m-main:[m
[31m-	push	%ebp[m
[31m-	mov	%esp, %ebp[m
[31m-[m
[31m-	sub	$92, %esp[m
[31m-[m
[31m-	mov	b, %eax[m
[31m-	mov	%eax, -4(%ebp)[m
[31m-	mov	$1, -8(%ebp)[m
[31m-	mov	$2, -12(%ebp)[m
[31m-[m
[31m-	push	-4(%ebp)[m
[31m-	push	$str0[m
[31m-	call	printf[m
[31m-	add	$8, %esp[m
[31m-[m
[31m-	mov	$69, %eax[m
[31m-[m
[31m-	mov	%ebp, %esp[m
[31m-	pop	%ebp[m
[31m-	ret[m
[1mdiff --git a/examples/test.o b/examples/test.o[m
[1mindex c19de4c..5661ce9 100644[m
Binary files a/examples/test.o and b/examples/test.o differ
[1mdiff --git a/examples/test.s b/examples/test.s[m
[1mindex 87b47e1..9189966 100644[m
[1m--- a/examples/test.s[m
[1m+++ b/examples/test.s[m
[36m@@ -5,13 +5,17 @@[m [ma:[m
 b:[m
 	.long 30[m
 c:[m
[31m-	.zero 0[m
[32m+[m	[32m.zero 1[m
 	.globl	main[m
 main:[m
 	push	%ebp[m
 	mov	%esp, %ebp[m
[31m-	sub	$92, %esp[m
[32m+[m	[32msub	$108, %esp[m
 	mov	b, %eax[m
[32m+[m	[32mpush	%eax[m
[32m+[m	[32mmov	$70, %eax[m
[32m+[m	[32mpop	%ebx[m
[32m+[m	[32madd	%ebx, %eax[m
 	mov	%eax, -4(%ebp)[m
 	mov	$1, -92(%ebp)[m
 	mov	$2, -88(%ebp)[m
[36m@@ -19,9 +23,115 @@[m [mmain:[m
 	push	$str0[m
 	call	printf[m
 	add	$8, %esp[m
[32m+[m	[32mmov	$0, %eax[m
[32m+[m	[32mmov	%eax, -96(%ebp)[m
[32m+[m[32m.L0:[m
[32m+[m	[32mmov	-96(%ebp), %eax[m
[32m+[m	[32mpush	%eax[m
[32m+[m	[32mmov	$2, %eax[m
[32m+[m	[32mpop	%ebx[m
[32m+[m	[32mcmp	%eax, %ebx[m
[32m+[m	[32msetl	%al[m
[32m+[m	[32mmovzbl	%al, %eax[m
[32m+[m	[32mcmp	$0, %eax[m
[32m+[m	[32mje	.L1[m
[32m+[m	[32mpush	$0[m
[32m+[m	[32mpush	-96(%ebp)[m
[32m+[m	[32mpush	$0[m
[32m+[m	[32mpush	-92(%ebp)[m
[32m+[m	[32mpush	-96(%ebp)[m
[32m+[m	[32mpush	$str1[m
[32m+[m	[32mcall	printf[m
[32m+[m	[32madd	$24, %esp[m
[32m+[m	[32mmov	-96(%ebp), %eax[m
[32m+[m	[32mpush	%eax[m
[32m+[m	[32mmov	$1, %eax[m
[32m+[m	[32mpop	%ebx[m
[32m+[m	[32madd	%ebx, %eax[m
[32m+[m	[32mmov	%eax, -96(%ebp)[m
[32m+[m	[32mjmp	.L0[m
[32m+[m[32m.L1:[m
[32m+[m	[32mmov	$10, %eax[m
[32m+[m	[32mmov	%eax, -100(%ebp)[m
[32m+[m	[32mmov	-100(%ebp), %eax[m
[32m+[m	[32mpush	%eax[m
[32m+[m	[32mmov	$5, %eax[m
[32m+[m	[32mpop	%ebx[m
[32m+[m	[32mcmp	%eax, %ebx[m
[32m+[m	[32msetg	%al[m
[32m+[m	[32mmovzbl	%al, %eax[m
[32m+[m	[32mcmp	$0, %eax[m
[32m+[m	[32mje	.L2[m
[32m+[m	[32mpush	$str2[m
[32m+[m	[32mcall	printf[m
[32m+[m	[32madd	$4, %esp[m
[32m+[m	[32mjmp	.L3[m
[32m+[m[32m.L2:[m
[32m+[m	[32mpush	$str3[m
[32m+[m	[32mcall	printf[m
[32m+[m	[32madd	$4, %esp[m
[32m+[m[32m.L3:[m
[32m+[m	[32mmov	$0, %eax[m
[32m+[m	[32mmov	%eax, -104(%ebp)[m
[32m+[m[32m.L4:[m
[32m+[m	[32mmov	-104(%ebp), %eax[m
[32m+[m	[32mpush	%eax[m
[32m+[m	[32mmov	$3, %eax[m
[32m+[m	[32mpop	%ebx[m
[32m+[m	[32mcmp	%eax, %ebx[m
[32m+[m	[32msetl	%al[m
[32m+[m	[32mmovzbl	%al, %eax[m
[32m+[m	[32mcmp	$0, %eax[m
[32m+[m	[32mje	.L5[m
[32m+[m	[32mpush	-104(%ebp)[m
[32m+[m	[32mpush	$str4[m
[32m+[m	[32mcall	printf[m
[32m+[m	[32madd	$8, %esp[m
[32m+[m	[32mmov	-104(%ebp), %eax[m
[32m+[m	[32mpush	%eax[m
[32m+[m	[32mmov	$1, %eax[m
[32m+[m	[32mpop	%ebx[m
[32m+[m	[32madd	%ebx, %eax[m
[32m+[m	[32mmov	%eax, -104(%ebp)[m
[32m+[m	[32mjmp	.L4[m
[32m+[m[32m.L5:[m
[32m+[m	[32mmov	$0, %eax[m
[32m+[m	[32mmov	%eax, -108(%ebp)[m
[32m+[m[32m.L6:[m
[32m+[m	[32mmov	-108(%ebp), %eax[m
[32m+[m	[32mpush	%eax[m
[32m+[m	[32mmov	$3, %eax[m
[32m+[m	[32mpop	%ebx[m
[32m+[m	[32mcmp	%eax, %ebx[m
[32m+[m	[32msetl	%al[m
[32m+[m	[32mmovzbl	%al, %eax[m
[32m+[m	[32mcmp	$0, %eax[m
[32m+[m	[32mje	.L7[m
[32m+[m	[32mpush	-108(%ebp)[m
[32m+[m	[32mpush	$str5[m
[32m+[m	[32mcall	printf[m
[32m+[m	[32madd	$8, %esp[m
[32m+[m	[32mmov	-108(%ebp), %eax[m
[32m+[m	[32mpush	%eax[m
[32m+[m	[32mmov	$1, %eax[m
[32m+[m	[32mpop	%ebx[m
[32m+[m	[32madd	%ebx, %eax[m
[32m+[m	[32mmov	%eax, -108(%ebp)[m
[32m+[m	[32mjmp	.L6[m
[32m+[m[32m.L7:[m
 	mov	$69, %eax[m
 	mov	%ebp, %esp[m
 	pop	%ebp[m
 	ret[m
 str0:[m
 	.string "d: %d\n"[m
[32m+[m[32mstr1:[m
[32m+[m	[32m.string "f[%d]: %d\n"[m
[32m+[m[32mstr2:[m
[32m+[m	[32m.string "10 is greater than 5\n"[m
[32m+[m[32mstr3:[m
[32m+[m	[32m.string "10 is not greater than 5\n"[m
[32m+[m[32mstr4:[m
[32m+[m	[32m.string "i: %d\n"[m
[32m+[m[32mstr5:[m
[32m+[m	[32m.string "j: %d\n"[m
[1mdiff --git a/examples/test.z b/examples/test.z[m
[1mdeleted file mode 100644[m
[1mindex ba5551d..0000000[m
[1m--- a/examples/test.z[m
[1m+++ /dev/null[m
[36m@@ -1,13 +0,0 @@[m
[31m-let a[10];[m
[31m-let b = 30;[m
[31m-let c;[m
[31m-[m
[31m-pub fn main {[m
[31m-	let d = b;[m
[31m-	let e[20];[m
[31m-	let f[2] = { 1, 2 };[m
[31m-[m
[31m-	printf("d: %d\n", d);[m
[31m-[m
[31m-	ret 69;[m
[31m-}[m
[1mdiff --git a/examples/test_out b/examples/test_out[m
[1mdeleted file mode 100755[m
[1mindex 16765da..0000000[m
Binary files a/examples/test_out and /dev/null differ
[1mdiff --git a/makefile b/makefile[m
[1mindex 4399d5d..cc7fad4 100644[m
[1m--- a/makefile[m
[1m+++ b/makefile[m
[36m@@ -17,7 +17,7 @@[m [msrc = $(srcc) $(srcs)[m
 objd = obj[m
 obj = $(srcc:$(srcd)/%.c=$(objd)/%.o) $(srcs:$(srcd)/%.s=$(objd)/%.o)[m
 [m
[31m-out = zc[m
[32m+[m[32mout = rc[m
 [m
 all: $(out)[m
 [m
[36m@@ -27,7 +27,7 @@[m [m$(out): $(obj)[m
 # esta parte >/dev/null 2>&1 para pipear less mas limpio[m
 debug:[m
 	@cc $(src) -o $@ $(ccf) $(ldf) -DDEBUG[m
[31m-	@./debug examples/test.z 2>&1 >/dev/null[m
[32m+[m	[32m@./debug examples/test.ru 2>&1 >/dev/null[m
 	@rm debug[m
 [m
 install:[m
[1mdiff --git a/readme.md b/readme.md[m
[1mindex 6196662..631069b 100644[m
[1m--- a/readme.md[m
[1m+++ b/readme.md[m
[36m@@ -1,12 +1,14 @@[m
[31m-# zlang compiler (ZC)[m
[32m+[m[32m# rune compiler (RC)[m
[32m+[m
[32m+[m[32mIt is a small compiler to create binaries in very old computers.[m
 [m
 ## Specs[m
 [m
[31m-All variables are of size word: 32-bit. No data types.[m
[32m+[m[32mThere are two data types: word, and byte.[m
 [m
[31m-Input: .z file, Output: x86-i386 AT&T .s file.[m
[32m+[m[32mInput: .ru file, Output: x86-i386 AT&T .s file.[m
 [m
[31m-Variables outside of any scope are stored in .data.[m
[32m+[m[32mVariables outside of any scope, and strings are stored in .data.[m
 [m
 Variables in a scope are stored in this way:[m
 [m
[36m@@ -49,11 +51,11 @@[m [mThe outputed assembly code uses libc, so this code:[m
 ```c[m
 // file.z[m
 [m
[31m-let glob = 67;[m
[31m-let arr[10];[m
[32m+[m[32mword glob = 67;[m
[32m+[m[32mword arr[10];[m
 [m
 pub fn main {[m
[31m-	let res = glob;[m
[32m+[m	[32mword res = glob;[m
 [m
 	foo();[m
 [m
[1mdiff --git a/src/codegen.c b/src/codegen.c[m
[1mindex 71e1f8f..3709001 100644[m
[1m--- a/src/codegen.c[m
[1m+++ b/src/codegen.c[m
[36m@@ -25,11 +25,20 @@[m [mstatic int string_count;[m
 [m
 static int current;[m
 static int out_fd;[m
[32m+[m[32mstatic int label_count;[m
 [m
 static void emit_function(int pub);[m
[31m-static void emit_let(void);[m
[32m+[m[32mstatic void emit_decl(int type);[m
 static void emit_call(void);[m
 static void emit_ret(void);[m
[32m+[m[32mstatic void emit_expr(void);[m
[32m+[m[32mstatic void emit_expr_prec(int min_prec);[m
[32m+[m[32mstatic void emit_unary(void);[m
[32m+[m[32mstatic void emit_primary(void);[m
[32m+[m[32mstatic void emit_assignment(void);[m
[32m+[m[32mstatic void emit_if(void);[m
[32m+[m[32mstatic void emit_while(void);[m
[32m+[m[32mstatic void emit_for(void);[m
 [m
 static struct token[m
 cur(void)[m
[36m@@ -39,6 +48,14 @@[m [mcur(void)[m
 	return tokens[current];[m
 }[m
 [m
[32m+[m[32mstatic struct token[m
[32m+[m[32mpeek(int offset)[m
[32m+[m[32m{[m
[32m+[m	[32mif (current + offset >= token_count)[m
[32m+[m		[32mreturn (struct token){EOFT, "", 0};[m
[32m+[m	[32mreturn tokens[current + offset];[m
[32m+[m[32m}[m
[32m+[m
 static struct token[m
 adv(void)[m
 {[m
[36m@@ -127,19 +144,488 @@[m [mfind_global(char *name, int name_len)[m
 	return -1;[m
 }[m
 [m
[32m+[m[32mstatic int[m
[32m+[m[32mtype_size(int type)[m
[32m+[m[32m{[m
[32m+[m	[32mreturn type == TYPE_BYTE ? 1 : 4;[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32mstatic void emit_body(void);[m
[32m+[m[32mstatic void emit_primary(void);[m
[32m+[m[32mstatic int get_prec(int type);[m
[32m+[m
[32m+[m[32mstatic void[m
[32m+[m[32mskip_expr(void)[m
[32m+[m[32m{[m
[32m+[m	[32m/* skip unary prefix */[m
[32m+[m	[32mif (cur().type == TILDE || cur().type == BANG) {[m
[32m+[m		[32madv();[m
[32m+[m		[32mskip_expr();[m
[32m+[m		[32mreturn;[m
[32m+[m	[32m}[m
[32m+[m
[32m+[m	[32m/* atom */[m
[32m+[m	[32mif (cur().type == LPAREN) {[m
[32m+[m		[32madv();[m
[32m+[m		[32mskip_expr();[m
[32m+[m		[32mexpect(RPAREN);[m
[32m+[m	[32m} else if (cur().type == NUMT || cur().type == CHARLITT || cur().type == IDENTT) {[m
[32m+[m		[32madv();[m
[32m+[m	[32m} else {[m
[32m+[m		[32mfatal(USER_ERR, NULL, "Expected expression in skip");[m
[32m+[m		[32mreturn;[m
[32m+[m	[32m}[m
[32m+[m
[32m+[m	[32m/* skip binary operators and their right-hand sides */[m
[32m+[m	[32mwhile (get_prec(cur().type) > 0) {[m
[32m+[m		[32madv();[m
[32m+[m		[32mskip_expr();[m
[32m+[m	[32m}[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32mstatic int[m
[32m+[m[32mget_prec(int type)[m
[32m+[m[32m{[m
[32m+[m	[32mswitch (type) {[m
[32m+[m	[32mcase PIPEPIPE:              return 1;[m
[32m+[m	[32mcase AMPAMP:                return 2;[m
[32m+[m	[32mcase PIPE:                  return 3;[m
[32m+[m	[32mcase CARET:                 return 4;[m
[32m+[m	[32mcase AMP:                   return 5;[m
[32m+[m	[32mcase EQT: case NEQ:         return 6;[m
[32m+[m	[32mcase LTT: case GTT:[m
[32m+[m	[32mcase LET: case GET:         return 7;[m
[32m+[m	[32mcase LSHIFTT: case RSHIFTT: return 8;[m
[32m+[m	[32mcase PLUST: case MINUST:    return 9;[m
[32m+[m	[32mcase START: case SLASHT:    return 10;[m
[32m+[m	[32mdefault:                    return 0;[m
[32m+[m	[32m}[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32mstatic void[m
[32m+[m[32memit_binop(int op)[m
[32m+[m[32m{[m
[32m+[m	[32mswitch (op) {[m
[32m+[m	[32mcase PLUST:[m
[32m+[m		[32mout("\tadd\t%%ebx, %%eax\n");[m
[32m+[m		[32mbreak;[m
[32m+[m	[32mcase MINUST:[m
[32m+[m		[32mout("\tsub\t%%eax, %%ebx\n");[m
[32m+[m		[32mout("\tmov\t%%ebx, %%eax\n");[m
[32m+[m		[32mbreak;[m
[32m+[m	[32mcase START:[m
[32m+[m		[32mout("\timul\t%%ebx, %%eax\n");[m
[32m+[m		[32mbreak;[m
[32m+[m	[32mcase SLASHT:[m
[32m+[m		[32mout("\tmov\t%%eax, %%ecx\n");[m
[32m+[m		[32mout("\tmov\t%%ebx, %%eax\n");[m
[32m+[m		[32mout("\tcdq\n");[m
[32m+[m		[32mout("\tidiv\t%%ecx\n");[m
[32m+[m		[32mbreak;[m
[32m+[m	[32mcase LSHIFTT:[m
[32m+[m		[32mout("\tmov\t%%eax, %%ecx\n");[m
[32m+[m		[32mout("\tmov\t%%ebx, %%eax\n");[m
[32m+[m		[32mout("\tshl\t%%cl, %%eax\n");[m
[32m+[m		[32mbreak;[m
[32m+[m	[32mcase RSHIFTT:[m
[32m+[m		[32mout("\tmov\t%%eax, %%ecx\n");[m
[32m+[m		[32mout("\tmov\t%%ebx, %%eax\n");[m
[32m+[m		[32mout("\tsar\t%%cl, %%eax\n");[m
[32m+[m		[32mbreak;[m
[32m+[m	[32mcase AMP:[m
[32m+[m		[32mout("\tand\t%%ebx, %%eax\n");[m
[32m+[m		[32mbreak;[m
[32m+[m	[32mcase PIPE:[m
[32m+[m		[32mout("\tor\t%%ebx, %%eax\n");[m
[32m+[m		[32mbreak;[m
[32m+[m	[32mcase CARET:[m
[32m+[m		[32mout("\txor\t%%ebx, %%eax\n");[m
[32m+[m		[32mbreak;[m
[32m+[m	[32mcase LTT:[m
[32m+[m		[32mout("\tcmp\t%%eax, %%ebx\n");[m
[32m+[m		[32mout("\tsetl\t%%al\n");[m
[32m+[m		[32mout("\tmovzbl\t%%al, %%eax\n");[m
[32m+[m		[32mbreak;[m
[32m+[m	[32mcase GTT:[m
[32m+[m		[32mout("\tcmp\t%%eax, %%ebx\n");[m
[32m+[m		[32mout("\tsetg\t%%al\n");[m
[32m+[m		[32mout("\tmovzbl\t%%al, %%eax\n");[m
[32m+[m		[32mbreak;[m
[32m+[m	[32mcase EQT:[m
[32m+[m		[32mout("\tcmp\t%%eax, %%ebx\n");[m
[32m+[m		[32mout("\tsete\t%%al\n");[m
[32m+[m		[32mout("\tmovzbl\t%%al, %%eax\n");[m
[32m+[m		[32mbreak;[m
[32m+[m	[32mcase NEQ:[m
[32m+[m		[32mout("\tcmp\t%%eax, %%ebx\n");[m
[32m+[m		[32mout("\tsetne\t%%al\n");[m
[32m+[m		[32mout("\tmovzbl\t%%al, %%eax\n");[m
[32m+[m		[32mbreak;[m
[32m+[m	[32mcase LET:[m
[32m+[m		[32mout("\tcmp\t%%eax, %%ebx\n");[m
[32m+[m		[32mout("\tsetle\t%%al\n");[m
[32m+[m		[32mout("\tmovzbl\t%%al, %%eax\n");[m
[32m+[m		[32mbreak;[m
[32m+[m	[32mcase GET:[m
[32m+[m		[32mout("\tcmp\t%%eax, %%ebx\n");[m
[32m+[m		[32mout("\tsetge\t%%al\n");[m
[32m+[m		[32mout("\tmovzbl\t%%al, %%eax\n");[m
[32m+[m		[32mbreak;[m
[32m+[m	[32m}[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32m/*[m
[32m+[m[32m * Evaluates an expression with precedence climbing.[m
[32m+[m[32m * All binary operators are left-associative.[m
[32m+[m[32m * && and || use short-circuit evaluation.[m
[32m+[m[32m */[m
[32m+[m[32mstatic void[m
[32m+[m[32memit_expr_prec(int min_prec)[m
[32m+[m[32m{[m
[32m+[m	[32mint op, prec;[m
[32m+[m
[32m+[m	[32memit_unary();[m
[32m+[m
[32m+[m	[32mwhile (get_prec(cur().type) > min_prec) {[m
[32m+[m		[32mop = cur().type;[m
[32m+[m		[32mprec = get_prec(op);[m
[32m+[m
[32m+[m		[32mif (op == AMPAMP || op == PIPEPIPE) {[m
[32m+[m			[32mint false_label = label_count++;[m
[32m+[m			[32mint true_label = label_count++;[m
[32m+[m
[32m+[m			[32mout("\tcmp\t$0, %%eax\n");[m
[32m+[m			[32mif (op == AMPAMP)[m
[32m+[m				[32mout("\tje\t.L%d\n", false_label);[m
[32m+[m			[32melse[m
[32m+[m				[32mout("\tjne\t.L%d\n", true_label);[m
[32m+[m
[32m+[m			[32madv();[m
[32m+[m			[32memit_expr_prec(prec + 1);[m
[32m+[m
[32m+[m			[32mout("\tcmp\t$0, %%eax\n");[m
[32m+[m			[32mif (op == AMPAMP) {[m
[32m+[m				[32mout("\tje\t.L%d\n", false_label);[m
[32m+[m				[32mout("\tmov\t$1, %%eax\n");[m
[32m+[m				[32mout("\tjmp\t.L%d\n", true_label);[m
[32m+[m			[32m} else {[m
[32m+[m				[32mout("\tjne\t.L%d\n", true_label);[m
[32m+[m				[32mout("\tmov\t$0, %%eax\n");[m
[32m+[m				[32mout("\tjmp\t.L%d\n", false_label);[m
[32m+[m			[32m}[m
[32m+[m
[32m+[m			[32mif (op == AMPAMP) {[m
[32m+[m				[32mout(".L%d:\n", false_label);[m
[32m+[m				[32mout("\tmov\t$0, %%eax\n");[m
[32m+[m				[32mout(".L%d:\n", true_label);[m
[32m+[m			[32m} else {[m
[32m+[m				[32mout(".L%d:\n", false_label);[m
[32m+[m				[32mout("\tmov\t$1, %%eax\n");[m
[32m+[m				[32mout(".L%d:\n", true_label);[m
[32m+[m			[32m}[m
[32m+[m		[32m} else {[m
[32m+[m			[32madv();[m
[32m+[m			[32mout("\tpush\t%%eax\n");[m
[32m+[m			[32memit_expr_prec(prec + 1);[m
[32m+[m			[32mout("\tpop\t%%ebx\n");[m
[32m+[m			[32memit_binop(op);[m
[32m+[m		[32m}[m
[32m+[m	[32m}[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32mstatic void[m
[32m+[m[32memit_expr(void)[m
[32m+[m[32m{[m
[32m+[m	[32memit_expr_prec(0);[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32mstatic void[m
[32m+[m[32memit_unary(void)[m
[32m+[m[32m{[m
[32m+[m	[32mif (cur().type == TILDE) {[m
[32m+[m		[32madv();[m
[32m+[m		[32memit_unary();[m
[32m+[m		[32mout("\tnot\t%%eax\n");[m
[32m+[m		[32mreturn;[m
[32m+[m	[32m}[m
[32m+[m
[32m+[m	[32mif (cur().type == BANG) {[m
[32m+[m		[32madv();[m
[32m+[m		[32memit_unary();[m
[32m+[m		[32mout("\tcmp\t$0, %%eax\n");[m
[32m+[m		[32mout("\tsete\t%%al\n");[m
[32m+[m		[32mout("\tmovzbl\t%%al, %%eax\n");[m
[32m+[m		[32mreturn;[m
[32m+[m	[32m}[m
[32m+[m
[32m+[m	[32mif (cur().type == LPAREN) {[m
[32m+[m		[32madv();[m
[32m+[m		[32memit_expr();[m
[32m+[m		[32mexpect(RPAREN);[m
[32m+[m		[32mreturn;[m
[32m+[m	[32m}[m
[32m+[m
[32m+[m	[32memit_primary();[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32m/*[m
[32m+[m[32m * Parses a primary expression: number or variable.[m
[32m+[m[32m * Result left in %eax.[m
[32m+[m[32m */[m
[32m+[m[32mstatic void[m
[32m+[m[32memit_primary(void)[m
[32m+[m[32m{[m
[32m+[m	[32mint idx;[m
[32m+[m
[32m+[m	[32mif (cur().type == NUMT || cur().type == CHARLITT) {[m
[32m+[m		[32mout("\tmov\t$%d, %%eax\n", atoi(cur().start));[m
[32m+[m		[32madv();[m
[32m+[m		[32mreturn;[m
[32m+[m	[32m}[m
[32m+[m
[32m+[m	[32mif (cur().type == IDENTT) {[m
[32m+[m		[32midx = find_local(cur().start, cur().length);[m
[32m+[m		[32mif (idx >= 0) {[m
[32m+[m			[32mif (locals[idx].type == TYPE_BYTE)[m
[32m+[m				[32mout("\tmovzbl\t%d(%%ebp), %%eax\n", locals[idx].offset);[m
[32m+[m			[32melse[m
[32m+[m				[32mout("\tmov\t%d(%%ebp), %%eax\n", locals[idx].offset);[m
[32m+[m		[32m} else if (find_global(cur().start, cur().length) >= 0) {[m
[32m+[m			[32midx = find_global(cur().start, cur().length);[m
[32m+[m			[32mif (globals[idx].type == TYPE_BYTE)[m
[32m+[m				[32mout("\tmovzbl\t%.*s, %%eax\n", cur().length, cur().start);[m
[32m+[m			[32melse[m
[32m+[m				[32mout("\tmov\t%.*s, %%eax\n", cur().length, cur().start);[m
[32m+[m		[32m} else {[m
[32m+[m			[32mfatal(USER_ERR, NULL, "Undefined variable '%.*s'", cur().length, cur().start);[m
[32m+[m		[32m}[m
[32m+[m		[32madv();[m
[32m+[m		[32mreturn;[m
[32m+[m	[32m}[m
[32m+[m
[32m+[m	[32mfatal(USER_ERR, NULL, "Expected expression");[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32mstatic void[m
[32m+[m[32memit_assignment(void)[m
[32m+[m[32m{[m
[32m+[m	[32mchar *name;[m
[32m+[m	[32mint name_len;[m
[32m+[m	[32mint idx;[m
[32m+[m
[32m+[m	[32mname = cur().start;[m
[32m+[m	[32mname_len = cur().length;[m
[32m+[m	[32madv();[m
[32m+[m	[32mexpect(EQUALT);[m
[32m+[m	[32memit_expr();[m
[32m+[m
[32m+[m	[32midx = find_local(name, name_len);[m
[32m+[m	[32mif (idx >= 0) {[m
[32m+[m		[32mif (locals[idx].type == TYPE_BYTE)[m
[32m+[m			[32mout("\tmovb\t%%al, %d(%%ebp)\n", locals[idx].offset);[m
[32m+[m		[32melse[m
[32m+[m			[32mout("\tmov\t%%eax, %d(%%ebp)\n", locals[idx].offset);[m
[32m+[m	[32m} else if (find_global(name, name_len) >= 0) {[m
[32m+[m		[32midx = find_global(name, name_len);[m
[32m+[m		[32mif (globals[idx].type == TYPE_BYTE)[m
[32m+[m			[32mout("\tmovb\t%%al, %.*s\n", name_len, name);[m
[32m+[m		[32melse[m
[32m+[m			[32mout("\tmov\t%%eax, %.*s\n", name_len, name);[m
[32m+[m	[32m} else {[m
[32m+[m		[32mfatal(USER_ERR, NULL, "Undefined variable '%.*s'", name_len, name);[m
[32m+[m	[32m}[m
[32m+[m	[32mexpect(SEMIT);[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32mstatic void[m
[32m+[m[32memit_if(void)[m
[32m+[m[32m{[m
[32m+[m	[32mint my_label;[m
[32m+[m
[32m+[m	[32mexpect(IFT);[m
[32m+[m	[32memit_expr();[m
[32m+[m
[32m+[m	[32mmy_label = label_count++;[m
[32m+[m
[32m+[m	[32mout("\tcmp\t$0, %%eax\n");[m
[32m+[m	[32mout("\tje\t.L%d\n", my_label);[m
[32m+[m
[32m+[m	[32mexpect(LBRAC);[m
[32m+[m	[32memit_body();[m
[32m+[m	[32mexpect(RBRAC);[m
[32m+[m
[32m+[m	[32mif (cur().type == ELSET) {[m
[32m+[m		[32mint else_label = label_count++;[m
[32m+[m		[32mout("\tjmp\t.L%d\n", else_label);[m
[32m+[m		[32mout(".L%d:\n", my_label);[m
[32m+[m		[32madv();[m
[32m+[m		[32mexpect(LBRAC);[m
[32m+[m		[32memit_body();[m
[32m+[m		[32mexpect(RBRAC);[m
[32m+[m		[32mout(".L%d:\n", else_label);[m
[32m+[m	[32m} else {[m
[32m+[m		[32mout(".L%d:\n", my_label);[m
[32m+[m	[32m}[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32mstatic void[m
[32m+[m[32memit_while(void)[m
[32m+[m[32m{[m
[32m+[m	[32mint loop_label;[m
[32m+[m	[32mint end_label;[m
[32m+[m
[32m+[m	[32mexpect(WHILET);[m
[32m+[m
[32m+[m	[32mloop_label = label_count++;[m
[32m+[m	[32mend_label = label_count++;[m
[32m+[m
[32m+[m	[32mout(".L%d:\n", loop_label);[m
[32m+[m	[32memit_expr();[m
[32m+[m
[32m+[m	[32mout("\tcmp\t$0, %%eax\n");[m
[32m+[m	[32mout("\tje\t.L%d\n", end_label);[m
[32m+[m
[32m+[m	[32mexpect(LBRAC);[m
[32m+[m	[32memit_body();[m
[32m+[m	[32mexpect(RBRAC);[m
[32m+[m
[32m+[m	[32mout("\tjmp\t.L%d\n", loop_label);[m
[32m+[m	[32mout(".L%d:\n", end_label);[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32mstatic void[m
[32m+[m[32memit_for(void)[m
[32m+[m[32m{[m
[32m+[m	[32mint loop_label;[m
[32m+[m	[32mint end_label;[m
[32m+[m	[32mint update_start;[m
[32m+[m
[32m+[m	[32mexpect(FORT);[m
[32m+[m
[32m+[m	[32m/* init */[m
[32m+[m	[32mif (cur().type == WORDT || cur().type == BYTET) {[m
[32m+[m		[32memit_decl((cur().type == WORDT) ? TYPE_WORD : TYPE_BYTE);[m
[32m+[m	[32m} else if (cur().type == IDENTT && peek(1).type == EQUALT) {[m
[32m+[m		[32memit_assignment();[m
[32m+[m	[32m} else {[m
[32m+[m		[32mexpect(SEMIT);[m
[32m+[m	[32m}[m
[32m+[m
[32m+[m	[32m/* condition */[m
[32m+[m	[32mloop_label = label_count++;[m
[32m+[m	[32mend_label = label_count++;[m
[32m+[m
[32m+[m	[32mout(".L%d:\n", loop_label);[m
[32m+[m	[32memit_expr();[m
[32m+[m
[32m+[m	[32mout("\tcmp\t$0, %%eax\n");[m
[32m+[m	[32mout("\tje\t.L%d\n", end_label);[m
[32m+[m
[32m+[m	[32mexpect(SEMIT);[m
[32m+[m
[32m+[m	[32m/* save update tokens position */[m
[32m+[m	[32mupdate_start = current;[m
[32m+[m	[32mwhile (cur().type != LBRAC)[m
[32m+[m		[32madv();[m
[32m+[m	[32m/* cur() is now LBRAC */[m
[32m+[m
[32m+[m	[32mexpect(LBRAC);[m
[32m+[m	[32memit_body();[m
[32m+[m	[32mexpect(RBRAC);[m
[32m+[m
[32m+[m	[32m/* emit update by temporarily restoring position */[m
[32m+[m	[32m{[m
[32m+[m		[32mint saved = current;[m
[32m+[m		[32mcurrent = update_start;[m
[32m+[m		[32mwhile (cur().type != LBRAC) {[m
[32m+[m			[32mif (cur().type == IDENTT && peek(1).type == EQUALT) {[m
[32m+[m				[32mchar *uname = cur().start;[m
[32m+[m				[32mint uname_len = cur().length;[m
[32m+[m				[32mint uidx;[m
[32m+[m				[32madv();[m
[32m+[m				[32madv();[m
[32m+[m				[32memit_expr();[m
[32m+[m				[32muidx = find_local(uname, uname_len);[m
[32m+[m				[32mif (uidx >= 0) {[m
[32m+[m					[32mif (locals[uidx].type == TYPE_BYTE)[m
[32m+[m						[32mout("\tmovb\t%%al, %d(%%ebp)\n", locals[uidx].offset);[m
[32m+[m					[32melse[m
[32m+[m						[32mout("\tmov\t%%eax, %d(%%ebp)\n", locals[uidx].offset);[m
[32m+[m				[32m} else if (find_global(uname, uname_len) >= 0) {[m
[32m+[m					[32muidx = find_global(uname, uname_len);[m
[32m+[m					[32mif (globals[uidx].type == TYPE_BYTE)[m
[32m+[m						[32mout("\tmovb\t%%al, %.*s\n", uname_len, uname);[m
[32m+[m					[32melse[m
[32m+[m						[32mout("\tmov\t%%eax, %.*s\n", uname_len, uname);[m
[32m+[m				[32m} else[m
[32m+[m					[32mfatal(USER_ERR, NULL, "Undefined variable '%.*s'", uname_len, uname);[m
[32m+[m			[32m} else {[m
[32m+[m				[32madv();[m
[32m+[m			[32m}[m
[32m+[m		[32m}[m
[32m+[m		[32mcurrent = saved;[m
[32m+[m	[32m}[m
[32m+[m
[32m+[m	[32mout("\tjmp\t.L%d\n", loop_label);[m
[32m+[m	[32mout(".L%d:\n", end_label);[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32m/*[m
[32m+[m[32m * Emits a block of statements until a closing brace is found.[m
[32m+[m[32m * Used by if, while, and for to emit their bodies.[m
[32m+[m[32m */[m
[32m+[m[32mstatic void[m
[32m+[m[32memit_body(void)[m
[32m+[m[32m{[m
[32m+[m	[32mwhile (cur().type != RBRAC) {[m
[32m+[m		[32mswitch (cur().type) {[m
[32m+[m		[32mcase WORDT:[m
[32m+[m			[32memit_decl(TYPE_WORD);[m
[32m+[m			[32mbreak;[m
[32m+[m		[32mcase BYTET:[m
[32m+[m			[32memit_decl(TYPE_BYTE);[m
[32m+[m			[32mbreak;[m
[32m+[m		[32mcase IDENTT:[m
[32m+[m			[32mif (peek(1).type == EQUALT)[m
[32m+[m				[32memit_assignment();[m
[32m+[m			[32melse[m
[32m+[m				[32memit_call();[m
[32m+[m			[32mbreak;[m
[32m+[m		[32mcase RETT:[m
[32m+[m			[32memit_ret();[m
[32m+[m			[32mbreak;[m
[32m+[m		[32mcase IFT:[m
[32m+[m			[32memit_if();[m
[32m+[m			[32mbreak;[m
[32m+[m		[32mcase WHILET:[m
[32m+[m			[32memit_while();[m
[32m+[m			[32mbreak;[m
[32m+[m		[32mcase FORT:[m
[32m+[m			[32memit_for();[m
[32m+[m			[32mbreak;[m
[32m+[m		[32mdefault:[m
[32m+[m			[32mfatal(USER_ERR, NULL, "Unexpected token in function body");[m
[32m+[m		[32m}[m
[32m+[m	[32m}[m
[32m+[m[32m}[m
[32m+[m
 static void[m
 emit_global_data(int pub)[m
 {[m
[32m+[m	[32mint vtype;[m
[32m+[m
 	if (pub)[m
 		globals[global_count].is_pub = 1;[m
 	else[m
 		globals[global_count].is_pub = 0;[m
 [m
[31m-	while (cur().type == LETT) {[m
[32m+[m	[32mwhile (cur().type == WORDT || cur().type == BYTET) {[m
[32m+[m		[32mvtype = (cur().type == WORDT) ? TYPE_WORD : TYPE_BYTE;[m
 		adv();[m
 		expect(IDENTT);[m
 		globals[global_count].name = tokens[current - 1].start;[m
 		globals[global_count].name_len = tokens[current - 1].length;[m
[32m+[m		[32mglobals[global_count].type = vtype;[m
 [m
 		if (pub) {[m
 			out("\t.globl\t%.*s\n", globals[global_count].name_len,[m
[36m@@ -156,7 +642,7 @@[m [memit_global_data(int pub)[m
 			expect(RBRAT);[m
 			expect(SEMIT);[m
 			out("%.*s:\n", globals[global_count].name_len, globals[global_count].name);[m
[31m-			out("\t.zero %d\n", globals[global_count].array_size * 4);[m
[32m+[m			[32mout("\t.zero %d\n", globals[global_count].array_size * type_size(vtype));[m
 		} else if (cur().type == EQUALT) {[m
 			adv();[m
 			globals[global_count].is_array = 0;[m
[36m@@ -166,14 +652,17 @@[m [memit_global_data(int pub)[m
 			adv();[m
 			expect(SEMIT);[m
 			out("%.*s:\n", globals[global_count].name_len, globals[global_count].name);[m
[31m-			out("\t.long %d\n", globals[global_count].init_val);[m
[32m+[m			[32mif (vtype == TYPE_BYTE)[m
[32m+[m				[32mout("\t.byte %d\n", globals[global_count].init_val);[m
[32m+[m			[32melse[m
[32m+[m				[32mout("\t.long %d\n", globals[global_count].init_val);[m
 		} else {[m
 			globals[global_count].is_array = 0;[m
 			globals[global_count].array_size = 0;[m
 			globals[global_count].has_init = 0;[m
 			expect(SEMIT);[m
 			out("%.*s:\n", globals[global_count].name_len, globals[global_count].name);[m
[31m-			out("\t.zero 0\n");[m
[32m+[m			[32mout("\t.zero %d\n", type_size(vtype));[m
 		}[m
 		global_count++;[m
 	}[m
[36m@@ -235,20 +724,23 @@[m [memit_function(int pub)[m
 [m
 	/* first pass: calculate stack size and build locals table */[m
 	while (cur().type != RBRAC) {[m
[31m-		if (cur().type == LETT) {[m
[32m+[m		[32mif (cur().type == WORDT || cur().type == BYTET) {[m
[32m+[m			[32mint vtype;[m
 			char *lname;[m
 			int lname_len;[m
[32m+[m			[32mvtype = (cur().type == WORDT) ? TYPE_WORD : TYPE_BYTE;[m
 			adv();[m
 			lname = cur().start;[m
 			lname_len = cur().length;[m
 			adv(); /* skip variable name */[m
 			if (cur().type == LBRAT) {[m
 				adv();[m
[31m-				stack_size += atoi(cur().start) * 4;[m
[32m+[m				[32mstack_size += atoi(cur().start) * type_size(vtype);[m
 				adv();[m
 				expect(RBRAT);[m
 				locals[local_count].name = lname;[m
 				locals[local_count].name_len = lname_len;[m
[32m+[m				[32mlocals[local_count].type = vtype;[m
 				locals[local_count].offset = -stack_size;[m
 				local_count++;[m
 				if (match(EQUALT)) {[m
[36m@@ -263,17 +755,19 @@[m [memit_function(int pub)[m
 				expect(SEMIT);[m
 			} else if (cur().type == EQUALT) {[m
 				adv();[m
[31m-				stack_size += 4;[m
[32m+[m				[32mstack_size += type_size(vtype);[m
 				locals[local_count].name = lname;[m
 				locals[local_count].name_len = lname_len;[m
[32m+[m				[32mlocals[local_count].type = vtype;[m
 				locals[local_count].offset = -stack_size;[m
 				local_count++;[m
[31m-				adv();[m
[32m+[m				[32mskip_expr();[m
 				expect(SEMIT);[m
 			} else {[m
[31m-				stack_size += 4;[m
[32m+[m				[32mstack_size += type_size(vtype);[m
 				locals[local_count].name = lname;[m
 				locals[local_count].name_len = lname_len;[m
[32m+[m				[32mlocals[local_count].type = vtype;[m
 				locals[local_count].offset = -stack_size;[m
 				local_count++;[m
 				expect(SEMIT);[m
[36m@@ -281,44 +775,71 @@[m [memit_function(int pub)[m
 		} else if (cur().type == RETT) {[m
 			adv();[m
 			if (cur().type != SEMIT)[m
[31m-				adv();[m
[32m+[m				[32mskip_expr();[m
 			expect(SEMIT);[m
 		} else if (cur().type == IDENTT) {[m
 			adv();[m
[31m-			expect(LPAREN);[m
[31m-			while (cur().type != RPAREN) {[m
[31m-				if (match(COMMAT))[m
[31m-					continue;[m
[32m+[m			[32mif (cur().type == LPAREN) {[m
[32m+[m				[32madv();[m
[32m+[m				[32mwhile (cur().type != RPAREN) {[m
[32m+[m					[32mif (match(COMMAT))[m
[32m+[m						[32mcontinue;[m
[32m+[m					[32madv();[m
[32m+[m				[32m}[m
[32m+[m				[32mexpect(RPAREN);[m
[32m+[m				[32mexpect(SEMIT);[m
[32m+[m			[32m} else if (cur().type == EQUALT) {[m
 				adv();[m
[32m+[m				[32mskip_expr();[m
[32m+[m				[32mexpect(SEMIT);[m
[32m+[m			[32m} else {[m
[32m+[m				[32mexpect(SEMIT);[m
 			}[m
[31m-			expect(RPAREN);[m
[31m-			expect(SEMIT);[m
[32m+[m		[32m} else if (cur().type == IFT) {[m
[32m+[m			[32madv();[m
[32m+[m			[32mwhile (cur().type != LBRAC)[m
[32m+[m				[32madv();[m
[32m+[m			[32mexpect(LBRAC);[m
[32m+[m			[32mwhile (cur().type != RBRAC)[m
[32m+[m				[32madv();[m
[32m+[m			[32mexpect(RBRAC);[m
[32m+[m			[32mif (cur().type == ELSET) {[m
[32m+[m				[32madv();[m
[32m+[m				[32mexpect(LBRAC);[m
[32m+[m				[32mwhile (cur().type != RBRAC)[m
[32m+[m					[32madv();[m
[32m+[m				[32mexpect(RBRAC);[m
[32m+[m			[32m}[m
[32m+[m		[32m} else if (cur().type == WHILET) {[m
[32m+[m			[32madv();[m
[32m+[m			[32mwhile (cur().type != LBRAC)[m
[32m+[m				[32madv();[m
[32m+[m			[32mexpect(LBRAC);[m
[32m+[m			[32mwhile (cur().type != RBRAC)[m
[32m+[m				[32madv();[m
[32m+[m			[32mexpect(RBRAC);[m
[32m+[m		[32m} else if (cur().type == FORT) {[m
[32m+[m			[32madv();[m
[32m+[m			[32mwhile (cur().type != LBRAC)[m
[32m+[m				[32madv();[m
[32m+[m			[32mexpect(LBRAC);[m
[32m+[m			[32mwhile (cur().type != RBRAC)[m
[32m+[m				[32madv();[m
[32m+[m			[32mexpect(RBRAC);[m
 		} else {[m
 			adv();[m
 		}[m
 	}[m
 [m
[32m+[m	[32m/* align stack to 4 bytes */[m
[32m+[m	[32mstack_size = (stack_size + 3) & ~3;[m
[32m+[m
 	if (stack_size > 0)[m
 		out("\tsub\t$%d, %%esp\n", stack_size);[m
 [m
 	/* second pass: emit code */[m
 	current = body_start;[m
[31m-[m
[31m-	while (cur().type != RBRAC) {[m
[31m-		switch (cur().type) {[m
[31m-		case LETT:[m
[31m-			emit_let();[m
[31m-			break;[m
[31m-		case IDENTT:[m
[31m-			emit_call();[m
[31m-			break;[m
[31m-		case RETT:[m
[31m-			emit_ret();[m
[31m-			break;[m
[31m-		default:[m
[31m-			fatal(USER_ERR, NULL, "Unexpected token in function body");[m
[31m-		}[m
[31m-	}[m
[32m+[m	[32memit_body();[m
 [m
 	expect(RBRAC);[m
 [m
[36m@@ -328,14 +849,17 @@[m [memit_function(int pub)[m
 }[m
 [m
 static void[m
[31m-emit_let(void)[m
[32m+[m[32memit_decl(int type)[m
 {[m
 	char *name;[m
 	int name_len;[m
 	int idx;[m
 	int offset;[m
 [m
[31m-	expect(LETT);[m
[32m+[m	[32mif (type == TYPE_WORD)[m
[32m+[m		[32mexpect(WORDT);[m
[32m+[m	[32melse[m
[32m+[m		[32mexpect(BYTET);[m
 	name = cur().start;[m
 	name_len = cur().length;[m
 	expect(IDENTT);[m
[36m@@ -355,7 +879,7 @@[m [memit_let(void)[m
 					continue;[m
 				out("\tmov\t$%d, %d(%%ebp)\n",[m
 				    atoi(cur().start), arr_off);[m
[31m-				arr_off += 4;[m
[32m+[m				[32marr_off += type_size(type);[m
 				adv();[m
 			}[m
 			expect(RBRAC);[m
[36m@@ -369,19 +893,11 @@[m [memit_let(void)[m
 		idx = find_local(name, name_len);[m
 		offset = locals[idx].offset;[m
 [m
[31m-		if (cur().type == NUMT) {[m
[31m-			out("\tmov\t$%d, %d(%%ebp)\n", atoi(cur().start), offset);[m
[31m-			adv();[m
[31m-		} else if (cur().type == IDENTT) {[m
[31m-			if (find_local(cur().start, cur().length) >= 0) {[m
[31m-				out("\tmov\t%d(%%ebp), %%eax\n", locals[find_local(cur().start, cur().length)].offset);[m
[31m-				out("\tmov\t%%eax, %d(%%ebp)\n", offset);[m
[31m-			} else if (find_global(cur().start, cur().length) >= 0) {[m
[31m-				out("\tmov\t%.*s, %%eax\n", cur().length, cur().start);[m
[31m-				out("\tmov\t%%eax, %d(%%ebp)\n", offset);[m
[31m-			}[m
[31m-			adv();[m
[31m-		}[m
[32m+[m		[32memit_expr();[m
[32m+[m		[32mif (type == TYPE_BYTE)[m
[32m+[m			[32mout("\tmovb\t%%al, %d(%%ebp)\n", offset);[m
[32m+[m		[32melse[m
[32m+[m			[32mout("\tmov\t%%eax, %d(%%ebp)\n", offset);[m
 		expect(SEMIT);[m
 		return;[m
 	}[m
[36m@@ -420,11 +936,23 @@[m [memit_call(void)[m
 	for (i = args_count - 1; i >= 0; i--) {[m
 		if (strings[0][0] && find_string(arg_starts[i], arg_lengths[i]) >= 0)[m
 			out("\tpush\t$str%d\n", find_string(arg_starts[i], arg_lengths[i]));[m
[31m-		else if (find_local(arg_starts[i], arg_lengths[i]) >= 0)[m
[31m-			out("\tpush\t%d(%%ebp)\n", locals[find_local(arg_starts[i], arg_lengths[i])].offset);[m
[31m-		else if (find_global(arg_starts[i], arg_lengths[i]) >= 0)[m
[31m-			out("\tpush\t%.*s\n", arg_lengths[i], arg_starts[i]);[m
[31m-		else if (arg_lengths[i] == 1 && *arg_starts[i] >= '0' && *arg_starts[i] <= '9')[m
[32m+[m		[32melse if (find_local(arg_starts[i], arg_lengths[i]) >= 0) {[m
[32m+[m			[32mint alidx = find_local(arg_starts[i], arg_lengths[i]);[m
[32m+[m			[32mif (locals[alidx].type == TYPE_BYTE) {[m
[32m+[m				[32mout("\tmovzbl\t%d(%%ebp), %%eax\n", locals[alidx].offset);[m
[32m+[m				[32mout("\tpush\t%%eax\n");[m
[32m+[m			[32m} else {[m
[32m+[m				[32mout("\tpush\t%d(%%ebp)\n", locals[alidx].offset);[m
[32m+[m			[32m}[m
[32m+[m		[32m} else if (find_global(arg_starts[i], arg_lengths[i]) >= 0) {[m
[32m+[m			[32mint agidx = find_global(arg_starts[i], arg_lengths[i]);[m
[32m+[m			[32mif (globals[agidx].type == TYPE_BYTE) {[m
[32m+[m				[32mout("\tmovzbl\t%.*s, %%eax\n", arg_lengths[i], arg_starts[i]);[m
[32m+[m				[32mout("\tpush\t%%eax\n");[m
[32m+[m			[32m} else {[m
[32m+[m				[32mout("\tpush\t%.*s\n", arg_lengths[i], arg_starts[i]);[m
[32m+[m			[32m}[m
[32m+[m		[32m} else if (arg_lengths[i] == 1 && *arg_starts[i] >= '0' && *arg_starts[i] <= '9')[m
 			out("\tpush\t$%d\n", atoi(arg_starts[i]));[m
 		else[m
 			out("\tpush\t$%d\n", atoi(arg_starts[i]));[m
[36m@@ -438,25 +966,10 @@[m [memit_call(void)[m
 static void[m
 emit_ret(void)[m
 {[m
[31m-	int idx;[m
[31m-[m
 	expect(RETT);[m
 [m
[31m-	if (cur().type == SEMIT) {[m
[31m-		adv();[m
[31m-		return;[m
[31m-	}[m
[31m-[m
[31m-	if (cur().type == NUMT) {[m
[31m-		out("\tmov\t$%d, %%eax\n", atoi(cur().start));[m
[31m-		adv();[m
[31m-	} else if (cur().type == IDENTT) {[m
[31m-		idx = find_local(cur().start, cur().length);[m
[31m-		if (idx >= 0)[m
[31m-			out("\tmov\t%d(%%ebp), %%eax\n", locals[idx].offset);[m
[31m-		else if (find_global(cur().start, cur().length) >= 0)[m
[31m-			out("\tmov\t%.*s, %%eax\n", cur().length, cur().start);[m
[31m-		adv();[m
[32m+[m	[32mif (cur().type != SEMIT) {[m
[32m+[m		[32memit_expr();[m
 	}[m
 [m
 	expect(SEMIT);[m
[36m@@ -468,6 +981,7 @@[m [mvoid codegen(int fd)[m
 	current = 0;[m
 	global_count = 0;[m
 	string_count = 0;[m
[32m+[m	[32mlabel_count = 0;[m
 [m
 	collect_strings();[m
 [m
[36m@@ -479,15 +993,15 @@[m [mvoid codegen(int fd)[m
 			if (cur().type == FNT) {[m
 				adv();[m
 				emit_function(1);[m
[31m-			} else if (cur().type == LETT) {[m
[32m+[m			[32m} else if (cur().type == WORDT || cur().type == BYTET) {[m
 				emit_global_data(1);[m
 			} else {[m
[31m-				fatal(USER_ERR, NULL, "Expected fn or let after pub");[m
[32m+[m				[32mfatal(USER_ERR, NULL, "Expected fn, word, or byte after pub");[m
 			}[m
 		} else if (cur().type == FNT) {[m
 			adv();[m
 			emit_function(0);[m
[31m-		} else if (cur().type == LETT) {[m
[32m+[m		[32m} else if (cur().type == WORDT || cur().type == BYTET) {[m
 			emit_global_data(0);[m
 		} else {[m
 			fatal(USER_ERR, NULL, "Expected function or global declaration");[m
[1mdiff --git a/src/codegen.h b/src/codegen.h[m
[1mindex 12c2385..0b51add 100644[m
[1m--- a/src/codegen.h[m
[1m+++ b/src/codegen.h[m
[36m@@ -12,9 +12,13 @@[m
 #define MAX_STRINGS 64[m
 #define MAX_STR_LEN 256[m
 [m
[32m+[m[32m#define TYPE_WORD 0[m
[32m+[m[32m#define TYPE_BYTE 1[m
[32m+[m
 struct global_var {[m
 	char *name;[m
 	int name_len;[m
[32m+[m	[32mint type;[m
 	int is_array;[m
 	int array_size;[m
 	int has_init;[m
[36m@@ -25,6 +29,7 @@[m [mstruct global_var {[m
 struct local_var {[m
 	char *name;[m
 	int name_len;[m
[32m+[m	[32mint type;[m
 	int offset;[m
 };[m
 [m
[1mdiff --git a/src/compiler.c b/src/compiler.c[m
[1mindex 56d61a9..33124c8 100644[m
[1m--- a/src/compiler.c[m
[1m+++ b/src/compiler.c[m
[36m@@ -51,10 +51,10 @@[m [mget_paths(const char *path, char *src, char *out)[m
 {[m
 	int len = strlen(path);[m
 [m
[31m-	if (strcmp(&path[len-2], ".z"))[m
[31m-		fatal(USER_ERR, NULL, "%s is not a .z file!", path);[m
[32m+[m	[32mif (strcmp(&path[len-3], ".ru"))[m
[32m+[m		[32mfatal(USER_ERR, NULL, "%s is not a .ru file!", path);[m
 [m
 	strcpy(src, path);[m
 	strcpy(out, path);[m
[31m-	strcpy(&out[len-2], ".s");[m
[32m+[m	[32mstrcpy(&out[len-3], ".s");[m
 }[m
[1mdiff --git a/src/debug.c b/src/debug.c[m
[1mindex 87a1622..8734830 100644[m
[1m--- a/src/debug.c[m
[1m+++ b/src/debug.c[m
[36m@@ -11,14 +11,14 @@[m
 [m
 void say_version()[m
 {[m
[31m-	printf("zc (zlang compiler) v%s\n", VERSION);[m
[32m+[m	[32mprintf("rc (rune compiler) v%s\n", VERSION);[m
 	printf("Copyright (c) 2026 %s\n", AUTHOR);[m
 	printf("License: GPL\n");[m
 }[m
 [m
 void help()[m
 {[m
[31m-	printf("Usage: zc [options] [file.zc]\n");[m
[32m+[m	[32mprintf("Usage: rc [options] [file.ru]\n");[m
 	printf("Options:\n");[m
 	printf("\t-h print this text.\n");[m
 	printf("\t-v show the version.\n");[m
[1mdiff --git a/src/tokenizer.c b/src/tokenizer.c[m
[1mindex 8fd2cd3..502b017 100644[m
[1m--- a/src/tokenizer.c[m
[1m+++ b/src/tokenizer.c[m
[36m@@ -12,7 +12,11 @@[m
 struct token tokens[MAX_TOKENS];[m
 int token_count;[m
 [m
[32m+[m[32mstatic char charbuf[MAX_TOKENS][16];[m
[32m+[m
 static void add_token(enum token_type type, char *start, int length);[m
[32m+[m[32mstatic int is_hex(char c);[m
[32m+[m[32mstatic int hex_val(char c);[m
 [m
 /*[m
  * Tokenizes the source string into an array of tokens.[m
[36m@@ -52,14 +56,24 @@[m [mvoid tokenize(char *src)[m
 				i++;[m
 			len = (int)(&src[i] - start);[m
 [m
[31m-			if (len == 3 && !memcmp(start, "let", 3))[m
[31m-				add_token(LETT, start, len);[m
[32m+[m			[32mif (len == 4 && !memcmp(start, "word", 4))[m
[32m+[m				[32madd_token(WORDT, start, len);[m
[32m+[m			[32melse if (len == 4 && !memcmp(start, "byte", 4))[m
[32m+[m				[32madd_token(BYTET, start, len);[m
 			else if (len == 3 && !memcmp(start, "pub", 3))[m
 				add_token(PUBT, start, len);[m
 			else if (len == 2 && !memcmp(start, "fn", 2))[m
 				add_token(FNT, start, len);[m
 			else if (len == 3 && !memcmp(start, "ret", 3))[m
 				add_token(RETT, start, len);[m
[32m+[m			[32melse if (len == 2 && !memcmp(start, "if", 2))[m
[32m+[m				[32madd_token(IFT, start, len);[m
[32m+[m			[32melse if (len == 4 && !memcmp(start, "else", 4))[m
[32m+[m				[32madd_token(ELSET, start, len);[m
[32m+[m			[32melse if (len == 5 && !memcmp(start, "while", 5))[m
[32m+[m				[32madd_token(WHILET, start, len);[m
[32m+[m			[32melse if (len == 3 && !memcmp(start, "for", 3))[m
[32m+[m				[32madd_token(FORT, start, len);[m
 			else[m
 				add_token(IDENTT, start, len);[m
 			continue;[m
[36m@@ -85,7 +99,54 @@[m [mvoid tokenize(char *src)[m
 			continue;[m
 		}[m
 [m
[31m-		/* single-char tokens */[m
[32m+[m		[32m/* char literals */[m
[32m+[m		[32mif (src[i] == '\'') {[m
[32m+[m			[32mchar val;[m
[32m+[m			[32mi++; /* skip opening ' */[m
[32m+[m			[32mif (src[i] == '\\') {[m
[32m+[m				[32mi++;[m
[32m+[m				[32mswitch (src[i]) {[m
[32m+[m				[32mcase 'n':  val = '\n'; i++; break;[m
[32m+[m				[32mcase 't':  val = '\t'; i++; break;[m
[32m+[m				[32mcase '\\': val = '\\'; i++; break;[m
[32m+[m				[32mcase '\'': val = '\''; i++; break;[m
[32m+[m				[32mcase '0':  val = '\0'; i++; break;[m
[32m+[m				[32mcase 'x': {[m
[32m+[m					[32mint h = 0, digits = 0;[m
[32m+[m					[32mi++;[m
[32m+[m					[32mwhile (is_hex(src[i]) && digits < 2) {[m
[32m+[m						[32mh = h * 16 + hex_val(src[i]);[m
[32m+[m						[32mi++;[m
[32m+[m						[32mdigits++;[m
[32m+[m					[32m}[m
[32m+[m					[32mval = (char)h;[m
[32m+[m					[32mbreak;[m
[32m+[m				[32m}[m
[32m+[m				[32mcase 'b': {[m
[32m+[m					[32mint b = 0, bits = 0;[m
[32m+[m					[32mi++;[m
[32m+[m					[32mwhile ((src[i] == '0' || src[i] == '1') && bits < 8) {[m
[32m+[m						[32mb = b * 2 + (src[i] - '0');[m
[32m+[m						[32mi++;[m
[32m+[m						[32mbits++;[m
[32m+[m					[32m}[m
[32m+[m					[32mval = (char)b;[m
[32m+[m					[32mbreak;[m
[32m+[m				[32m}[m
[32m+[m				[32mdefault:[m
[32m+[m					[32mfatal(USER_ERR, NULL, "Unknown escape '\\%c'", src[i]);[m
[32m+[m				[32m}[m
[32m+[m			[32m} else {[m
[32m+[m				[32mval = src[i];[m
[32m+[m				[32mi++;[m
[32m+[m			[32m}[m
[32m+[m			[32mi++; /* skip closing ' */[m
[32m+[m			[32msprintf(charbuf[token_count], "%d", (int)(unsigned char)val);[m
[32m+[m			[32madd_token(CHARLITT, charbuf[token_count], strlen(charbuf[token_count]));[m
[32m+[m			[32mcontinue;[m
[32m+[m		[32m}[m
[32m+[m
[32m+[m		[32m/* tokens */[m
 		switch (src[i]) {[m
 		case '[': add_token(LBRAT, &src[i], 1); i++; continue;[m
 		case ']': add_token(RBRAT, &src[i], 1); i++; continue;[m
[36m@@ -94,8 +155,39 @@[m [mvoid tokenize(char *src)[m
 		case '(': add_token(LPAREN, &src[i], 1); i++; continue;[m
 		case ')': add_token(RPAREN, &src[i], 1); i++; continue;[m
 		case ',': add_token(COMMAT, &src[i], 1); i++; continue;[m
[31m-		case '=': add_token(EQUALT, &src[i], 1); i++; continue;[m
 		case ';': add_token(SEMIT, &src[i], 1); i++; continue;[m
[32m+[m		[32mcase '+': add_token(PLUST, &src[i], 1); i++; continue;[m
[32m+[m		[32mcase '-': add_token(MINUST, &src[i], 1); i++; continue;[m
[32m+[m		[32mcase '*': add_token(START, &src[i], 1); i++; continue;[m
[32m+[m		[32mcase '/': add_token(SLASHT, &src[i], 1); i++; continue;[m
[32m+[m		[32mcase '=':[m
[32m+[m			[32mif (src[i+1] == '=') { add_token(EQT, &src[i], 2); i += 2; }[m
[32m+[m			[32melse { add_token(EQUALT, &src[i], 1); i++; }[m
[32m+[m			[32mcontinue;[m
[32m+[m		[32mcase '!':[m
[32m+[m			[32mif (src[i+1] == '=') { add_token(NEQ, &src[i], 2); i += 2; }[m
[32m+[m			[32melse { add_token(BANG, &src[i], 1); i++; }[m
[32m+[m			[32mcontinue;[m
[32m+[m		[32mcase '<':[m
[32m+[m			[32mif (src[i+1] == '<') { add_token(LSHIFTT, &src[i], 2); i += 2; }[m
[32m+[m			[32melse if (src[i+1] == '=') { add_token(LET, &src[i], 2); i += 2; }[m
[32m+[m			[32melse { add_token(LTT, &src[i], 1); i++; }[m
[32m+[m			[32mcontinue;[m
[32m+[m		[32mcase '>':[m
[32m+[m			[32mif (src[i+1] == '>') { add_token(RSHIFTT, &src[i], 2); i += 2; }[m
[32m+[m			[32melse if (src[i+1] == '=') { add_token(GET, &src[i], 2); i += 2; }[m
[32m+[m			[32melse { add_token(GTT, &src[i], 1); i++; }[m
[32m+[m			[32mcontinue;[m
[32m+[m		[32mcase '&':[m
[32m+[m			[32mif (src[i+1] == '&') { add_token(AMPAMP, &src[i], 2); i += 2; }[m
[32m+[m			[32melse { add_token(AMP, &src[i], 1); i++; }[m
[32m+[m			[32mcontinue;[m
[32m+[m		[32mcase '|':[m
[32m+[m			[32mif (src[i+1] == '|') { add_token(PIPEPIPE, &src[i], 2); i += 2; }[m
[32m+[m			[32melse { add_token(PIPE, &src[i], 1); i++; }[m
[32m+[m			[32mcontinue;[m
[32m+[m		[32mcase '^': add_token(CARET, &src[i], 1); i++; continue;[m
[32m+[m		[32mcase '~': add_token(TILDE, &src[i], 1); i++; continue;[m
 		}[m
 [m
 		fatal(USER_ERR, NULL, "Unexpected character '%c'", src[i]);[m
[36m@@ -119,6 +211,22 @@[m [mbool is_alnum(char c)[m
 	return is_alpha(c) || is_digit(c);[m
 }[m
 [m
[32m+[m[32mstatic int[m
[32m+[m[32mis_hex(char c)[m
[32m+[m[32m{[m
[32m+[m	[32mreturn is_digit(c) ||[m
[32m+[m	[32m       (c >= 'a' && c <= 'f') ||[m
[32m+[m	[32m       (c >= 'A' && c <= 'F');[m
[32m+[m[32m}[m
[32m+[m
[32m+[m[32mstatic int[m
[32m+[m[32mhex_val(char c)[m
[32m+[m[32m{[m
[32m+[m	[32mif (c >= '0' && c <= '9') return c - '0';[m
[32m+[m	[32mif (c >= 'a' && c <= 'f') return c - 'a' + 10;[m
[32m+[m	[32mreturn c - 'A' + 10;[m
[32m+[m[32m}[m
[32m+[m
 static void[m
 add_token(enum token_type type, char *start, int length)[m
 {[m
[1mdiff --git a/src/tokenizer.h b/src/tokenizer.h[m
[1mindex 70feaf1..208305c 100644[m
[1m--- a/src/tokenizer.h[m
[1m+++ b/src/tokenizer.h[m
[36m@@ -12,12 +12,19 @@[m
 #define MAX_TOKENS 1024[m
 [m
 enum token_type {[m
[31m-	LETT, PUBT, FNT, RETT,[m
[31m-	IDENTT, NUMT, STRT,[m
[32m+[m	[32mWORDT, BYTET, PUBT, FNT, RETT,[m
[32m+[m	[32mIFT, ELSET, WHILET, FORT,[m
[32m+[m	[32mIDENTT, NUMT, STRT, CHARLITT,[m
 	LBRAT, RBRAT,[m
 	LBRAC, RBRAC,[m
 	LPAREN, RPAREN,[m
 	COMMAT, EQUALT, SEMIT,[m
[32m+[m	[32mLTT, GTT, EQT, NEQ, LET, GET,[m
[32m+[m	[32mLSHIFTT, RSHIFTT,[m
[32m+[m	[32mPLUST, MINUST, START, SLASHT,[m
[32m+[m	[32mAMP, PIPE, CARET,[m
[32m+[m	[32mTILDE, BANG,[m
[32m+[m	[32mAMPAMP, PIPEPIPE,[m
 	EOFT[m
 };[m
 [m
[1mdiff --git a/zc b/zc[m
[1mdeleted file mode 100755[m
[1mindex adfc656..0000000[m
Binary files a/zc and /dev/null differ
