#include <stdio.h>
#include "util.h"
#include "errormsg.h"
#include "symbol.h"
#include "absyn.h"
#include "types.h"
#include "env.h"

typedef void* Tr_exp;

struct expty {Tr_exp exp; Ty_ty ty; };
struct expty expTy(Tr_exp e, Ty_ty t);

void SEM_transProg(A_exp exp);

static struct expty transVar(S_table venv, S_table tenv, A_var v);
static struct expty transExp(S_table venv, S_table tenv, A_exp e);
static void         transDec(S_table venv, S_table tenv, A_dec d);
static Ty_ty        transTy (              S_table tenv, A_ty  t);

static Ty_tyList    makeFormalTyList(S_table t, A_fieldList p);
static Ty_ty        actual_ty(Ty_ty ty);
static bool 		args_match(S_table v, S_table t, A_expList el, Ty_tyList fl, A_exp fun); 
static bool 		ty_match(Ty_ty t1, Ty_ty t2);
static bool 		efields_match(S_table v, S_table t, Ty_ty ty, A_exp e);
static Ty_fieldList makeFieldTys(S_table t, A_fieldList fs);
