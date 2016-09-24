/* This file is not complete.  You should fill it in with your
   solution to the programming exercise. */

/* 
 * author: zhitong yan
 * date: 24/09/2016
 *
 * straight-line program language Argument Counter and Interpreter
 */
#include <stdio.h>
#include <stdlib.h>
#include "prog1.h"
#include "slp.h"

int maxargs(A_stm stm);
void interp(A_stm stm);

// tiger book hints for stm interpret
typedef struct table *Table_;							
struct table {string id; int value; Table_ tail; };			// 存放标识符和对应的值，包含中间值
Table_ Table(string id, int value, struct table *tail)
{
	Table_ t = malloc(sizeof(*t));
	t->id = id; t->value = value; t->tail = tail;
	return t;
}
Table_ interpStm(A_stm s, Table_ t);						// 通过 查询表t 解释语句stm 并产生一个更新后的表
Table_ update(Table_ t, string id, int value);				// 将表t的id标识符的值更新为value
int lookup(Table_ t, string key);							// 在t表中找到key的值

// tiger book hints for exp interpret
struct IntAndTable{int i; Table_ t; };						// i表示解释表达式得到的新值
// 用表t解释表达式之后，得到一个整数值i和新的表，将他们封装成 IntAndTable
struct IntAndTable interpExp(A_exp e, Table_ t);			

// helper functions
int max2(int a, int b);
int exps_ma(A_expList exps);								//计算表达式串中的参数最大数量

/*
 *Please don't modify the main() function
 */
int main()
{
	int args;

	printf("prog\n");
	args = maxargs(prog());
	printf("args: %d\n",args);
	interp(prog());

	printf("prog_prog\n");
	args = maxargs(prog_prog());
	printf("args: %d\n",args);
	interp(prog_prog());

	printf("right_prog\n");
	args = maxargs(right_prog());
	printf("args: %d\n",args);
	interp(right_prog());

	return 0;
}

int maxargs(A_stm stm)
{
	switch (stm->kind) {
		case A_compoundStm:
		{
			int left_max_args = maxargs(stm->u.compound.stm1);
			int right_max_args = maxargs(stm->u.compound.stm2);
			return max2(left_max_args, right_max_args);
		}
		case A_assignStm:
		{
			A_exp exp = stm->u.assign.exp;
			if (exp->kind == A_eseqExp) {
				return maxargs(exp->u.eseq.stm);
			} else {
				return 0;
			}
		}
		// most complecated part of this function
		case A_printStm:
		{
			A_expList exps = stm->u.print.exps;
			return exps_ma(exps);
		}
		default:
		{
			return 0;
		}
	}
}

void interp(A_stm stm)
{
	interpStm(stm, NULL);
}

/*----------------------------------------------------------------------------*/
int max2(int a, int b)
{
	return (a > b) ? a : b;
}

int exps_ma(A_expList exps)
{
	if (exps->kind == A_lastExpList) {
		// 处理有可能的 print语句的嵌套
		// 但是两个递归函数互相调用 不知道是否会发生互相无限制调用的情况？
		//printf("this exp kind is: %d\n", exps->u.last->kind);
		if (exps->u.last->kind == A_eseqExp) {
			A_exp exp = exps->u.last;
			return maxargs(exp->u.eseq.stm);
		}
		return 1;
	}
	if (exps->u.pair.head->kind == A_eseqExp) {
		A_exp exp = exps->u.pair.head;
		return maxargs(exp->u.eseq.stm) + exps_ma(exps->u.pair.tail);
	}
	return 1 + exps_ma(exps->u.pair.tail);
}

/*----------------------------------------------------------------------------*/
Table_ interpStm(A_stm s, Table_ t)
{
	struct IntAndTable *int_and_table = NULL;
	int_and_table = malloc(sizeof(*int_and_table));

	switch (s->kind) {
		case A_compoundStm:
		{
			t = interpStm(s->u.compound.stm1, t);
			t = interpStm(s->u.compound.stm2, t);
			return t;
		}
		case A_assignStm:
		{
			*int_and_table = interpExp(s->u.assign.exp, t);
			t = update(int_and_table->t, s->u.assign.id, int_and_table->i);
			return t;
		}
		case A_printStm:
		{
			
		}
		default:
		{
			return NULL;
		}
	}
	return NULL;
}

struct IntAndTable interpExp(A_exp e, Table_ t)
{
	
}

/*----------------------------------------------------------------------------*/
