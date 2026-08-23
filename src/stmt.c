/*
 * stmt.c
 * statement codegen module for rc
 * Copyright (c) 2026 Pablo Trik Marin
 * License: GPL
 */

#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "tokenizer.h"
#include "parser.h"
#include "symtab.h"
#include "expr.h"
#include "var.h"
#include "stmt.h"

struct argdesc {
	int kind;
	int lab;
	FILE *buf;
};

static void parse_stmts(FILE *out);
static void parse_stmt(FILE *out);
static void parse_body(FILE *out);
static void parse_if(FILE *out);
static void parse_while(FILE *out);
static void parse_for(FILE *out);
static void parse_var_local(FILE *out);
static void parse_ret(FILE *out);
static int compound_op(enum token_type t);
static void emit_binop(enum token_type op);
static void emit_incdec(int inc, struct sym *s);
static void parse_deref_assign(FILE *out);
static void parse_indexed_assign(FILE *out);
static int skip_index(int p);

/*
 * Parses a function definition and emits its code into the
 * .text section (accumulated in a temporary file). The first
 * function creates that temporary file.
 */
void parse_fn(enum visi vis, FILE *out)
{
	expect(FNT);
	expect(IDT);

	if (!has_fn) {
		code = tmpfile();
		has_fn = 1;
	}

	if (vis == PUB)
		emit(code, "\t.globl\t%.*s\n", tokens[pos - 1].length, tokens[pos - 1].start);
	emit(code, "%.*s:\n", tokens[pos - 1].length, tokens[pos - 1].start);
	emit(code, "\tpush\t%%ebp\n\tmov\t%%esp, %%ebp\n\n");

	sym_locals_clear();

	expect(LBKT);
	parse_stmts(out);
	expect(RBKT);

	emit(code, "\tmov\t%%ebp, %%esp\n\tpop\t%%ebp\n\tret\n");
}

/*
 * Parses the statements of a function body until the closing
 * brace. Statements are separated by newlines, semicolons are
 * optional and ignored.
 */
static void
parse_stmts(FILE *out)
{
	while (!is(RBKT)) {
		while (is(NEWT) || is(SEMIT))
			pos++;

		if (is(RBKT))
			break;
		if (is(EOFT))
			fatal(USER_ERR, NULL, "Unexpected end of file inside a function!");

		parse_stmt(out);
	}
}

/*
 * Parses the body of a control flow statement: either a block
 * between braces or the single statement that follows.
 */
static void
parse_body(FILE *out)
{
	if (accept(LBKT)) {
		parse_stmts(out);
		expect(RBKT);
		return;
	}

	while (is(NEWT) || is(SEMIT))
		pos++;
	parse_stmt(out);
}

/*
 * Parses a single statement. Supported: function calls,
 * variable declarations, returns, control flow (if, while,
 * for), increment/decrement, assignments and calls.
 */
