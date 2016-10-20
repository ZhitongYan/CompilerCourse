%{
#include <string.h>
#include "util.h"
#include "tokens.h"
#include "errormsg.h"

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
#define BUFSIZE   256
int comment_nesting_depth = 0;
char *buf = NULL;
int buf_capacity = 0;
int str_start_pos = 0;

void init_buf(void)
{
  buf = checked_malloc(BUFSIZE);
  buf[0] = 0;
  buf_capacity = BUFSIZE;
}

void add_char(char ch)
{
    size_t len = strlen(buf) + 1;
    if (len == buf_capacity) {
        // allocate a new bigger space
        char *str;
        buf_capacity *= 2;
        str = checked_malloc(buf_capacity);
        memcpy(str, buf, len);
        free(buf);
        buf = str;
    }
    buf[len - 1] = ch;
    buf[len] = 0;
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
%x STRING_MODE



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
  yylval.sval = yytext;
  return ID;
}

  /* positive integers */
{digits} {
  adjust();
  yylval.ival = atoi(yytext);
  return INT;
}

  /* String literals. */
\"  {
    adjust();
    init_buf();
    str_start_pos = charPos - 1;
    BEGIN(STRING_MODE);
}
<STRING_MODE>{
    \" {
      // the end of a string
      adjust();
      if(buf[0] == '\0'){
        yylval.sval = "(null)";
      } else {
        yylval.sval = buf;
      }
      EM_tokPos = str_start_pos;
      BEGIN(INITIAL);
      return STRING;
    }

    \n {
      adjust();
      EM_error(EM_tokPos, "string without an \" !");
      yyterminate();
    }

    \\n {adjust(); add_char('\n');}
    \\t {adjust(); add_char('\t');}
    \\r {adjust(); add_char('\r');}
    \\b {adjust(); add_char('\b');}
    \\f {adjust(); add_char('\f');}

    <<EOF>> {
      EM_error(EM_tokPos, "Encounter EOF.");
      yyterminate();
    }

    . {
      // add the string to buf
      adjust();
      char *yptr = yytext;
      while (*yptr) {
        add_char(*yptr++);
      }
    }
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
    EM_error(EM_tokPos, "Encounter EOF.");
    yyterminate();
  }

  \n  {
    adjust();
    EM_newline();
    continue;
  }

  . {
    adjust();
  }
}

  /* error handling */
.	 {adjust(); EM_error(EM_tokPos, "illegal token");}

