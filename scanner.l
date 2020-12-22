%{
#include <cstdlib> // strtoul
#include <cstring> // _strdup
#include "Debug.h"
#include "parser.h"
#include "scanner.h"
#define DWL(v) DebugWriteLine(" '%s':%d ", yytext[0] == '\n' ? "" : yytext, v) // See Project Properties for Adrezdi library reference.
#define RETURN_(v) do { auto v__ = (v); DWL(v__); return v__; } while(false)
#define RETURN_ASSIGNMENT(v) do { auto v__ = (v); DWL(v__); yylval.value = v__; return ASSIGNMENT; } while(false)
#pragma warning(push)
#pragma warning(disable: 4996) // 'read': The POSIX name for this item is deprecated. Instead, use the ISO C and C++ conformant name: _read. See online help for details.
%}

ws      [\r\t\v ]+
rem     \/\/.*
id      [_A-Za-z][_0-9A-Za-z]*
hex     $[0-9A-Fa-f]+
num     [0-9]+
str     \"[^"]\"

%x COMMENT

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
in       { RETURN_(IN); }
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
"/*"     BEGIN(COMMENT);
\n       ++yylineno;
"="      RETURN_ASSIGNMENT(0);
.        RETURN_(yytext[0]);

<COMMENT>{
"*/"     BEGIN(INITIAL);
[^*]*    |
.        ;
}

%%

#pragma warning(pop)

int yywrap() {
	return 1;
}

#pragma warning(disable: 4505) // 'yyunput': unreferenced function with internal linkage has been removed