static void
parse_stmt(FILE *out)
{
	struct sym *s;

	if (is(ASMT)) {
		parse_asm();
		return;
	}

	if (is(SYST) && tokens[pos + 1].type == LPT) {
		parse_syscall();
		return;
	}

	if (is(IDT) && tokens[pos + 1].type == LPT) {
		parse_call();
		return;
	}

	if (is(SETWT) || is(SETBT)) {
		parse_var_local(out);
		return;
	}

	if (is(RETT)) {
		parse_ret(out);
		return;
	}

	if (is(IFT)) {
		parse_if(out);
		return;
	}

	if (is(WHILET)) {
		parse_while(out);
		return;
	}

	if (is(FORT)) {
		parse_for(out);
		return;
	}

	/* prefix increment/decrement: ++i / --i */
	if (is(INCT) || is(DECT)) {
		int inc = is(INCT);

		pos++;
		emit_incdec(inc, NULL);
		emit(code, "\n");
		return;
	}

	/* postfix increment/decrement: i++ / i-- */
	if (is(IDT) && (tokens[pos + 1].type == INCT ||
			tokens[pos + 1].type == DECT)) {
		s = sym_find(tokens[pos].start, tokens[pos].length);
		emit_incdec(tokens[pos + 1].type == INCT, s);
		pos += 2;
		emit(code, "\n");
		return;
	}

	/* assignment through a pointer: *a = expr / *a op= expr */
	if (is(MULT)) {
		parse_deref_assign(out);
		return;
	}

	/* indexed assignment: p[i] = expr / p[i] op= expr */
	if (is(IDT) && tokens[pos + 1].type == LBCT) {
		int j = skip_index(pos + 1);

		if (j > 0 && (tokens[j].type == EQT ||
			      compound_op(tokens[j].type))) {
			parse_indexed_assign(out);
			return;
		}
	}

	/* assignment: var = expr / var op= expr */
	if (is(IDT) && (tokens[pos + 1].type == EQT ||
			compound_op(tokens[pos + 1].type))) {
		struct sym *t = sym_find(tokens[pos].start, tokens[pos].length);
		enum token_type op = tokens[pos + 1].type;

		if (t->dim > 0)
			fatal(USER_ERR, NULL, "Can't assign to an array!");

		pos += 2; /* IDT EQ */
		if (op == EQT) {
			if (t->kind == LOC)
				eval_expr(t->off);
			else {
				expr_eax();
				emit(code, "\tmov\t%%eax, %.*s\n",
				     t->length, t->start);
			}
		} else {
			expr_into("ebx");
			if (t->kind == LOC)
				emit(code, "\tmov\t%d(%%ebp), %%eax\n", t->off);
			else
				emit(code, "\tmov\t%.*s, %%eax\n",
				     t->length, t->start);
			emit_binop(op);
			if (t->kind == LOC)
				emit(code, "\tmov\t%%eax, %d(%%ebp)\n", t->off);
			else
				emit(code, "\tmov\t%%eax, %.*s\n",
				     t->length, t->start);
		}
		emit(code, "\n");
		return;
	}

	fatal(USER_ERR, NULL, "Statement not implemented yet! tok: %s", t_name(tokens[pos].type));
}

/*
 * Emits incl/decl on the variable found at the current
 * position when s is NULL, otherwise on s. The identifier is
 * consumed only when s is NULL.
 */
static void
emit_incdec(int inc, struct sym *s)
{
	if (!s) {
		if (!is(IDT))
			fatal(USER_ERR, NULL, "Expected variable after ++/--!");
		s = sym_find(tokens[pos].start, tokens[pos].length);
		pos++;
	}

	if (s->dim > 0)
		fatal(USER_ERR, NULL, "Can't increment an array!");

	if (s->kind == LOC)
		emit(code, inc ? "\tincl\t%d(%%ebp)\n" : "\tdecl\t%d(%%ebp)\n",
		     s->off);
	else
		emit(code, inc ? "\tincl\t%.*s\n" : "\tdecl\t%.*s\n",
		     s->length, s->start);
}

/*
 * Parses an if statement with an optional else. The body is a
 * block between braces or the single statement that follows.
 * An else-if chains naturally through recursion.
 */
static void
parse_if(FILE *out)
{
	int lelse, lend;

	expect(IFT);

	lelse = new_label();
	lend = new_label();

	expr_eax();
	emit(code, "\ttest\t%%eax, %%eax\n\tje\tL%d\n\n", lelse);

	parse_body(out);

	while (is(NEWT) || is(SEMIT))
		pos++;
	if (accept(ELSET)) {
		emit(code, "\tjmp\tL%d\nL%d:\n", lend, lelse);
		if (is(IFT))
			parse_if(out);
		else
			parse_body(out);
		emit(code, "L%d:\n\n", lend);
	} else {
		emit(code, "L%d:\n\n", lelse);
	}
}

/*
 * Parses a while loop: the condition jumps out of the body,
 * the body falls back to the condition.
 */
