#include "semant.h"
/*Lab4: Your implementation of lab4*/
/*
 * zhitong yan
 * 5140219099
 * 3/11/2016
 */

// ............................................HELP FUNCTIONS.......................................
// the constructor of "struct expty"
struct expty expTy(Tr_exp e, Ty_ty t) 
{
	struct expty et;
	et.exp = e;
	et.ty  = t;
	return et;
}

// get the ACTUAL name of a ty
// ACTUAL means not the user defined, rather, the build-in type name.
static Ty_ty actual_ty(Ty_ty ty)
{
	if (!ty) return ty;

	if (ty->kind == Ty_name) 
		actual_ty(ty->u.name.ty);
	else 
		return ty;
}

// return: HEAD->(symbol1, type1)->...->(symboln, typen)
static Ty_fieldList makeFieldTys(S_table t, A_fieldList fs) 
{
	A_fieldList f;
	Ty_fieldList tys = NULL, head;
	Ty_ty ty;
	Ty_field tmp;

	for (f = fs; f; f = f->tail) {
		ty = S_look(t, f->head->typ);
		if (!ty) {
			// test17
			fprintf(stderr, " undefined type %s\n", S_name(f->head->typ));
		} 
		else {
			tmp = Ty_Field(f->head->name, ty);
			if (tys) {
				tys->tail = Ty_FieldList(tmp, NULL);
				tys = tys->tail;
			} 
			else {
				tys = Ty_FieldList(tmp, NULL);
				head = tys;
			}
		}
	}
	return head;
}

// return: head->type1->...->typen
static Ty_tyList makeFormalTyList(S_table t, A_fieldList fl) 
{
	Ty_tyList final = NULL, head = final;
	A_fieldList l = fl;
	Ty_ty ty;

	for (; l; l = l->tail) {
		ty = S_look(t, l->head->typ);
		if(!ty) {
			fprintf(stderr, " undefined type %s\n", S_name(l->head->typ));
			ty = Ty_Int();
		}
		if (!final) {
			final = Ty_TyList(ty, NULL);
			head = final;
		} 
		else {
			final->tail = Ty_TyList(ty, NULL);
			final = final->tail;
		}
	}
	return head;
}

// does tt and ee match type
// either eq, comapre NULL etc.
static bool ty_match(Ty_ty tt, Ty_ty ee) {
	Ty_ty t = actual_ty(tt);
	Ty_ty e = actual_ty(ee);
	int tk = t->kind;
	int ek = e->kind;

	if((tk == Ty_record || tk == Ty_array) && t == e)
		return TRUE;
	if(tk == Ty_record && ek == Ty_nil)
		return TRUE;
	if(ek == Ty_record && tk == Ty_nil)
		return TRUE;
	if(tk != Ty_record && tk != Ty_array && tk == ek)
		return TRUE;
	return FALSE;
}

// check the function parameter TYPE & NUMBER match or not
static bool args_match(S_table v, S_table tt, A_expList ell, Ty_tyList fll, A_exp fun) 
{
	struct expty t;
	A_expList el = ell;
	Ty_tyList fl = fll;

	while (el && fl) {
		t = transExp(v, tt, el->head);
		if (!ty_match(t.ty, fl->head)) {
			// test34
			fprintf(stderr, "para type mismatch\n");
			return FALSE;
		}
		el = el->tail;
		fl = fl->tail;
	}
	if (el && !fl) {
		// test36
		fprintf(stderr, "too many params in function %s\n", S_name(fun->u.call.func));
		return FALSE;
	} 
	else if (!el && fl) {
		fprintf(stderr, "too few params in function %s\n", S_name(fun->u.call.func));
		return FALSE;
	} 
	else {
		return TRUE;
	}
}

