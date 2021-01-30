%{
#include <cassert> // assert
#include <cstdio> // printf
#include <cstdlib> // strtoul
#include <cstring> // _strdup
#include "parser.h"
#include "scanner.h"
#ifdef _DEBUG
# define RETURN_(v) return (printf(" '%s':%d ", yytext, (v)), (v))
#else
# define RETURN_(v) return (v)
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
{hex}    { yylval.value = strtoul(yytext + 1, nullptr, 16); RETURN_(NUMBER); }
{num}    { yylval.value = strtoul(yytext, nullptr, 10); RETURN_(NUMBER); }
{str}    { yylval.id = _strdup(yytext); RETURN_(STRING); }
continue { RETURN_(CONTINUE); }
function { RETURN_(FUNCTION); }
default  { RETURN_(DEFAULT); }
import   { RETURN_(IMPORT); }
return   { RETURN_(RETURN); }
switch   { RETURN_(SWITCH); }
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
"&="     RETURN_(AA);
">>>="   RETURN_(ARA);
"/="     RETURN_(DA);
"-="     RETURN_(MIA);
"%="     RETURN_(MOA);
"|="     RETURN_(OA);
"+="     RETURN_(PA);
"<<="    RETURN_(SLA);
">>="    RETURN_(SRA);
"*="     RETURN_(TA);
"^="     RETURN_(XA);
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
\n       { ++yylineno; if(bracket_depth == 0) RETURN_(SEP); }
";"      RETURN_(SEP);
"{"      |
"("      |
"["      { ++bracket_depth; RETURN_(yytext[0]); }
"}"      { if(bracket_depth > 0) --bracket_depth; RETURN_(yytext[0]); }
")"      |
"]"      { --bracket_depth; RETURN_(yytext[0]); }
.        RETURN_(yytext[0]);

%%

#pragma warning(pop)

int yywrap() {
	return 1;
}

void undo_bracket(int expected_bracket_depth) {
	// The detection of an open curly brace immediately succeeds an invocation of this function.  For those constructs that require a
	// token look-ahead, the bracket depth will be one.  Otherwise, it will be zero.
	printf("bracket_depth is %d\n", bracket_depth);
	assert((expected_bracket_depth & ~1) == 0);
	assert(bracket_depth == expected_bracket_depth);
	--bracket_depth;
}

#pragma warning(disable: 4505)