static void
parse_while(FILE *out)
{
	int ltop, lend;

	expect(WHILET);

	ltop = new_label();
	lend = new_label();

	emit(code, "L%d:\n", ltop);
	expr_eax();
	emit(code, "\ttest\t%%eax, %%eax\n\tje\tL%d\n\n", lend);

	parse_body(out);

	emit(code, "\tjmp\tL%d\nL%d:\n\n", ltop, lend);
}

/*
 * Parses a for loop with the syntax: for init, cond, step.
 * The init runs once, the condition guards every round and
 * the step runs after each round of the body.
 */
static void
parse_for(FILE *out)
{
	int lcond, lend;
	FILE *stepbuf, *saved;

	expect(FORT);

	lcond = new_label();
	lend = new_label();

	/* init: declaration or simple statement */
	if (is(SETWT) || is(SETBT))
		parse_var_local(out);
	else if (!is(COMT))
		parse_stmt(out);
	accept(COMT);

	emit(code, "L%d:\n", lcond);
	if (!is(COMT)) {
		expr_eax();
		emit(code, "\ttest\t%%eax, %%eax\n\tje\tL%d\n", lend);
	}
	expect(COMT);

	/*
	 * The step comes before the body in the token stream
	 * but must run after it, so it is parsed into a buffer.
	 */
	stepbuf = NULL;
	if (!is(NEWT) && !is(LBKT)) {
		stepbuf = tmpfile();
		saved = code;
		code = stepbuf;
		parse_stmt(out);
		code = saved;
	}

	parse_body(out);

	if (stepbuf) {
		char buf[1024];
		int k;

		rewind(stepbuf);
		while ((k = (int)fread(buf, 1, sizeof buf, stepbuf)) > 0)
			fwrite(buf, 1, (size_t)k, code);
		fclose(stepbuf);
	}

	emit(code, "\tjmp\tL%d\nL%d:\n\n", lcond, lend);
}

/*
 * Parses a local variable declaration and emits the stack
 * allocation plus the initialization. A bare literal is
 * stored directly into the slot, any other initializer goes
 * through the expression evaluator. Byte variables are stored
 * with movb, all slots are 4 bytes wide.
 */
