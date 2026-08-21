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
	char text[64];
	int start_pos;
};

static void parse_stmts(FILE *out);
static void parse_stmt(FILE *out);
static void parse_call(FILE *out);
static void parse_var_local(FILE *out);
static void parse_ret(FILE *out);

/*
 * Parses a function definition and emits its code into the
 * .text section (accumulated in a temporary file). The first
 * function creates that temporary file.
 */
void
parse_fn(enum visi vis, FILE *out)
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
	for (;;) {
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
 * Parses a single statement. Currently function calls,
 * variable declarations and return statements are supported.
 */
static void
parse_stmt(FILE *out)
{
	if (is(IDT) && tokens[pos + 1].type == LPT) {
		parse_call(out);
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

	/* assignment: var = expr */
	if (is(IDT) && tokens[pos + 1].type == EQT) {
		struct sym *s = sym_find(tokens[pos].start, tokens[pos].length);
		pos++; /* IDT */
		pos++; /* EQT */
		if (s->kind == LOC)
			eval_expr(s->off);
		else {
			eval_atom("eax");
			emit(code, "\tmov\t%%eax, %.*s\n", s->length, s->start);
		}
		emit(code, "\n");
		return;
	}

	fatal(USER_ERR, NULL, "Statement not implemented yet! tok: %s", t_name(tokens[pos].type));
}

/*
 * Parses a local variable declaration and emits the stack
 * allocation plus the initialization. A bare literal or
 * variable is stored directly into the slot, more complex
 * initializers are evaluated by the expression codegen.
 * Byte variables use the same 4-byte slot but are stored
 * with movb.
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
	if (accept(LBCT)) {
		if (is(INTT)) {
			dim = num_val(&tokens[pos]);
			pos++;
		}
		expect(RBCT);
	}

	off = -(sym_local_count() + 1) * 4;
	sym_register(LOC, name->start, name->length, off, ptr);

	emit(code, "\tsub\t$4, %%esp\n");

	if (accept(EQT)) {
		if (is_lit() && !next_is_arith()) {
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
			int lab = reg_str(out);
			pos++;
			emit(code, "\tlea\tstr%d, %%eax\n", lab);
			emit(code, "\tmov\t%%eax, %d(%%ebp)\n", off);
		} else if (is(LBKT)) {
			/* array literal initializer */
			int lab = reg_arr(out);
			emit(code, "\tlea\tarr%d, %%eax\n", lab);
			emit(code, "\tmov\t%%eax, %d(%%ebp)\n", off);
		} else if (is(IDT) && !next_is_arith() &&
			   tokens[pos + 1].type != LPT &&
			   tokens[pos + 1].type != LBCT) {
			/* copy */
			struct sym *src = sym_find(tokens[pos].start, tokens[pos].length);

			pos++;
			if (src->kind == LOC)
				emit(code, word
					? "\tmov\t%d(%%ebp), %%eax\n"
					: "\tmovb\t%d(%%ebp), %%al\n", src->off);
			else
				emit(code, word
					? "\tmov\t%.*s, %%eax\n"
					: "\tmovb\t%.*s, %%al\n",
					src->length, src->start);
			emit(code, word
				? "\tmov\t%%eax, %d(%%ebp)\n"
				: "\tmovb\t%%al, %d(%%ebp)\n", off);
		} else if (is(IDT) && tokens[pos + 1].type == LPT) {
			/* call as initializer */
			parse_call(out);
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
		} else if (is(MULT)) {
			/* dereference initializer */
			pos++;
			if (is(IDT)) {
				struct sym *s = sym_find(tokens[pos].start, tokens[pos].length);
				pos++;
				if (s->kind == LOC)
					emit(code, "\tmov\t%d(%%ebp), %%eax\n", s->off);
				else
					emit(code, "\tmov\t%.*s, %%eax\n",
						s->length, s->start);
				emit(code, "\tmov\t(%%eax), %%eax\n");
				emit(code, "\tmov\t%%eax, %d(%%ebp)\n", off);
			}
		} else if (!word) {
			fatal(USER_ERR, NULL, "Byte expressions not implemented yet!");
		} else {
			eval_expr(off);
		}
	}

	emit(code, "\n");
}

/*
 * Parses a return statement. The value is loaded into %eax.
 * Literals, variables and add/subtract expressions are
 * supported. The left side goes into %eax and every operator
 * applies its right side directly as a memory operand.
 */
