/*
 * expr.c
 * expression codegen module for rc
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
#include "stmt.h"
#include "var.h"

/* precedence chain, lowest first */
static void lor_eax(void);
static void land_eax(void);
static void bor_eax(void);
static void bxor_eax(void);
static void band_eax(void);
static void eq_eax(void);
static void rel_eax(void);
static void shift_eax(void);
static void sum_eax(void);
static void muldiv_eax(void);
static void unary_eax(void);
static void primary_eax(void);

/*
 * True when dereferencing s yields a byte: one pointer level
 * left and it points to bytes. Deeper levels still hold
 * pointers, which are words. Arrays deref to their element
 * type directly.
 */
int deref_is_byte(struct sym *s)
{
	return s->is_ptr == 1 && s->base == T_BYTE;
}

int elem_is_byte(struct sym *s)
{
	return s->dim > 0 ? s->base == T_BYTE : deref_is_byte(s);
}

/*
 * Symbol a dereference applies to, looking through opening
 * parentheses: *p, *(p), *(p + i). Returns NULL when the
 * operand does not start with a variable name.
 */
struct sym *
deref_target_sym(void)
{
	int p = pos;

	while (tokens[p].type == LPT)
		p++;
	if (tokens[p].type != IDT)
		return NULL;
	return sym_lookup(tokens[p].start, tokens[p].length);
}

int is_lit(void)
{
	return is(INTT) || is(CHART) ||
	       (is(SUBT) && tokens[pos + 1].type == INTT);
}

/*
 * True when the token at position p is a binary operator,
 * so a lone literal or variable must not be taken as the
 * whole expression.
 */
int op_follows(int p)
{
	switch (tokens[p].type) {
	case ADDT: case SUBT: case MULT: case DIVT: case MODT:
	case SLT: case SRT:
	case LTT: case GTT: case LET: case GET:
	case EQQT: case NOTIT:
	case BANDT: case BORT: case XORT:
	case ANDT: case ORT:
		return 1;
	default:
		return 0;
	}
}

/* Evaluates a full expression into %eax. */
void expr_eax(void)
{
	lor_eax();
}

/* Evaluates a full expression into the given register. */
void expr_into(const char *reg)
{
	expr_eax();
	if (strcmp(reg, "eax") != 0)
		emit(code, "\tmov\t%%eax, %%%s\n", reg);
}

/*
 * Evaluates a full expression into the stack slot at the
 * given offset.
 */
void eval_expr(int off)
{
	expr_eax();
	emit(code, "\tmov\t%%eax, %d(%%ebp)\n", off);
}