static void
parse_var_local(FILE *out)
{
	struct token *name;
	bool word = is(SETWT);
	int off;
	int ptr;
	int dim = 0;

	expect(is(SETBT) ? SETBT : SETWT);
	ptr = 0;
	while (accept(MULT))
		ptr++;

	expect(IDT);
	name = &tokens[pos - 1];

	/* array dimensions */
	while (accept(LBCT)) {
		int v;

		if (!is(INTT)) {
			expect(RBCT);
			break;
		}
		v = num_val(&tokens[pos]);
		pos++;
		expect(RBCT);
		dim = dim ? dim * v : v;
	}

	/*
	 * An initialized declaration keeps the historical
	 * behavior: the address of the literal lands in a single
	 * 4-byte cell and the dimensions are ignored. Without an
	 * initializer, an array reserves its full size on the
	 * stack and its name decays to the address of element 0,
	 * so the slot offset is the base of the buffer.
	 */
	if (is(EQT))
		dim = 0;

	if (dim > 0) {
		int size = dim * (word ? 4 : 1);

		off = sym_local_alloc(size);
		sym_register(LOC, name->start, name->length, off, ptr,
			     word ? T_WORD : T_BYTE, dim);
		emit(code, "\tsub\t$%d, %%esp\n", size);
	} else {
		off = sym_local_alloc(4);
		sym_register(LOC, name->start, name->length, off, ptr,
			     word ? T_WORD : T_BYTE, 0);
		emit(code, "\tsub\t$4, %%esp\n");
	}

	if (accept(EQT)) {
		if (is_lit() && !op_follows(pos + (is(SUBT) ? 2 : 1))) {
			/* literal */
			int sign = 1;

			if (is(SUBT)) {
				pos++;
				sign = -1;
			}
			if (is(CHART))
				emit(code, word ? "\tmov\t$0x%x, %d(%%ebp)\n"
					: "\tmovb\t$0x%x, %d(%%ebp)\n",
					num_val(&tokens[pos]), off);
			else if (sign < 0)
				emit(code, word ? "\tmov\t$-%d, %d(%%ebp)\n"
					: "\tmovb\t$-%d, %d(%%ebp)\n",
					num_val(&tokens[pos]), off);
			else
				emit(code, word ? "\tmov\t$%d, %d(%%ebp)\n"
					: "\tmovb\t$%d, %d(%%ebp)\n",
					num_val(&tokens[pos]), off);
			pos++;
		} else if (is(STRT) && dim == 0) {
			/* string initializer */
			int lab = reg_str();
			pos++;
			emit(code, "\tlea\tstr%d, %%eax\n", lab);
			emit(code, "\tmov\t%%eax, %d(%%ebp)\n", off);
		} else if (is(LBKT)) {
			/* array literal initializer */
			int lab = reg_arr();
			emit(code, "\tlea\tarr%d, %%eax\n", lab);
			emit(code, "\tmov\t%%eax, %d(%%ebp)\n", off);
		} else if (is(BANDT)) {
			/* address-of initializer */
			pos++;
			if (is(IDT)) {
				struct sym *s = sym_find(tokens[pos].start, tokens[pos].length);
				pos++;
				if (s->kind == LOC)
					emit(code, "\tlea\t%d(%%ebp), %%eax\n", s->off);
				else
					emit(code, "\tlea\t%.*s, %%eax\n",
						s->length, s->start);
				emit(code, "\tmov\t%%eax, %d(%%ebp)\n", off);
			} else if (is(MULT)) {
				/* &(*x) = address of x */
				pos++;
				if (is(IDT)) {
					struct sym *s = sym_find(tokens[pos].start, tokens[pos].length);
					pos++;
					if (s->kind == LOC)
						emit(code, "\tlea\t%d(%%ebp), %%eax\n", s->off);
					else
						emit(code, "\tlea\t%.*s, %%eax\n",
							s->length, s->start);
					emit(code, "\tmov\t%%eax, %d(%%ebp)\n", off);
				}
			}
		} else if (!word && ptr == 0) {
			/* byte expressions store their low byte */
			expr_eax();
			emit(code, "\tmovb\t%%al, %d(%%ebp)\n", off);
		} else {
			/* pointers are words regardless of setw/setb */
			eval_expr(off);
		}
	}

	emit(code, "\n");
}

/*
 * Parses a return statement. The value, when present, is a
 * full expression evaluated into %eax.
 */
static void
parse_ret(FILE *out)
{
	expect(RETT);

	if (!is(NEWT) && !is(RBKT) && !is(SEMIT) && !is(EOFT))
		expr_eax();

	emit(code, "\n");
}

/*
 * Parses a function call with its arguments and emits the
 * pushes (in reverse order), the call and the stack cleanup.
 * Every argument is a full expression: it is evaluated into a
 * temporary buffer during the scan and pushed afterwards so
 * the arguments land in cdecl order.
 */
void parse_call(void)
{
	enum { ASTR, AARR, AEXPR };
	struct argdesc args[32];
	char buf[1024];
	int n = 0, i, k;
	FILE *saved;
	struct token *name = &tokens[pos];

	pos++; /* IDT */
	expect(LPT);

	while (!is(RPT)) {
		if (n >= 32)
			fatal(USER_ERR, NULL, "Too many arguments!");
		if (is(STRT)) {
			args[n].kind = ASTR;
			args[n].lab = reg_str();
			pos++;
		} else if (is(LBKT)) {
			args[n].kind = AARR;
			args[n].lab = reg_arr();
		} else {
			args[n].kind = AEXPR;
			args[n].buf = tmpfile();
			saved = code;
			code = args[n].buf;
			expr_eax();
			code = saved;
		}
		n++;
		if (accept(COMT))
			continue;
		break;
	}
	expect(RPT);

	for (i = n - 1; i >= 0; i--) {
		switch (args[i].kind) {
		case ASTR:
			emit(code, "\tpush\t$str%d\n", args[i].lab);
			break;
		case AARR:
			emit(code, "\tpush\t$arr%d\n", args[i].lab);
			break;
		default:
			rewind(args[i].buf);
			while ((k = (int)fread(buf, 1, sizeof buf, args[i].buf)) > 0)
				fwrite(buf, 1, (size_t)k, code);
			fclose(args[i].buf);
			emit(code, "\tpush\t%%eax\n");
			break;
		}
	}
	emit(code, "\tcall\t%.*s\n", name->length, name->start);
	if (n > 0)
		emit(code, "\tadd\t$%d, %%esp\n", n * 4);
	emit(code, "\n");
}