// check type name/type & number match or not
static bool efields_match(S_table v, S_table t, Ty_ty tyy, A_exp e) 
{
	struct expty et;
	Ty_fieldList ty = tyy->u.record;
	A_efieldList fl = e->u.record.fields;
	while (ty && fl) {
		et = transExp(v, t, fl->head->exp);
		if (!(ty->head->name == fl->head->name) || !ty_match(ty->head->ty, et.ty)){
			fprintf(stderr, "unmatched name: in %s\n", S_name(e->u.record.typ));
			return FALSE;
		}
		ty = ty->tail;
		fl = fl->tail;
	}
	if (ty && !fl) {
		fprintf(stderr, "too few fields\n");
		return FALSE;
	} 
	else if (!ty && fl) {
		fprintf(stderr, "too many fields\n");
		return FALSE;
	}
	return TRUE;
}
// ............................................HELP FUNCTIONS.......................................


void SEM_transProg(A_exp exp)
{
	//fprintf(stderr, "@zhitong: below is the parse tree:\n");
	//FILE* out = stdout;
	//pr_exp(out, exp, 4);
	//fprintf(out, "\n");

	struct expty et;
	S_table t = E_base_tenv();
	S_table v = E_base_venv();
	et = transExp(v, t, exp);
	// check the return result (use Ty_ty->kind stand)
	//fprintf(stderr, "@zhitong: this expr return: %d\n", et.ty->kind); 
	return;
}

