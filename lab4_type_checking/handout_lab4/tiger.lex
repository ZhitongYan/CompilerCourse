%{
#include <string.h>
#include "util.h"
//#include "tokens.h"
#include"symbol.h"
#include "errormsg.h"
#include "absyn.h"
#include "y.tab.h"

/*
 * editor: zhitong yan
 * id: 5140219099
 * date: 11/10/2016
 */

int charPos=1;

int yywrap(void)
{
 charPos=1;
 return 1;
}

void adjust(void)
{
 EM_tokPos=charPos;
 charPos+=yyleng;
}
/*
* Please don't modify the lines above.
* You can add C declarations of your own below.
*/
/********************* PART 1 ************************/
/* c declarations: */
int comment_nesting_depth = 0;

char* strOp(char* str)
{
	size_t str_size = strlen(str) + 1;

	char* buf1 = checked_malloc(str_size-2);
	str++;
	strncpy(buf1, str, str_size-3);
	if (buf1[0] == '\0')
		return "";
	if (buf1[0] == '\\')
		return "\n";

	char* buf2 = checked_malloc(strlen(buf1)+1);
	strcpy(buf2, buf1);
	size_t buf2_size = strlen(buf2);
	char* pos = strstr(buf2, "\\n");

	while (pos) {
		char *buf3 = checked_malloc(buf2_size);
		size_t size = 0;
		while (pos != buf2) {
			size += 1;
			buf2++;
		}
		if (pos != buf2)
			strncpy(buf3, buf2, size);
		*pos = '\n';
		if (buf2_size > size+2) {
			char* sp1 = pos + 2;
			char* sp2 = pos + 1;
			strcpy(sp2, sp1);
		}
		buf2_size = strlen(buf3);
		pos = strstr(buf3, "\\n");
		strcpy(buf2, buf3);
	}
	char* buf4 = checked_malloc(buf2_size);
	strcpy(buf4, buf2);
	return buf4;
}
%}



  /* You can add lex definitions here. */
  /********************* PART 2 ************************/
  /* lex definitions: */
spaces          [ |\t|\r]
digits          [0-9]+
letters         [a-z|A-Z]+
identifiers     {letters}[a-z|A-Z|0-9|_]*

%x COMMENT_MODE



%%
  /* 
  * Below are some examples, which you can wipe out
  * and write reguler expressions and actions of your own.
  */ 
  /********************* PART 3 ************************/
  /* Regular Expressions & actions */
  /* Reserved words */
while     {adjust(); return WHILE;}
for       {adjust(); return FOR;}
to        {adjust(); return TO;}
break     {adjust(); return BREAK;}
let       {adjust(); return LET;}
in        {adjust(); return IN;}
end       {adjust(); return END;}
function  {adjust(); return FUNCTION;}
var       {adjust(); return VAR;}
type      {adjust(); return TYPE;}
array     {adjust(); return ARRAY;}
if        {adjust(); return IF;}
then      {adjust(); return THEN;}
else      {adjust(); return ELSE;}
do        {adjust(); return DO;}
of        {adjust(); return OF;}
nil       {adjust(); return NIL;}

  /* Punctuations */
","   {adjust(); return COMMA;}
":"   {adjust(); return COLON;}
";"   {adjust(); return SEMICOLON;}
"("   {adjust(); return LPAREN;}
")"   {adjust(); return RPAREN;}
"["   {adjust(); return LBRACK;}
"]"   {adjust(); return RBRACK;}
"{"   {adjust(); return LBRACE;}
"}"   {adjust(); return RBRACE;}
"."   {adjust(); return DOT;}
"+"   {adjust(); return PLUS;}
"-"   {adjust(); return MINUS;}
"*"   {adjust(); return TIMES;}
"/"   {adjust(); return DIVIDE;}
"="   {adjust(); return EQ;}
"<>"  {adjust(); return NEQ;}
"<"   {adjust(); return LT;}
"<="  {adjust(); return LE;}
">"   {adjust(); return GT;}
">="  {adjust(); return GE;}
"&"   {adjust(); return AND;}
"|"   {adjust(); return OR;}
":="  {adjust(); return ASSIGN;}

  /* skip spaces */
{spaces}    {adjust(); continue;}

  /* count a new line */
[\n]        {adjust(); EM_newline(); continue;}

  /* identifiers */
{identifiers} {
  adjust();
  yylval.sval = String(yytext);
  return ID;
}

  /* positive integers */
{digits} {
  adjust();
  yylval.ival = atoi(yytext);
  return INT;
}

  /* String literals. */
[\"][^\"]*[\"]  {
    adjust();
    yylval.sval = String(strOp(yytext));
    return STRING;
}

  /* Comments. */
"/*" {
     adjust();
     comment_nesting_depth++;
     BEGIN(COMMENT_MODE);
}
<COMMENT_MODE>{
  "/*" {
    adjust();
    comment_nesting_depth++;
    continue;
  }

  "*/" {
    adjust();
    comment_nesting_depth--;
    if (comment_nesting_depth == 0) {
      BEGIN(INITIAL);
    }
  }

  <<EOF>> {
    //EM_error(EM_tokPos, "Encounter EOF.");
    yyterminate();
  }

  \n  {
    adjust();
    //EM_newline();
    continue;
  }

  . {
    adjust();
  }
}

  /* error handling */
.	 {adjust(); EM_error(EM_tokPos, "illegal token");}