/*
 * Syntax suport: operator for syscalls. Arguments are pushed
 * and then popped into the registers, so evaluating one can
 * never clobber a register already loaded for another.
 */
void parse_syscall(void)
{
	enum { ASTR, AARR, AEXPR };
	struct argdesc args[6];
	char buf[1024];
	int n = 0, i, k;
	FILE *saved;
	char *regs[] = { "eax", "ebx", "ecx", "edx", "esi", "edi" };

	pos++; /* SYST */
	expect(LPT);

	while (!is(RPT)) {
		if (n >= 6)
			fatal(USER_ERR, NULL, "Too many arguments!");
		if (is(STRT)) {
			args[n].kind = ASTR;
			args[n].lab = reg_str();
			pos++;
		} else if (is(LBKT)) {
			args[n].kind = AARR;
			args[n].lab = reg_arr();
		} else {
			args[n].kind = AEXPR;
			args[n].buf = tmpfile();
			saved = code;
			code = args[n].buf;
			expr_eax();
			code = saved;
		}
		n++;
		if (accept(COMT))
			continue;
		break;
	}
	expect(RPT);

	for (i = 0; i < n; i++) {
		switch (args[i].kind) {
		case ASTR:
			emit(code, "\tpush\t$str%d\n", args[i].lab);
			break;
		case AARR:
			emit(code, "\tpush\t$arr%d\n", args[i].lab);
			break;
		default:
			rewind(args[i].buf);
			while ((k = (int)fread(buf, 1, sizeof buf, args[i].buf)) > 0)
				fwrite(buf, 1, (size_t)k, code);
			fclose(args[i].buf);
			emit(code, "\tpush\t%%eax\n");
			break;
		}
	}
	for (i = n - 1; i >= 0; i--)
		emit(code, "\tpop\t%%%s\n", regs[i]);

	emit(code, "\tint\t$0x80\n");
	emit(code, "\n");
}

/*
 * Syntax suport: raw assembly block. The tokenizer hands the
 * whole body over as one RAW token; its source bytes go into
 * the .text section verbatim, keeping tabs and newlines.
 */
void parse_asm(void)
{
	struct token *t;

	pos++; /* ASMT */
	if (!is(RAWT))
		fatal(USER_ERR, NULL, "Expected '{' after asm!");

	t = &tokens[pos];
	pos++;

	if (t->length > 0) {
		fwrite(t->start, 1, (size_t)t->length, code);
		emit(code, "\n\n");
	}
}

/* True for the compound assignment operators. */
static int
compound_op(enum token_type t)
{
	return t == ADDIT || t == SUBIT || t == MULIT || t == DIVIT ||
	       t == BANDIT || t == BORIT || t == XORIT;
}

/* Stores %eax into the cell pointed by %edi, sized by byt. */
static void
emit_store_ind(int byt)
{
	if (byt)
		emit(code, "\tmovb\t%%al, (%%edi)\n");
	else
		emit(code, "\tmov\t%%eax, (%%edi)\n");
}

