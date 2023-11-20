%{
#pragma warning(push)
#pragma warning(disable: 4996) // 'read': The POSIX name for this item is deprecated. Instead, use the ISO C and C++ conformant name: _read. See online help for details.
#pragma warning(disable: 4005) // '*': macro redefinition
#include <cstdlib> // std::strtod and std::strtoll
#include "Debug.h"
#include "JojobaScript.h"
#define DWL(v) DebugWriteLine(" '%s':%d ", yytext[0] == '\n' ? "" : yytext, v) // See Project Properties for Adrezdi library reference.
#define RETURN_(v) do { auto v__ = (v); DWL(v__); return v__; } while(false)
#define RETURN_ASSIGNMENT(v) do { auto v__ = (v); DWL(v__); yylval.assignment = v__; return ASSIGNMENT; } while(false)
%}

ws      [\r\t\v ]+
rem     \/\/.*
id      [_A-Za-z][_0-9A-Za-z]*
hex     $[0-9A-Fa-f]+
num     [0-9]+
str     \"([^\"]|(\\\"))*\"
real    ({num}?\.{num}|{num}\.{num}?)([Ee][-+]?{num})?

%x COMMENT

%%

{ws}     |
{rem}    ;
"infinity" | 
"NaN"    |
{real}   { yylval.number = new std::variant<std::int64_t, double>(std::strtod(yytext + 1, nullptr)); RETURN_(NUMBER); }
{hex}    { yylval.number = new std::variant<std::int64_t, double>(std::strtoll(yytext + 1, nullptr, 16)); RETURN_(NUMBER); }
{num}    { yylval.number = new std::variant<std::int64_t, double>(std::strtoll(yytext, nullptr, 10)); RETURN_(NUMBER); }
{str}    { yylval.id = new std::string(yytext); RETURN_(STRING); }
continue { RETURN_(CONTINUE); }
function { RETURN_(FUNCTION); }
default  { RETURN_(DEFAULT); }
import   { RETURN_(IMPORT); }
return   { RETURN_(RETURN); }
switch   { RETURN_(SWITCH); }
await    { RETURN_(AWAIT); }
break    { RETURN_(BREAK); }
false    { yylval.boolean = false; RETURN_(BOOLEAN); }
until    { RETURN_(UNTIL); }
while    { RETURN_(WHILE); }
yield    { RETURN_(YIELD); }
case     { RETURN_(CASE); }
else     { RETURN_(ELSE); }
from     { RETURN_(FROM); }
true     { yylval.boolean = true; RETURN_(BOOLEAN); }
for      { RETURN_(FOR); }
var      { RETURN_(VAR); }
as       { RETURN_(AS); }
do       { RETURN_(DO); }
if       { RETURN_(IF); }
in       { RETURN_(IN); }
{id}     { yylval.id = new std::string(yytext); RETURN_(ID); }
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
