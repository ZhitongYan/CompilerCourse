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

/**
 * 键值对 链表结构
 * 存放 标识符和值的信息（包括过程中的值 和当前值，当前值在最前面）
 */
typedef struct table *Table_;							
struct table {string id; int value; Table_ tail; };	
Table_ Table(string id, int value, struct table *tail)
{
	Table_ t = malloc(sizeof(*t));
	t->id = id; t->value = value; t->tail = tail;
	return t;
}

/**
 * 整型值和参数表 结构体
 * 解释表达式的时候需要记录最终的值
 */
typedef struct intAndTable *IntAndTable_;
struct intAndTable{int i; Table_ t; };		
IntAndTable_ IntAndTable(int i, Table_ t)
{
	IntAndTable_ iat = malloc(sizeof(*iat));
	iat->i = i; iat->t = t;
	return iat;
}

/**
 * 函数声明
 */
int maxargs(A_stm stm);
int max2(int a, int b);
int exps_ma(A_expList exps);
int exp_ma(A_exp e);

void interp(A_stm stm);
Table_ interpStm(A_stm s, Table_ t);			
IntAndTable_ interpExp(A_exp e, Table_ t);			
IntAndTable_ interpExpList(A_expList expList, Table_ t);
Table_ update(Table_ t, string id, int value);	
int lookup(Table_ t, string key);			

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

/**
 * 给定语句，返回语句中的 打印参数个数的 最大值
 */
int maxargs(A_stm stm)
{
	switch (stm->kind) {
		// 租合语句 递归
		case A_compoundStm:
		{
			int left_max_args = maxargs(stm->u.compound.stm1);
			int right_max_args = maxargs(stm->u.compound.stm2);
			return max2(left_max_args, right_max_args);
		}
		// 赋值语句 表达式类型为eseqExp 才有可能含有打印语句，从而对最终结果有影响
		case A_assignStm:
		{
			A_exp exp = stm->u.assign.exp;
			if (exp->kind == A_eseqExp) {
				return maxargs(exp->u.eseq.stm);
			} else {
				return 0;
			}
		}
		// 打印语句 具体计算参数个数
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
	interpStm(stm, NULL);		// 初始为空表
}

/*----------------------------------------------------------------------------*/
int max2(int a, int b)
{
	return (a > b) ? a : b;
}

/**
 * 计算表达式链表的 参数总数
 */
int exps_ma(A_expList exps)
{
	switch (exps->kind) {
		case A_lastExpList:
		{
			A_exp e = exps->u.last;
			return exp_ma(e);
		}
		case A_pairExpList:
		{
			A_exp head = exps->u.pair.head;
			A_expList tail = exps->u.pair.tail;
			return (exp_ma(head) + exps_ma(tail));
		}
		default:
		{
			return 0;
		}
	}
}

/**
 * 计算表达式的 参数总数
 * 真正数字的累加在这里不断递归完成
 * 目的只有一个：防止print语句内部嵌套print语句
 */
int exp_ma(A_exp e)
{
	if ((e->kind == A_idExp) || (e->kind == A_numExp) || (e->kind == A_opExp)) {
		return 1;
	} else if (e->kind == A_eseqExp) {
		return (maxargs(e->u.eseq.stm) + exp_ma(e->u.eseq.exp));
	} else {
		return 0;
	}
}

/*----------------------------------------------------------------------------*/
/**
 * 使用符号表t解释语句stm，并生成一个新表
 */
Table_ interpStm(A_stm s, Table_ t)
{
	IntAndTable_ iat;	//解释表达式的时候会产生一个整型值

	switch (s->kind) {
		case A_compoundStm:
		{
			t = interpStm(s->u.compound.stm1, t);
			t = interpStm(s->u.compound.stm2, t);
			return t;
		}
		case A_assignStm:
		{
			iat = interpExp(s->u.assign.exp, t);
			t = update(iat->t, s->u.assign.id, iat->i);
			return t;
		}
		case A_printStm:
		{
			iat = interpExpList(s->u.print.exps, t);
      		return iat->t;
		}
		default:
		{
			return NULL;
		}
	}
	return t;
}

/**
 *
 */
IntAndTable_ interpExp(A_exp e, Table_ t)
{
	switch (e->kind) {
		case A_idExp:
		{
			return IntAndTable(lookup(t, e->u.id), t);

		}
		case A_numExp:
		{
			return IntAndTable(e->u.num, t);
		}
		case A_opExp:
		{
			int lval = 0, rval = 0;
			IntAndTable_ tmp;

			tmp = interpExp(e->u.op.left, t);
			lval = tmp->i;
			tmp = interpExp(e->u.op.right, tmp->t);
			rval = tmp->i;

			int ival = 0;
			switch (e->u.op.oper) {
				case A_plus:
				{
					ival = lval + rval;
					break;
				}
				case A_minus:
				{
					ival = lval - rval;
					break;
				}
				case A_times:
				{
					ival = lval * rval;
					break;
				}
				case A_div:
				{
					if (rval != 0) {
						ival = lval / rval;
					} else {
						return NULL;
					}
					break;
				}
				default:
				{
					return NULL;
				}
			}
			return IntAndTable(ival, tmp->t);
		}
		case A_eseqExp:
		{
			t = interpStm(e->u.eseq.stm, t);
			return interpExp(e->u.eseq.exp, t);
		}
		default:
		{
			return NULL;
		}
	}
}

/**
 *
 */
IntAndTable_ interpExpList(A_expList expList, Table_ t)
{
	IntAndTable_ iat;
	switch (expList->kind) {
		case A_pairExpList:
		{
			iat = interpExp(expList->u.pair.head, t);
      		printf("%d ", iat->i);
      		return interpExpList(expList->u.pair.tail, iat->t);
		}
		case A_lastExpList:
		{
			iat = interpExp(expList->u.last, t);
      		printf("%d\n", iat->i);
      		return iat;
		}
		default:
		{
			return NULL;
		}
	}
}

/**
 *
 */
Table_ update(Table_ t, string id, int value)
{
	return Table(id, value, t);
}

int lookup(Table_ t, string key)
{
	Table_ temp = t;
  	while (temp != NULL) {
    	if (temp->id == key) {
      		return t->value;
    	}
    	temp = temp->tail;
  	}
	return -1;
}

/*----------------------------------------------------------------------------*/
