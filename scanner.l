%{
#include <cassert> // assert
#include <cstdio> // printf
#include <cstdlib> // strtoul
#include <cstring> // _strdup
#include "parser.h"
#include "scanner.h"
#ifdef _DEBUG
# define RETURN_(v) return (printf(" '%s':%d ", yytext, (v)), (v))
# define RETURN_ASSIGNMENT(v) return (printf(" '%s':%d ", yytext, (v)), yylval.value = (v), ASSIGNMENT)
#else
# define RETURN_(v) return (v)
# define RETURN_ASSIGNMENT(v) return (yylval.value = (v), ASSIGNMENT)
#endif
#pragma warning(push)
#pragma warning(disable: 4996) // 'read': The POSIX name for this item is deprecated. Instead, use the ISO C and C++ conformant name: _read. See online help for details.
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
{hex}    { yylval.value = strtoul(yytext + 1, nullptr, 16); RETURN_(NUMBER); }
{num}    { yylval.value = strtoul(yytext, nullptr, 10); RETURN_(NUMBER); }
{str}    { yylval.id = _strdup(yytext); RETURN_(STRING); }
continue { RETURN_(CONTINUE); }
function { RETURN_(FUNCTION); }
default  { RETURN_(DEFAULT); }
import   { RETURN_(IMPORT); }
return   { RETURN_(RETURN); }
switch   { RETURN_(SWITCH); }
await    { RETURN_(AWAIT); }
break    { RETURN_(BREAK); }
while    { RETURN_(WHILE); }
yield    { RETURN_(YIELD); }
case     { RETURN_(CASE); }
else     { RETURN_(ELSE); }
from     { RETURN_(FROM); }
for      { RETURN_(FOR); }
var      { RETURN_(VAR); }
as       { RETURN_(AS); }
do       { RETURN_(DO); }
if       { RETURN_(IF); }
{id}     { yylval.id = _strdup(yytext); RETURN_(ID); }
">>>"    RETURN_(ASR);
"&="     RETURN_ASSIGNMENT(AA);
">>>="   RETURN_ASSIGNMENT(ARA);
"/="     RETURN_ASSIGNMENT(DA);
"-="     RETURN_ASSIGNMENT(MIA);
"%="     RETURN_ASSIGNMENT(MOA);
"|="     RETURN_ASSIGNMENT(OA);
"+="     RETURN_ASSIGNMENT(PA);
"<<="    RETURN_ASSIGNMENT(SLA);
">>="    RETURN_ASSIGNMENT(SRA);
"*="     RETURN_ASSIGNMENT(TA);
"^="     RETURN_ASSIGNMENT(XA);
"<<"     RETURN_(SHL);
">>"     RETURN_(LSR);
"--"     RETURN_(DEC);
"++"     RETURN_(INC);
"=="     RETURN_(EQ);
"!="     RETURN_(NE);
"<="     RETURN_(LE);
">="     RETURN_(GE);
"&&"     RETURN_(AND);
"||"     RETURN_(OR);
"**"     RETURN_(SS);
\n       ++yylineno;
"="      RETURN_ASSIGNMENT(0);
.        RETURN_(yytext[0]);

%%

#pragma warning(pop)

int yywrap() {
	return 1;
}

#pragma warning(disable: 4505) // 'yyunput': unreferenced function with internal linkage has been removed