static void
parse_ret(FILE *out)
{
	expect(RETT);

	if (is(IDT) && tokens[pos + 1].type == LPT) {
		parse_call(out);
	} else {
		emit(code, "\tmov\t%s, %%eax\n", atom_operand());
	}
	while (is(ADDT) || is(SUBT)) {
		bool add = accept(ADDT);

		if (!add)
			pos++;
		emit(code, add ? "\tadd\t%s, %%eax\n" : "\tsub\t%s, %%eax\n",
		     atom_operand());
	}

	emit(code, "\n");
}

/*
 * Parses a function call with its arguments and emits the
 * pushes (in reverse order), the call and the stack cleanup.
 * Strings and array literals are emitted to the .data section
 * as strN/arrN labels, every other argument is pushed as a
 * raw operand (literal or variable).
 */
static void
parse_call(FILE *out)
{
	enum { ASTR, AARR, AOP, AAE, ACALL };
	struct argdesc args[32];
	int n = 0, i;
	struct token *name = &tokens[pos];

	pos++; /* IDT */
	expect(LPT);

	while (!is(RPT)) {
		if (n >= 32)
			fatal(USER_ERR, NULL, "Too many arguments!");
		if (is(STRT)) {
			args[n].kind = ASTR;
			args[n].lab = reg_str(out);
			pos++;
		} else if (is(LBKT)) {
			args[n].kind = AARR;
			args[n].lab = reg_arr(out);
		} else if (is(ARGT)) {
			/* check for chained subscripts: arg[N][M] needs AAE */
			int peek = pos + 4;
			if (tokens[peek].type == LBCT) {
				args[n].kind = AAE;
				args[n].start_pos = pos;
				pos++;
				while (is(LBCT)) { pos++; pos++; pos++; }
			} else {
				args[n].kind = AOP;
				snprintf(args[n].text, sizeof args[n].text, "%s",
					 atom_operand());
			}
		} else if (is(MULT) || is(BANDT) || is(LPT)) {
			args[n].kind = AAE;
			args[n].start_pos = pos;
			/* skip past the expression */
			if (is(ARGT)) {
				pos++;
				while (is(LBCT)) { pos++; pos++; pos++; }
			} else if (is(BANDT)) {
				pos++;
				if (is(MULT)) { pos++; while (is(LBCT)) { pos++; pos++; pos++; } }
				else if (is(IDT)) { pos++; while (is(LBCT)) { pos++; pos++; pos++; } }
				else if (is(LPT)) {
					int depth = 1; pos++;
					while (depth > 0) {
						if (is(LPT)) depth++;
						else if (is(RPT)) depth--;
						if (depth > 0) pos++;
					}
					pos++;
					while (is(LBCT)) { pos++; pos++; pos++; }
				}
			} else if (is(MULT)) {
				pos++;
				if (is(LPT)) {
					int depth = 1; pos++;
					while (depth > 0) {
						if (is(LPT)) depth++;
						else if (is(RPT)) depth--;
						if (depth > 0) pos++;
					}
					pos++;
				} else if (is(IDT)) {
					pos++;
				}
				while (is(LBCT)) { pos++; pos++; pos++; }
			} else if (is(LPT)) {
				int depth = 1; pos++;
				while (depth > 0) {
					if (is(LPT)) depth++;
					else if (is(RPT)) depth--;
					if (depth > 0) pos++;
				}
				pos++;
				while (is(LBCT)) { pos++; pos++; pos++; }
			}
		} else if (is(IDT) && tokens[pos + 1].type == LPT) {
			args[n].kind = ACALL;
			args[n].start_pos = pos;
			/* skip IDT, LPT, args..., RPT */
			pos += 2;
			{ int depth = 1;
			while (depth > 0) {
				if (is(LPT)) depth++;
				else if (is(RPT)) depth--;
				if (depth > 0) pos++;
			}
			pos++; /* final RPT */
			}
		} else {
			args[n].kind = AOP;
			snprintf(args[n].text, sizeof args[n].text, "%s",
				 atom_operand());
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
		case AAE:
			{
				int saved = pos;
				pos = args[i].start_pos;
				eval_atom("eax");
				emit(code, "\tpush\t%%eax\n");
				pos = saved;
			}
			break;
		case ACALL:
			{
				int saved = pos;
				pos = args[i].start_pos;
				parse_call(out);
				emit(code, "\tpush\t%%eax\n");
				pos = saved;
			}
			break;
		default:
			emit(code, "\tpush\t%s\n", args[i].text);
			break;
		}
	}
	emit(code, "\tcall\t%.*s\n", name->length, name->start);
	if (n > 0)
		emit(code, "\tadd\t$%d, %%esp\n", n * 4);
	emit(code, "\n");
}