static struct expty transExp(S_table v, S_table t, A_exp e)
{
	switch(e->kind) {
		case A_varExp: {
			return transVar(v, t, e->u.var);
		}
		case A_intExp: {
			return expTy(NULL, Ty_Int());
		}
		case A_stringExp: {
			return expTy(NULL, Ty_String());
		}
		case A_recordExp: {
			Ty_ty recty = actual_ty(S_look(t, e->u.record.typ));
		    if (!recty) {
		    	// test33
				fprintf(stderr, "undefined type %s\n", S_name(e->u.record.typ));
			} 
			else {
				if (recty->kind != Ty_record){
					fprintf(stderr, "record type required %s\n", S_name(e->u.record.typ));
					return expTy(NULL, Ty_Record(NULL));
				}
				if (efields_match(v, t, recty, e)) {
					return expTy(NULL, recty);
				}
			}
			return expTy(NULL, Ty_Record(NULL));
		}
		case A_arrayExp: {
			Ty_ty arrayty = actual_ty(S_look(t, e->u.array.typ));
			struct expty arraySize, arrayType;

			if (!arrayty) {
				fprintf(stderr, "undefined array type %s\n", S_name(e->u.array.typ));
				return expTy(NULL, Ty_Array(NULL));
			}
			if (arrayty->kind != Ty_array) {
				fprintf(stderr, "not an array type %s\n", S_name(e->u.array.typ));
				return expTy(NULL, Ty_Array(NULL));
			}
		    arraySize = transExp(v, t, e->u.array.size);
			arrayType = transExp(v, t, e->u.array.init);
			if (arraySize.ty->kind != Ty_int) {
				fprintf(stderr, "array size should be int type");
			} 
			else if (!ty_match(arrayType.ty, arrayty->u.array)){
				// test32
				fprintf(stderr, "type mismatch\n");
			} 
			else {
				return expTy(NULL, arrayty);
			}
			return expTy(NULL, Ty_Array(NULL));
		}
		case A_opExp: {
			A_oper oper = e->u.op.oper;
			struct expty left, right;

			left  = transExp(v, t, e->u.op.left); 
			right = transExp(v, t, e->u.op.right);
			if (oper == A_eqOp || oper == A_neqOp || oper == A_leOp || oper == A_ltOp || oper == A_gtOp || oper == A_geOp) {
				if (left.ty->kind != A_nilExp && right.ty->kind != A_nilExp && left.ty->kind != right.ty->kind)
					// test13
					fprintf(stderr, "%s\n", "same type required");
			}
			if (0 <= oper && oper < 4) {
				if (left.ty->kind != Ty_int){
					// test43
					fprintf(stderr, "integer required\n");
				}
				if (right.ty->kind != Ty_int ) {
					// test21
					fprintf(stderr, "integer required\n");
				}
				return expTy(NULL, Ty_Int());
			} 
			else if (3 < oper && oper < 10) {
				if (oper == 4 || oper == 5) {
					if (left.ty->kind == Ty_record && right.ty->kind == Ty_nil) {
						return expTy(NULL, Ty_Int());
					}
					if (left.ty->kind == Ty_nil && right.ty->kind == Ty_record) {
						return expTy(NULL, Ty_Int());
					}
				}
				if(left.ty->kind != Ty_int && left.ty->kind != Ty_string){
					fprintf(stderr, "%s\n", "int or null required");
				}
				if (right.ty->kind != Ty_int && right.ty->kind !=Ty_string) {
					fprintf(stderr, "%s\n", "int or null required");
				}
				return expTy(NULL, Ty_Int());
			} 
			else {
				assert(0);	
			}
		}
		case A_callExp: {
			E_enventry callinfo = S_look(v, e->u.call.func);
			if (callinfo && callinfo->kind == E_funEntry){
				if (args_match(v, t, e->u.call.args, callinfo->u.fun.formals, e)) {
					if (callinfo->u.fun.result) {
						return expTy(NULL, actual_ty(callinfo->u.fun.result));
					} 
					else {
						return expTy(NULL, Ty_Void());
					}
				} 
			} 
			else {
				// test18
				fprintf(stderr, "undefined function %s\n", S_name(e->u.call.func));
			}
			return expTy(NULL, Ty_Void());
		}
		case A_whileExp: {
			struct expty whileTest = transExp(v, t, e->u.whilee.test);
			if (whileTest.ty->kind != Ty_int) {
				fprintf(stderr, "%s\n", "integer required");
			}
			struct expty body = transExp(v, t, e->u.whilee.body);
			if(body.ty->kind == Ty_int) {
				// test10
				fprintf(stderr, "while body must produce no value\n");
	      	}
			return expTy(NULL, Ty_Void());
		}
		case A_assignExp: {
			struct expty left = transVar(v, t, e->u.assign.var);
			struct expty right = transExp(v, t, e->u.assign.exp);
			if (!ty_match(left.ty, right.ty)) {
				// test23
				fprintf(stderr, "%s\n", "unmatched assign exp");
			}
			if(e->u.assign.var->kind == A_simpleVar) {
				// test11
				fprintf(stderr, "loop variable can't be assigned\n");
			}
			return expTy(NULL, Ty_Void());
		}
		case A_breakExp: {
			return expTy(NULL, Ty_Void());
		}
		case A_forExp: {
			struct expty lo = transExp(v, t, e->u.forr.lo);
			struct expty hi = transExp(v, t, e->u.forr.hi);
			struct expty body;

			if (lo.ty != Ty_Int() || hi.ty != Ty_Int()) {
				// test11
				fprintf(stderr, "for exp's range type is not integer\n");
			}

			S_beginScope(v);
			transDec(v, t, A_VarDec(e->pos, e->u.forr.var, S_Symbol("int"), e->u.forr.lo));
			body = transExp(v, t, e->u.forr.body);
			S_endScope(v);
			return expTy(NULL, Ty_Void());
		}
		case A_letExp: {
			S_beginScope(v);
			S_beginScope(t);

			A_decList decitem;
			for (decitem = e->u.let.decs; decitem; decitem = decitem->tail) {
				transDec(v, t, decitem->head);
			}

			struct expty etype = transExp(v, t, e->u.let.body);

			S_endScope(v);
			S_endScope(t);
			return etype;
		}
		case A_ifExp: {
			struct expty ifTest = transExp(v, t, e->u.iff.test);
			struct expty ifThen = transExp(v, t, e->u.iff.then);
			if (e->u.iff.elsee) {
				struct expty ifElse = transExp(v, t, e->u.iff.elsee);
				if((ifThen.ty->kind != ifElse.ty->kind) &&(ifThen.ty->kind != Ty_nil && ifElse.ty->kind != Ty_nil)) {
	  				// test9
	  				fprintf(stderr, "%s\n", "then exp and else exp type mismatch");
				}
				if (ifTest.ty->kind != Ty_int) {
					fprintf(stderr, "%s\n", "integer required");
				} 
				else if(!ty_match(ifThen.ty, ifElse.ty)) {
					// test15
					fprintf(stderr, "if-then exp's body must produce no value\n");
				} else { }
			}
			return expTy(NULL, ifThen.ty);
		}
		case A_seqExp: {
			A_expList list = e->u.seq;
			if (!list) {
				return expTy(NULL, Ty_Void());
			}
			while (list->tail) {
				transExp(v, t, list->head);
				list = list->tail;
			}
			return transExp(v, t, list->head);
		}
		case A_nilExp: {
			return expTy(NULL, Ty_Nil());
		}
		default: {
			assert(0);
		}
	}
}

