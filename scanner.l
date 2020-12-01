%{
#include <cctype>
#include <cstdlib>
#include <cstring>
#include "parser.h"
#ifdef _DEBUG
# define DBOUT(v) (printf(" '%s':%d ", yytext, (v)), (v))
#else
# define DBOUT(v) (v)
#endif
#pragma warning(push)
#pragma warning(disable: 4127 4267 4996)

static int bracket_depth;
%}

ws      [\r\t\v ]+
rem     \/\/.*
id      [_A-Za-z][_0-9A-Za-z]*
hex     $[0-9A-Fa-f]+
num     [0-9]+
str     \"[^"]\"

%%

{ws}     |
{rem}    ;
{hex}    { yylval.value = strtoul(yytext + 1, nullptr, 16); return DBOUT(NUMBER); }
{num}    { yylval.value = strtoul(yytext, nullptr, 10); return DBOUT(NUMBER); }
{str}    { yylval.id = _strdup(yytext); return DBOUT(STRING); }
continue { return DBOUT(CONTINUE); }
function { return DBOUT(FUNCTION); }
import   { return DBOUT(IMPORT); }
break    { return DBOUT(BREAK); }
while    { return DBOUT(WHILE); }
else     { return DBOUT(ELSE); }
from     { return DBOUT(FROM); }
for      { return DBOUT(FOR); }
as       { return DBOUT(AS); }
do       { return DBOUT(DO); }
if       { return DBOUT(IF); }
{id}     { yylval.id = _strdup(yytext); return DBOUT(ID); }
">>>"    return DBOUT(ASR);
"<<"     return DBOUT(SHL);
">>"     return DBOUT(LSR);
"=="     return DBOUT(EQ);
"!="     return DBOUT(NE);
"<="     return DBOUT(LE);
">="     return DBOUT(GE);
"&&"     return DBOUT(AND);
"||"     return DBOUT(OR);
\n       { ++yylineno; if(bracket_depth == 0) return DBOUT(SEP); }
";"      return DBOUT(SEP);
"{"      |
"("      |
"["      { ++bracket_depth; return DBOUT(yytext[0]); }
"}"      { if(bracket_depth > 0) --bracket_depth; return DBOUT(yytext[0]); }
")"      |
"]"      { --bracket_depth; return DBOUT(yytext[0]); }
.        return DBOUT(yytext[0]);

%%

#pragma warning(pop)

int yywrap() {
	return 1;
}

void undo_bracket() {
	--bracket_depth;
}

#pragma warning(disable: 4505)
