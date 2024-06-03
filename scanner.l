%{
#pragma warning(push)
#pragma warning(disable: 4996) // 'read': The POSIX name for this item is deprecated. Instead, use the ISO C and C++ conformant name: _read. See online help for details.
#pragma warning(disable: 4005) // '*': macro redefinition
#include <cstdlib> // std::strtod and std::strtoll
#include <cstring> // std::strlen
#include <limits> // std::numeric_limits
#include "Debug.h"
#include "Expression.h"
#include "Statement.h"
#include "yy.h"
#define DWL(v) DebugWriteLine(" '%s':%d ", yytext[0] == '\n' ? "" : yytext, v) // See Project Properties for Adrezdi library reference.
#define RETURN_(v) do { auto v__ = (v); DWL(v__); return v__; } while(false)
#define RETURN_ASSIGNMENT(v) do { auto v__ = (v); DWL(v__); yylval.assignment = v__; return ASSIGNMENT; } while(false)

static std::string* MakeString(char const* p);
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
{real}   { yylval.number = new Value(std::strtod(yytext, nullptr)); RETURN_(NUMBER); }
{hex}    { yylval.number = new Value(std::strtoll(yytext + 1, nullptr, 16)); RETURN_(NUMBER); }
{num}    { yylval.number = new Value(std::strtoll(yytext, nullptr, 10)); RETURN_(NUMBER); }
{str}    { yylval.id = MakeString(yytext); RETURN_(STRING); }
fallthrough { RETURN_(FALLTHROUGH); }
[Ii]nfinity |
INF         |
[Ii]nf      { yylval.number = new Value(std::numeric_limits<double>::infinity()); RETURN_(NUMBER); }
continue { RETURN_(CONTINUE); }
function { RETURN_(FUNCTION); }
default  { RETURN_(DEFAULT); }
finally  { RETURN_(FINALLY); }
rethrow  { RETURN_(RETHROW); }
import   { RETURN_(IMPORT); }
return   { RETURN_(RETURN); }
switch   { RETURN_(SWITCH); }
await    { RETURN_(AWAIT); }
break    { RETURN_(BREAK); }
catch    { RETURN_(CATCH); }
const    { yylval.boolean = true; RETURN_(VAR); }
false    { yylval.boolean = false; RETURN_(BOOLEAN); }
throw    { RETURN_(THROW); }
until    { RETURN_(UNTIL); }
while    { RETURN_(WHILE); }
yield    { RETURN_(YIELD); }
case     { RETURN_(CASE); }
else     { RETURN_(ELSE); }
exit     { RETURN_(EXIT); }
from     { RETURN_(FROM); }
true     { yylval.boolean = true; RETURN_(BOOLEAN); }
N[Aa]N   |
nan      { yylval.number = new Value(std::numeric_limits<double>::quiet_NaN()); RETURN_(NUMBER); }
for      { RETURN_(FOR); }
try      { RETURN_(TRY); }
var      { yylval.boolean = false; RETURN_(VAR); }
as       { RETURN_(AS); }
do       { RETURN_(DO); }
if       { RETURN_(IF); }
in       { RETURN_(IN); }
{id}     { yylval.id = new std::string(yytext); RETURN_(ID); }
">>>="   RETURN_ASSIGNMENT(Assignment::LSRA);
"<<="    RETURN_ASSIGNMENT(Assignment::SLA);
">>="    RETURN_ASSIGNMENT(Assignment::ASRA);
"%="     RETURN_ASSIGNMENT(Assignment::MA);
"&="     RETURN_ASSIGNMENT(Assignment::AA);
"*="     RETURN_ASSIGNMENT(Assignment::TA);
"+="     RETURN_ASSIGNMENT(Assignment::PA);
"-="     RETURN_ASSIGNMENT(Assignment::SA);
"/="     RETURN_ASSIGNMENT(Assignment::DA);
"^="     RETURN_ASSIGNMENT(Assignment::XA);
"|="     RETURN_ASSIGNMENT(Assignment::OA);
">>>"    RETURN_(LSR);
"@{"     RETURN_(OBJECT);
"!="     RETURN_(NE);
"<="     RETURN_(LE);
"=="     RETURN_(EQ);
">="     RETURN_(GE);
"&&"     RETURN_(AND);
"||"     RETURN_(OR);
"**"     RETURN_(SS);
"++"     RETURN_(INC);
"--"     RETURN_(DEC);
"->"     RETURN_(ARROW);
"=>"     RETURN_(ARROW);
"<<"     RETURN_(SL);
">>"     RETURN_(ASR);
"@("     RETURN_(ATOP);
"@["     RETURN_(LIST);
"/*"     BEGIN(COMMENT);
"="      RETURN_ASSIGNMENT(Assignment());
!+       |
~+       { yylval.n = std::strlen(yytext); RETURN_(yytext[0]); }
-/{ws}   RETURN_(yytext[0]);
-        RETURN_(NEG);
\n       ++yylineno;
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
