%{
#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "symbol.h"
#include "errormsg.h"
#include "absyn.h"

int yylex(void); /* function prototype */

A_exp absyn_root;

void yyerror(char *s)
{
	EM_error(EM_tokPos, "%s", s);
	exit(1);
}
%}


%union {
    int pos;
    int ival;
    string sval;
    A_var var;
    A_exp exp;
    A_expList	expList;
    A_dec dec; /* decline */
    A_decList	decList;
    A_ty ty; /* type */
    A_namety namety;
    A_nametyList nametyList;
    A_fundec funcdec;
    A_fundecList funcdecList;
    A_field	field;
    A_fieldList fieldList;
    A_efield efield;
    A_efieldList efieldList;
}

%token <sval> ID STRING
%token <ival> INT
%token
COMMA COLON SEMICOLON LPAREN RPAREN LBRACK RBRACK
LBRACE RBRACE DOT
PLUS MINUS TIMES DIVIDE EQ NEQ LT LE GT GE
AND OR ASSIGN
ARRAY IF THEN ELSE WHILE FOR TO DO LET IN END OF
BREAK NIL
FUNCTION VAR TYPE

%type <exp> exp program
%type <exp> varExp nilExp intExp stringExp opExp callExp seqExp assignExp recordExp arrayExp ifExp whileExp forExp breakExp letExp
%type <var> lvalue
%type <efield> efield
%type <efieldList> efieldList
%type <expList> seqList argList
%type <dec> dec
%type <decList> decList
%type <namety> namety
%type <nametyList> nametyList
%type <funcdecList> funcDecList
%type <funcdec> funcDec_
%type <field> field
%type <fieldList> fieldList 
%type <ty> ty

%nonassoc LOWER
%nonassoc OF
%nonassoc IF THEN WHILE DO FOR TO
%left ELSE
%nonassoc ASSIGN
%left OR AND
%nonassoc EQ NEQ GT LT GE LE
%left PLUS MINUS
%left TIMES DIVIDE
%nonassoc TYPE
%nonassoc FUNCTION

%start program
%%


/* top -> down */
program	:	exp		{absyn_root=$1;}

exp	:   	opExp     {$$=$1;}
 	|   	varExp    {$$=$1;}
	|   	nilExp    {$$=$1;}
	|   	intExp    {$$=$1;}
	|   	stringExp {$$=$1;}
	|   	callExp   {$$=$1;}
	|   	recordExp {$$=$1;}
	|   	seqExp    {$$=$1;}
	|   	assignExp {$$=$1;}
	|   	ifExp     {$$=$1;}
	|   	whileExp  {$$=$1;}
	|   	forExp    {$$=$1;}
	|   	breakExp  {$$=$1;}
	|   	arrayExp  {$$=$1;}	
	|   	letExp    {$$=$1;}

/* + - * / = != >= > < <= -Exp  and or */
opExp 	:   	exp PLUS exp 	{$$=A_OpExp(EM_tokPos,A_plusOp,$1,$3);}
	|   	exp MINUS exp 	{$$=A_OpExp(EM_tokPos,A_minusOp,$1,$3);}
	|   	exp TIMES exp 	{$$=A_OpExp(EM_tokPos,A_timesOp,$1,$3);}
	|   	exp DIVIDE exp 	{$$=A_OpExp(EM_tokPos,A_divideOp,$1,$3);}
	|   	exp EQ exp 	{$$=A_OpExp(EM_tokPos,A_eqOp,$1,$3);}
	|   	exp NEQ exp 	{$$=A_OpExp(EM_tokPos,A_neqOp,$1,$3);}
	|   	exp LT exp 	{$$=A_OpExp(EM_tokPos,A_ltOp,$1,$3);}
	|   	exp LE exp 	{$$=A_OpExp(EM_tokPos,A_leOp,$1,$3);}
	|   	exp GT exp 	{$$=A_OpExp(EM_tokPos,A_gtOp,$1,$3);}
	|   	exp GE exp 	{$$=A_OpExp(EM_tokPos,A_geOp,$1,$3);}
	|   	MINUS exp 	{$$=A_OpExp(EM_tokPos,A_minusOp,A_IntExp(EM_tokPos,0),$2);}
	|	exp AND exp	{$$ = A_IfExp(EM_tokPos, $1, $3, A_IntExp(EM_tokPos, 0)); }
	|	exp OR exp	{$$ = A_IfExp(EM_tokPos, $1 , A_IntExp(EM_tokPos, 1), $3); }

/* lvalue */
varExp 	:  	lvalue  {$$=A_VarExp(EM_tokPos,$1);}
lvalue 	:  	ID 				{$$=A_SimpleVar(EM_tokPos,S_Symbol($1));}
	|   	lvalue DOT ID 			{$$=A_FieldVar(EM_tokPos,$1,S_Symbol($3));}
	|   	lvalue LBRACK exp RBRACK 	{$$=A_SubscriptVar(EM_tokPos,$1,$3);}
	|   	ID LBRACK exp RBRACK  		{$$=A_SubscriptVar(EM_tokPos,A_SimpleVar(EM_tokPos,S_Symbol($1)),$3);}

nilExp 	:  	NIL		{$$=A_NilExp(EM_tokPos);}
	/*|	LPAREN RPAREN	{$$=A_NilExp(EM_tokPos);} */

intExp 	:  	INT     {$$=A_IntExp(EM_tokPos,$1);}

stringExp 	: 	STRING	{$$=A_StringExp(EM_tokPos,$1);}