/* Applies a compound operator: %eax <op>= %ebx. */
static void
emit_binop(enum token_type op)
{
	switch (op) {
	case ADDIT:
		emit(code, "\tadd\t%%ebx, %%eax\n");
		break;
	case SUBIT:
		emit(code, "\tsub\t%%ebx, %%eax\n");
		break;
	case MULIT:
		emit(code, "\timul\t%%ebx, %%eax\n");
		break;
	case DIVIT:
		emit(code, "\txor\t%%edx, %%edx\n\tdiv\t%%ebx\n");
		break;
	case BANDIT:
		emit(code, "\tand\t%%ebx, %%eax\n");
		break;
	case BORIT:
		emit(code, "\tor\t%%ebx, %%eax\n");
		break;
	case XORIT:
		emit(code, "\txor\t%%ebx, %%eax\n");
		break;
	default:
		break;
	}
}

/*
 * Parses an assignment through a dereference: *a = expr or
 * *a op= expr. The address is evaluated into %edi first, then
 * the right side. Byte pointers store with movb; storing
 * through deeper pointer levels keeps word size, since those
 * cells hold pointers.
 */
static void
parse_deref_assign(FILE *out)
{
	struct sym *s;
	int byt = 0;
	enum token_type op;

	pos++; /* MULT */
	s = deref_target_sym();
	if (s)
		byt = elem_is_byte(s);
	expr_into("edi");

	op = tokens[pos].type;
	if (accept(EQT)) {
		expr_eax();
	} else if (compound_op(op)) {
		pos++;
		expr_into("ebx");
		if (byt)
			emit(code, "\tmovzbl\t(%%edi), %%eax\n");
		else
			emit(code, "\tmov\t(%%edi), %%eax\n");
		emit_binop(op);
	} else {
		fatal(USER_ERR, NULL, "Expected '=' in assignment!");
	}
	emit_store_ind(byt);
	emit(code, "\n");
}

/*
 * Position right after the ] that closes the bracket group
 * opening at p, or -1 when the group is not balanced before a
 * newline or end of file.
 */
static int
skip_index(int p)
{
	int depth = 0;

	for (; tokens[p].type != EOFT && tokens[p].type != NEWT; p++) {
		if (tokens[p].type == LBCT) {
			depth++;
			continue;
		}
		if (tokens[p].type == RBCT && --depth == 0)
			return p + 1;
	}
	return -1;
}

/*
 * Parses an indexed assignment: p[i] = expr or p[i] op= expr.
 * The index may be any expression. The base lands in %edi,
 * each bracket group adds its scaled offset, and the store is
 * sized by the element type: bytes with movb, words four.
 */
static void
parse_indexed_assign(FILE *out)
{
	struct sym *s = sym_find(tokens[pos].start, tokens[pos].length);
	int arr = s->kind == LOC && s->dim > 0;
	int byt, stride, idx;
	enum token_type op;

	if (!s->is_ptr && !arr)
		fatal(USER_ERR, NULL, "Indexed assignment on a non-pointer!");
	byt = elem_is_byte(s);
	stride = byt ? 1 : 4;

	pos++; /* IDT */

	/* an array name is the address of element 0 */
	if (arr)
		emit(code, "\tlea\t%d(%%ebp), %%edi\n", s->off);
	else if (s->kind == LOC)
		emit(code, "\tmov\t%d(%%ebp), %%edi\n", s->off);
	else
		emit(code, "\tmov\t%.*s, %%edi\n", s->length, s->start);

	while (accept(LBCT)) {
		if (is(INTT) && tokens[pos + 1].type == RBCT) {
			idx = num_val(&tokens[pos]);
			pos++;
			expect(RBCT);
			if (idx != 0)
				emit(code, "\tadd\t$%d, %%edi\n", idx * stride);
		} else {
			emit_index_add(stride, "edi");
			expect(RBCT);
		}
	}

	op = tokens[pos].type;
	if (accept(EQT)) {
		expr_eax();
	} else if (compound_op(op)) {
		pos++;
		expr_into("ebx");
		if (byt)
			emit(code, "\tmovzbl\t(%%edi), %%eax\n");
		else
			emit(code, "\tmov\t(%%edi), %%eax\n");
		emit_binop(op);
	} else {
		fatal(USER_ERR, NULL, "Expected '=' in assignment!");
	}
	emit_store_ind(byt);
	emit(code, "\n");
}