/* Emits incl/decl on the cell of s. */
static void
emit_incdec(int inc, struct sym *s)
{
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
 * Adds a runtime index to the address held in reg: the base is
 * saved, any expression is evaluated as the index and scaled by
 * stride, then the base is restored and added. The caller
 * consumes the closing bracket.
 */
void emit_index_add(int stride, const char *reg)
{
	emit(code, "\tpush\t%%%s\n", reg);
	expr_eax();
	if (stride != 1)
		emit(code, "\timul\t$%d, %%eax\n", stride);
	emit(code, "\tmov\t%%eax, %%ecx\n\tpop\t%%%s\n"
		   "\tadd\t%%ecx, %%%s\n", reg, reg);
}

/*
 * Consumes one [ ... ] group adding its scaled offset to the
 * address in %eax. A lone literal takes the immediate add fast
 * path; any other expression goes through emit_index_add.
 */
static void
index_step(int stride)
{
	int idx;

	pos++;
	if (is(INTT) && tokens[pos + 1].type == RBCT) {
		idx = num_val(&tokens[pos]);
		pos++;
		expect(RBCT);
		if (idx != 0)
			emit(code, "\tadd\t$%d, %%eax\n", idx * stride);
		return;
	}

	emit_index_add(stride, "eax");
	expect(RBCT);
}

/*
 * Loads through indexes from the address in %eax. Each step
 * adds the scaled index and then loads, so [i][j] chains deref
 * repeatedly. Byte loads zero extend.
 */
static void
index_load(int stride, int byt)
{
	while (is(LBCT)) {
		index_step(stride);
		if (byt)
			emit(code, "\tmovzbl\t(%%eax), %%eax\n");
		else
			emit(code, "\tmov\t(%%eax), %%eax\n");
	}
}

/*
 * Same as index_load but only computes addresses: used to take
 * the address of an element.
 */
static void
index_ref(int stride)
{
	while (is(LBCT))
		index_step(stride);
}

/*
 * Applies a binary operator with the left side already in
 * %eax: saves it across the evaluation of the right side,
 * which lands in %ebx.
 */
static void
binop(const char *insn, void (*sub)(void))
{
	pos++;
	emit(code, "\tpush\t%%eax\n");
	sub();
	emit(code, "\tmov\t%%eax, %%ebx\n\tpop\t%%eax\n");
	emit(code, "\t%s\t%%ebx, %%eax\n", insn);
}

/* Applies a comparison producing 0/1 in %eax. */
static void
compare(const char *cc, void (*sub)(void))
{
	pos++;
	emit(code, "\tpush\t%%eax\n");
	sub();
	emit(code, "\tmov\t%%eax, %%ebx\n\tpop\t%%eax\n");
	emit(code, "\tcmp\t%%ebx, %%eax\n"
		   "\tset%s\t%%al\n\tmovzbl\t%%al, %%eax\n", cc);
}

/* || with short-circuit: each pair gets its own labels. */
static void
lor_eax(void)
{
	land_eax();
	while (is(ORT)) {
		int t = new_label(), end = new_label();

		pos++;
		emit(code, "\ttest\t%%eax, %%eax\n\tjne\tL%d\n", t);
		land_eax();
		emit(code, "\ttest\t%%eax, %%eax\n"
			   "\tsetne\t%%al\n\tmovzbl\t%%al, %%eax\n"
			   "\tjmp\tL%d\n", end);
		emit(code, "L%d:\n\tmov\t$1, %%eax\nL%d:\n", t, end);
	}
}

/* && with short-circuit: each pair gets its own labels. */
static void
land_eax(void)
{
	bor_eax();
	while (is(ANDT)) {
		int f = new_label(), end = new_label();

		pos++;
		emit(code, "\ttest\t%%eax, %%eax\n\tje\tL%d\n", f);
		bor_eax();
		emit(code, "\ttest\t%%eax, %%eax\n"
			   "\tsetne\t%%al\n\tmovzbl\t%%al, %%eax\n"
			   "\tjmp\tL%d\n", end);
		emit(code, "L%d:\n\txor\t%%eax, %%eax\nL%d:\n", f, end);
	}
}

static void
bor_eax(void)
{
	bxor_eax();
	while (is(BORT))
		binop("or", bxor_eax);
}

static void
bxor_eax(void)
{
	band_eax();
	while (is(XORT))
		binop("xor", band_eax);
}

static void
band_eax(void)
{
	eq_eax();
	while (is(BANDT))
		binop("and", eq_eax);
}

static void
eq_eax(void)
{
	rel_eax();
	while (is(EQQT) || is(NOTIT))
		compare(is(EQQT) ? "e" : "ne", rel_eax);
}

static void
rel_eax(void)
{
	const char *cc;

	shift_eax();
	while (is(LTT) || is(GTT) || is(LET) || is(GET)) {
		cc = is(LTT) ? "l" : is(GTT) ? "g" : is(LET) ? "le" : "ge";
		compare(cc, shift_eax);
	}
}

static void
shift_eax(void)
{
	int lft;

	sum_eax();
	while (is(SLT) || is(SRT)) {
		lft = is(SLT);

		pos++;
		emit(code, "\tpush\t%%eax\n");
		sum_eax();
		emit(code, "\tmov\t%%eax, %%ecx\n\tpop\t%%eax\n");
		emit(code, lft ? "\tshl\t%%cl, %%eax\n" : "\tsar\t%%cl, %%eax\n");
	}
}

static void
sum_eax(void)
{
	muldiv_eax();
	while (is(ADDT) || is(SUBT))
		binop(is(ADDT) ? "add" : "sub", muldiv_eax);
}

static void
muldiv_eax(void)
{
	int div, mod;

	unary_eax();
	while (is(MULT) || is(DIVT) || is(MODT)) {
		div = is(DIVT) || is(MODT);
		mod = is(MODT);

		pos++;
		emit(code, "\tpush\t%%eax\n");
		unary_eax();
		emit(code, "\tmov\t%%eax, %%ebx\n\tpop\t%%eax\n");
		if (div) {
			emit(code, "\txor\t%%edx, %%edx\n\tdiv\t%%ebx\n");
			if (mod)
				emit(code, "\tmov\t%%edx, %%eax\n");
		} else {
			emit(code, "\tmul\t%%ebx\n");
		}
	}
}

/*
 * Unary operators: logical not, bitwise not, prefix inc/dec,
 * unary minus, dereference and address-of.
 */
static void
unary_eax(void)
{
	struct sym *s;
	int byt;

	if (is(NOTT)) {
		pos++;
		unary_eax();
		emit(code, "\ttest\t%%eax, %%eax\n"
			   "\tsete\t%%al\n\tmovzbl\t%%al, %%eax\n");
		return;
	}

	if (is(BNOTT)) {
		pos++;
		unary_eax();
		emit(code, "\tnot\t%%eax\n");
		return;
	}

	if (is(INCT) || is(DECT)) {
		int inc = is(INCT);

		pos++;
		s = sym_find(tokens[pos].start, tokens[pos].length);
		expect(IDT);
		emit_incdec(inc, s);
		if (s->kind == LOC)
			emit(code, "\tmov\t%d(%%ebp), %%eax\n", s->off);
		else
			emit(code, "\tmov\t%.*s, %%eax\n", s->length, s->start);
		return;
	}

	if (is(SUBT)) {
		pos++;
		if (is(INTT)) {
			/* negative literal keeps its sign in the mov */
			emit(code, "\tmov\t$-%d, %%eax\n",
			     num_val(&tokens[pos]));
			pos++;
			return;
		}
		unary_eax();
		emit(code, "\tneg\t%%eax\n");
		return;
	}

	if (is(MULT)) {
		byt = 0;
		pos++;
		s = deref_target_sym();
		if (s)
			byt = elem_is_byte(s);
		unary_eax();
		if (byt)
			emit(code, "\tmovzbl\t(%%eax), %%eax\n");
		else
			emit(code, "\tmov\t(%%eax), %%eax\n");
		index_load(4, 0);
		return;
	}

	if (is(BANDT)) {
		int stride;

		pos++;
		if (is(IDT)) {
			s = sym_find(tokens[pos].start, tokens[pos].length);
			pos++;

			if (is(LBCT) && s->dim == 0 && !s->is_ptr)
				fatal(USER_ERR, NULL,
				      "Indexing a non-pointer!");

			stride = elem_is_byte(s) ? 1 : 4;
			if (!is(LBCT)) {
				/* plain address of the cell */
				if (s->kind == LOC)
					emit(code, "\tlea\t%d(%%ebp), %%eax\n",
					     s->off);
				else
					emit(code, "\tlea\t%.*s, %%eax\n",
					     s->length, s->start);
			} else if (s->kind == LOC && s->dim > 0) {
				/* &arr[i] walks from the buffer base */
				emit(code, "\tlea\t%d(%%ebp), %%eax\n",
				     s->off);
				index_ref(stride);
			} else {
				/* &p[i] walks from the pointer value */
				if (s->kind == LOC)
					emit(code,
					     "\tmov\t%d(%%ebp), %%eax\n",
					     s->off);
				else
					emit(code, "\tmov\t%.*s, %%eax\n",
					     s->length, s->start);
				index_ref(stride);
			}
		} else if (is(MULT)) {
			/* &(*x) is just x */
			pos++;
			unary_eax();
			emit(code, "\tlea\t(%%eax), %%eax\n");
		} else {
			fatal(USER_ERR, NULL, "Expected variable after &!");
		}
		return;
	}

	primary_eax();
}

/*
 * A primary: literals, arg[N], parenthesized expressions,
 * calls, variables with their constant indexes and postfix
 * inc/dec.
 */
static void
primary_eax(void)
{
	struct sym *s;
	int n;

	if (is(STRT)) {
		/* the string direction on .data */
		int lab = reg_str();

		emit(code, "\tmov\t$str%d, %%eax\n", lab);
		pos++;
	} else if (is(INTT) || is(CHART)) {
		if (is(CHART))
			emit(code, "\tmov\t$0x%x, %%eax\n",
			     num_val(&tokens[pos]));
		else
			emit(code, "\tmov\t$%d, %%eax\n",
			     num_val(&tokens[pos]));
		pos++;
	} else if (is(FLOATT)) {
		fatal(USER_ERR, NULL, "Float expressions not implemented yet!");
	} else if (is(ARGT)) {
		pos++;
		expect(LBCT);
		n = num_val(&tokens[pos]);
		pos++;
		expect(RBCT);
		emit(code, "\tmov\t%d(%%ebp), %%eax\n", n * 4 + 8);
		index_load(4, 0);
	} else if (is(LPT)) {
		pos++; /* skip ( */
		expr_eax();
		expect(RPT);
		index_load(4, 0);
	} else if (is(IDT)) {
		s = sym_lookup(tokens[pos].start, tokens[pos].length);

		/* undeclared name followed by ( is a call */
		if (!s && tokens[pos + 1].type == LPT) {
			parse_call();
			return;
		}
		if (!s)
			fatal(USER_ERR, NULL, "Unknown variable '%.*s'!",
			      tokens[pos].length, tokens[pos].start);
		pos++;

		if ((s->is_ptr || s->dim > 0) && is(LBCT)) {
			int byt = elem_is_byte(s);
			int stride = byt ? 1 : 4;

			/* an array name is the address of element 0 */
			if (s->kind == LOC) {
				if (s->dim > 0)
					emit(code,
					     "\tlea\t%d(%%ebp), %%eax\n",
					     s->off);
				else
					emit(code,
					     "\tmov\t%d(%%ebp), %%eax\n",
					     s->off);
			} else {
				emit(code, "\tmov\t%.*s, %%eax\n",
				     s->length, s->start);
			}
			index_load(stride, byt);
			return;
		}

		/* bare array name decays to its first element */
		if (s->kind == LOC) {
			if (s->dim > 0)
				emit(code, "\tlea\t%d(%%ebp), %%eax\n",
				     s->off);
			else
				emit(code, "\tmov\t%d(%%ebp), %%eax\n",
				     s->off);
		} else {
			emit(code, "\tmov\t%.*s, %%eax\n", s->length, s->start);
		}

		/* postfix ++/-- keeps the old value in %eax */
		if (is(INCT) || is(DECT)) {
			emit_incdec(is(INCT), s);
			pos++;
		}
	} else {
		fatal(USER_ERR, NULL, "Expression not implemented yet! tok: %s",
		      t_name(tokens[pos].type));
	}
}