/* call function */
callExp : 	ID LPAREN argList RPAREN	{$$=A_CallExp(EM_tokPos,S_Symbol($1),$3);}
	|	ID LPAREN RPAREN		{$$=A_CallExp(EM_tokPos,S_Symbol($1),NULL);}
argList : 	exp  			{$$=A_ExpList($1,NULL);}
	|	exp COMMA argList	{$$ = A_ExpList($1, $3); }

/* equal record expression */
recordExp 	: 	ID LBRACE efieldList RBRACE 	{$$=A_RecordExp(EM_tokPos,S_Symbol($1),$3);}
		|	ID LBRACE RBRACE		{$$=A_RecordExp(EM_tokPos,S_Symbol($1),NULL);}
efieldList 	:  	efield			{$$=A_EfieldList($1,NULL);}
		|	efield COMMA efieldList	{$$ = A_EfieldList($1, $3); }
efield 		:  	ID EQ exp {$$=A_Efield(S_Symbol($1),$3);}

/* expression sequence */
seqExp 	:  	LPAREN seqList RPAREN {$$=A_SeqExp(EM_tokPos,$2);}
seqList	: 				{$$=NULL;}
	|   	exp  			{$$=A_ExpList($1,NULL);}
	|   	exp SEMICOLON seqList 	{$$=A_ExpList($1,$3);}

/* assign expression */
assignExp 	: 	lvalue ASSIGN exp 	{$$=A_AssignExp(EM_tokPos,$1,$3);}

/* IF */
ifExp 	: 	IF exp THEN exp ELSE exp	{$$=A_IfExp(EM_tokPos,$2,$4,$6);}
	|   	IF exp THEN exp 		{$$=A_IfExp(EM_tokPos,$2,$4,A_NilExp(EM_tokPos));}

/* while */
whileExp:  	WHILE exp DO exp  	{$$=A_WhileExp(EM_tokPos,$2,$4);}

/* for */
forExp 	:  	FOR ID ASSIGN exp TO exp DO exp {$$=A_ForExp(EM_tokPos,S_Symbol($2),$4,$6,$8);}

/* break */
breakExp:  	BREAK 	{$$=A_BreakExp(EM_tokPos);}

/* array */
arrayExp: 	ID LBRACK exp RBRACK OF exp {$$=A_ArrayExp(EM_tokPos,S_Symbol($1),$3,$6);}

/* let */
letExp 	:  	LET decList IN seqList END  	{$$=A_LetExp(EM_tokPos,$2,A_SeqExp(EM_tokPos, $4));}
	|	LET decList IN END		{$$=A_LetExp(EM_tokPos,$2,NULL);}
decList : 	dec %prec LOWER {$$=A_DecList($1,NULL);}
	|   	dec decList 	{$$=A_DecList($1,$2);}
dec 	:   	nametyList 			{$$=A_TypeDec(EM_tokPos,$1);}
	|   	VAR ID ASSIGN exp  		{$$=A_VarDec(EM_tokPos,S_Symbol($2),NULL,$4);}
	|	VAR ID COLON ID ASSIGN exp	{$$=A_VarDec(EM_tokPos,S_Symbol($2),S_Symbol($4),$6);}
	|   	funcDecList 			{$$=A_FunctionDec(EM_tokPos,$1);}

nametyList	:	namety %prec LOWER 	{$$=A_NametyList($1,NULL);}
		|   	namety nametyList 	{$$=A_NametyList($1,$2);}
namety	:	TYPE ID EQ ty	{ $$ = A_Namety(S_Symbol($2) , $4 ) ;}
ty	:	LBRACE fieldList RBRACE	{$$ = A_RecordTy(EM_tokPos, $2); }
	|	LBRACE RBRACE		{$$ = A_RecordTy(EM_tokPos, NULL); }
	|	ARRAY OF ID		{/*printf("id name is: %s\n", S_name(S_Symbol($3)));*/ $$ = A_ArrayTy(EM_tokPos, S_Symbol($3)); }
/*
{
	A_ty p = checked_malloc(sizeof(*p));
	p->kind = A_arrayTy;
	p->pos = EM_tokPos;
	p->u.array = S_Symbol($3);
	//printf("I goch ya!\n");
	$$ = p;
}
*/
	|	ID  			{$$ = A_NameTy(EM_tokPos, S_Symbol($1)); }
fieldList 	:  	field			{ $$ = A_FieldList($1 , NULL) ; }
		|	field COMMA fieldList	{ $$ = A_FieldList($1 , $3) ; }
field 	: 	ID COLON ID 	{$$=A_Field(EM_tokPos,S_Symbol($1),S_Symbol($3));}

funcDecList 	: 	funcDec_ %prec LOWER 	{$$=A_FundecList($1,NULL);}
		|   	funcDec_ funcDecList  	{$$=A_FundecList($1,$2);}
funcDec_ : FUNCTION ID LPAREN fieldList RPAREN COLON ID EQ exp { $$ = A_Fundec(EM_tokPos, S_Symbol($2) , $4 , S_Symbol($7) , $9 ) ;     }
         	| FUNCTION ID LPAREN RPAREN COLON ID EQ exp         { $$ = A_Fundec(EM_tokPos, S_Symbol($2) , NULL , S_Symbol($6) , $8 ) ;   }
         	| FUNCTION ID LPAREN fieldList RPAREN EQ exp         { $$ = A_Fundec(EM_tokPos, S_Symbol($2) , $4 , NULL , $7 ) ;   }
         	| FUNCTION ID LPAREN RPAREN EQ exp                  { $$ = A_Fundec(EM_tokPos, S_Symbol($2) , NULL , NULL , $6 ) ; }