static void transDec(S_table v, S_table t, A_dec d)
{
	switch(d->kind) {
		case A_varDec: {
			struct expty etype;
			Ty_ty resTy;

			etype = transExp(v, t, d->u.var.init);
			if (!d->u.var.typ) {
				if (etype.ty->kind == Ty_nil) {
					// test45
					fprintf(stderr, "init should not be nil without type specified\n");
					S_enter(v, d->u.var.var, E_VarEntry(Ty_Int()));
				}
				else {
					S_enter(v, d->u.var.var, E_VarEntry(etype.ty));
				}
			} 
			else {
				resTy = S_look(t, d->u.var.typ);
				if (!resTy) {
					fprintf(stderr, "undifined type %s", S_name(d->u.var.typ));
				} 
				else {
					if (!ty_match(resTy, etype.ty)) {
						// test28
						fprintf(stderr, "type mismatch\n");
						S_enter(v, d->u.var.var, E_VarEntry(resTy));
					} 
					else {
						S_enter(v, d->u.var.var, E_VarEntry(resTy));
					}
				}
				
			}
			break;
		}
		case A_typeDec: {
			int iscyl;
			Ty_ty resTy, namety;
			A_namety prev = NULL;
			A_nametyList nl, nList;

			// add type symbols into type-environment
			for (nl = d->u.type; nl; nl = nl->tail) {
				S_enter(t, nl->head->name, Ty_Name(nl->head->name,NULL));
			}

			// check if two types have the same name
			nList = d->u.type;
			while (nList) {
	  			if(prev && !strcmp(S_name(nList->head->name),S_name(prev->name))) {
	  				// test38
					fprintf(stderr, "two types have the same name\n");
	  			}
	  			S_enter(t, nList->head->name, Ty_Name(nList->head->ty->u.name, NULL));
	  			prev = nList->head;
				nList = nList->tail;
			}

			// check if has some type decline cycle
			iscyl = TRUE;
			for (nl = d->u.type; nl; nl = nl->tail) {
				resTy = transTy(t, nl->head->ty);
				if (iscyl) {
					if (resTy->kind != Ty_name) {
						iscyl = FALSE;
					}
				}
				namety = S_look(t, nl->head->name);
				namety->u.name.ty = resTy;
			}
			if (iscyl) 
				// test16
				fprintf(stderr, "illegal type cycle\n");
			break;
		}
		case A_functionDec: {
			Ty_ty resTy;
			Ty_tyList formalTys, s;
			A_fundec f, pre = NULL;
			A_fundecList fcl;
			A_fieldList l;
			E_enventry fun;
			struct expty etype;

			for (fcl = d->u.function; fcl; fcl = fcl->tail) {
				// check the return type
				if (fcl->head->result) {
					resTy = S_look(t, fcl->head->result);
					if (!resTy) {
						fprintf(stderr, "undefined return type\n");
						resTy = Ty_Void();
					} 
				} 
				else {
					resTy = Ty_Void();
				}
				// check the function name
				formalTys = makeFormalTyList(t, fcl->head->params);
				if(pre && !strcmp(S_name(fcl->head->name), S_name(pre->name))) {
					// test39
					fprintf(stderr, "two functions have the same name\n");
	  			}
				S_enter(v, fcl->head->name, E_FunEntry(formalTys, resTy));
				pre = fcl->head;
			}
			for (fcl = d->u.function; fcl; fcl = fcl->tail) {
				f = fcl->head;
				// function qiantao...
				S_beginScope(v);
				formalTys = makeFormalTyList(t, f->params);
				for (l = f->params, s = formalTys; l && s; l = l->tail, s = s->tail) {
					S_enter(v, l->head->name, E_VarEntry(s->head));
				}
				etype = transExp(v, t, f->body);
				fun = S_look(v, f->name);
				if (!ty_match(fun->u.fun.result, etype.ty)) {
					// test21
					fprintf(stderr, "procedure returns value\n");
				}
				S_endScope(v);
			}
			break;
		}
		default: {
			assert(0);
		}
	}
}

static Ty_ty transTy(S_table tb, A_ty ty) 
{
	switch (ty->kind) {
		case A_nameTy: {
			Ty_ty type = S_look(tb, ty->u.name);
			if (!type) {
				fprintf(stderr, "undefined type %s\n", S_name(ty->u.name));
				return Ty_Int();
			}
			return type;
		}
		case A_recordTy: {
			Ty_fieldList fieldTys = makeFieldTys(tb, ty->u.record);
			return Ty_Record(fieldTys);
		}
		case A_arrayTy: {
			Ty_ty type = S_look(tb, ty->u.array);
			if (!type) 
				fprintf(stderr, "undefined type %s\n", S_name(ty->u.array));
			return Ty_Array(type);
		}
		default: {
			assert(0);
		}
	}
}

static struct expty transVar(S_table venv, S_table tenv, A_var v)
{
	switch (v->kind) {
		case A_simpleVar: {
			E_enventry eentry = S_look(venv, v->u.simple);
			if (eentry && eentry->kind == E_varEntry) {
				return expTy(NULL, actual_ty(eentry->u.var.ty));
			} 
			else {
				// test19
				fprintf(stderr, "undefined variable %s", S_name(v->u.simple));
				return expTy(NULL, Ty_Int());
			}
			break;
		}
		case A_fieldVar: {
			struct expty etype = transVar(venv, tenv, v->u.field.var);
			Ty_fieldList fl;

			if (etype.ty->kind != Ty_record) {
				// test25
				fprintf(stderr, "%s\n", "not a record type");
				return expTy(NULL, Ty_Record(NULL));
			} 
			else {
				fl = etype.ty->u.record;
				for (fl = etype.ty->u.record; fl; fl = fl->tail) {
					if (fl->head->name == v->u.field.sym) {
						return expTy(NULL, actual_ty(fl->head->ty));
					}
				}
				// test22
				fprintf(stderr, "field %s doesn't exist\n", S_name(v->u.field.sym));
			}
			return expTy(NULL, Ty_Record(NULL));
			break;
		}
		case A_subscriptVar: {
			struct expty etype = transVar(venv, tenv, v->u.subscript.var);
			struct expty etype2;

			if (etype.ty->kind != Ty_array) {
				// test24
				fprintf(stderr, "array type required\n");
			} 
			else {
				etype2 = transExp(venv, tenv, v->u.subscript.exp);
				if (etype2.ty->kind != Ty_int) {
					fprintf(stderr, "%s\n", "int type required");
				} 
				else {
					return expTy(NULL, actual_ty(etype.ty->u.array));
				}
			}
			return expTy(NULL, Ty_Array(NULL));
		}
		default: {
			assert(0);
		}
	}
}






















