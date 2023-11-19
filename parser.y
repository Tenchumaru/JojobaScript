%{
#include <cstdio>
#include <stdexcept>
#include "JojobaScript.h"

#pragma warning(push)
#pragma warning(disable: 4127 4244 4702)
%}

%union {
	std::vector<Statement*>* block;
	std::vector<SwitchStatement::Case>* cases;
	Expression* expr;
	std::vector<Expression*>* expr_list;
	InvocationExpression* fexpr;
	ForStatement::Clause* for_clause;
	std::vector<ForStatement::Clause*>* for_clauses;
	std::string* id;
	IfStatement* if_statement;
	std::vector<IfStatement*>* if_statements;
	std::pair<std::string, std::string>* import;
	std::tuple<std::string, std::string, Expression*>* initializer;
	std::vector<std::tuple<std::string, std::string, Expression*>>* initializers;
	std::vector<std::pair<Expression*, Expression*>>* kv_list;
	Statement* statement;
	bool boolean;
	int value;
	std::vector<std::pair<std::string, std::string>>* vpss;
}

%token AS BREAK CASE CONTINUE DEC DEFAULT DO ELSE FOR FROM FUNCTION IF IMPORT IN INC RETURN SWITCH UNTIL VAR WHILE YIELD
%token <value> ASSIGNMENT NUMBER
%token <id> ID STRING
%nonassoc '?' ':'
%left AND OR
%left EQ NE LE GE '<' '>'
%left '+' '-'
%left '&' '|' '^'
%left ASR LSR SHL
%left '*' '/' '%'
%precedence NEG /* negation:  unary minus, bit-wise complement, logical complement */
%right SS
%nonassoc AWAIT
%left '.' '['
%type<block> block
%type<cases> cases
%type<expr> dexpr dfpexpr expr
%type<expr_list> expr_list oexpr_list
%type<fexpr> fexpr
%type<for_clause> for_clause
%type<for_clauses> for_clauses ofor_clauses
%type<id> otype otype_list type type_list
%type<if_statement> if_statement elseif_statement
%type<if_statements> oelseif_statements
%type<import> import
%type<initializer> initializer
%type<initializers> initializers
%type<kv_list> kv_list
%type<statement> statement
%type<boolean> di uw
%type<vpss> id_list imports oid_list

%%

block:
%empty { $$ = new std::vector<Statement*>; }
| block statement { $1->emplace_back($2); $$ = $1; }
;

statement:
FUNCTION ID '(' oid_list ')' otype '{' block '}' {
	$$ = new FunctionStatement(std::move(*$2), std::move(*$6), std::move(*$4), std::move(*$8));
}
| VAR initializers { $$ = new VarStatement(std::move(*$2)); delete $2; }
| BREAK { $$ = new BreakStatement; }
| CONTINUE { $$ = new ContinueStatement; }
| DO '{' block '}' uw expr { $$ = new DoStatement(std::move(*$3), $6, $5); delete $3; }
| FOR ofor_clauses ';' oexpr_list ';' ofor_clauses '{' block '}' {
	$$ = new ForStatement(std::move(*$2), std::move(*$4), std::move(*$6), std::move(*$8));
	delete $2; delete $4; delete $6; delete $8;
}
| FOR id_list IN expr '{' block '}' {
	$$ = new RangeForStatement(std::move(*$2), $4, std::move(*$6));
	delete $2; delete $6;
}
| if_statement { $$ = $1; }
| if_statement oelseif_statements ELSE '{' block '}' {
	$$ = new IfStatement($1, std::move(*$2), std::move(*$5));
	delete $2; delete $5;
}
| RETURN expr { $$ = new ReturnStatement($2); }
| SWITCH expr '{' cases '}' { $$ = new SwitchStatement($2, std::move(*$4)); delete $4; }
| uw expr '{' block '}' { $$ = new WhileStatement($2, std::move(*$4), $1); delete $4; }
| YIELD expr { $$ = new YieldStatement($2); }
| FROM STRING IMPORT imports { $$ = new ImportStatement(std::move(*$2), std::move(*$4)); delete $2; delete $4; }
| IMPORT STRING { $$ = new ImportStatement(std::move(*$2)); delete $2; }
| IMPORT STRING AS ID { $$ = new ImportStatement(std::move(*$2), std::move(*$4)); delete $2; delete $4; }
| '@' dexpr ASSIGNMENT expr { $$ = new AssignmentStatement($2, $3, $4); }
| '@' fexpr { $$ = new InvocationStatement($2); }
| di dexpr { $$ = new IncrementStatement($2, $1); }
;

otype:
%empty { $$ = new std::string(); }
| ':' type { $$ = $2; }
;

type:
ID
| type '[' otype_list ']' { $$ = new std::string(std::move(std::format("{}[{}]", *$1, *$3))); delete $1; delete $3; }
;

otype_list:
%empty { $$ = new std::string(); }
| type_list
;

type_list:
type
| type_list ',' type { $$ = new std::string(std::move(std::format("{},{}", *$1, *$3))); delete $1; delete $3; }
;

initializers:
initializer { $$ = new std::vector<std::tuple<std::string, std::string, Expression*>>{ { std::move(*$1) } }; delete $1; }
| initializers ',' initializer { $1->emplace_back(std::move(*$3)); delete $3; }
;

initializer:
ID otype { $$ = new std::tuple(std::move(*$1), std::move(*$2), static_cast<Expression*>(nullptr)); delete $1; delete $2; }
| ID otype ASSIGNMENT expr {
	if ($3 != 0) throw std::logic_error("invalid initializing assignment");
	$$ = new std::tuple(std::move(*$1), std::move(*$2), $4); delete $1; delete $2;
}
;

uw:
UNTIL { $$ = false; }
| WHILE { $$ = true; }
;

ofor_clauses:
%empty { $$ = new std::vector<ForStatement::Clause*>; }
| for_clauses
;

for_clauses:
for_clause { $$ = new std::vector<ForStatement::Clause*>{ $1 }; }
| for_clauses ',' for_clause { $1->emplace_back($3); $$ = $1; }
;

for_clause:
dexpr ASSIGNMENT expr { $$ = new ForStatement::AssignmentClause($1, $2, $3); }
| di dexpr { $$ = new ForStatement::DiClause($2, $1); }
| fexpr { $$ = new ForStatement::InvocationClause($1); }
;

if_statement:
IF expr '{' block '}' { $$ = new IfStatement($2, std::move(*$4)); delete $4; }
;

oelseif_statements:
%empty { $$ = new std::vector<IfStatement*>; }
| oelseif_statements elseif_statement { $1->emplace_back($2); $$ = $1; }
;

elseif_statement:
ELSE IF expr '{' block '}' { $$ = new IfStatement($3, std::move(*$5)); delete $5; }
;

cases:
%empty { $$ = new std::vector<SwitchStatement::Case>; }
| cases CASE expr ':' block { $1->emplace_back(SwitchStatement::Case($3, std::move(*$5))); delete $5; $$ = $1; }
| cases DEFAULT ':' block { $1->emplace_back(SwitchStatement::Case(nullptr, std::move(*$4))); delete $4; $$ = $1; }
;

imports:
import { $$ = new std::vector<std::pair<std::string, std::string>>{ std::move(*$1) }; delete $1; }
| imports ',' import { $1->emplace_back(std::move(*$3)); delete $3; $$ = $1; }
;

import:
ID { $$ = new std::pair<std::string, std::string>(std::move(*$1), {}); delete $1; }
| ID AS ID { $$ = new std::pair<std::string, std::string>(std::move(*$1), std::move(*$3)); delete $1; delete $3; }
;

di:
DEC { $$ = false; }
| INC { $$ = true; }
;

expr:
dfpexpr
| expr '?' expr ':' expr { $$ = new TernaryExpression($1, $3, $5); }
| expr AND expr { $$ = new BinaryExpression($1, AND, $3); }
| expr OR expr { $$ = new BinaryExpression($1, OR, $3); }
| expr EQ expr { $$ = new BinaryExpression($1, EQ, $3); }
| expr NE expr { $$ = new BinaryExpression($1, NE, $3); }
| expr LE expr { $$ = new BinaryExpression($1, LE, $3); }
| expr GE expr { $$ = new BinaryExpression($1, GE, $3); }
| expr '<' expr { $$ = new BinaryExpression($1, '<', $3); }
| expr '>' expr { $$ = new BinaryExpression($1, '>', $3); }
| expr '+' expr { $$ = new BinaryExpression($1, '+', $3); }
| expr '-' expr { $$ = new BinaryExpression($1, '-', $3); }
| expr '&' expr { $$ = new BinaryExpression($1, '&', $3); }
| expr '|' expr { $$ = new BinaryExpression($1, '|', $3); }
| expr '^' expr { $$ = new BinaryExpression($1, '^', $3); }
| expr SHL expr { $$ = new BinaryExpression($1, SHL, $3); }
| expr LSR expr { $$ = new BinaryExpression($1, LSR, $3); }
| expr ASR expr { $$ = new BinaryExpression($1, ASR, $3); }
| expr '*' expr { $$ = new BinaryExpression($1, '*', $3); }
| expr '/' expr { $$ = new BinaryExpression($1, '/', $3); }
| expr '%' expr { $$ = new BinaryExpression($1, '%', $3); }
| expr SS expr { $$ = new BinaryExpression($1, SS, $3); }
| '-' expr %prec NEG { $$ = new UnaryExpression($2, '-'); }
| '~' expr %prec NEG { $$ = new UnaryExpression($2, '~'); }
| '!' expr %prec NEG { $$ = new UnaryExpression($2, '!'); }
| '[' oexpr_list ']' { $$ = new ListExpression(std::move(*$2)); delete $2; }
| '[' expr FOR id_list IN expr ']' { $$ = new ListComprehensionExpression($2, std::move(*$4), $6); delete $4; }
| '{' expr ':' expr FOR id_list IN expr '}' { $$ = new DictionaryComprehensionExpression($2, $4, std::move(*$6), $8); delete $6; }
| '{' oexpr_list '}' { if ($2->empty()) $$ = new DictionaryExpression(std::vector<std::pair<Expression*, Expression*>>()); else $$ = new SetExpression(std::move(*$2)); delete $2; }
| '{' kv_list '}' { $$ = new DictionaryExpression(std::move(*$2)); delete $2; }
| AWAIT expr { $$ = new AwaitExpression($2); }
| NUMBER { $$ = new NumericExpression($1); }
;

dfpexpr:
dexpr
| fexpr { $$ = $1; }
| '(' expr ')' { $$ = $2; }
;

dexpr:
dfpexpr '.' ID
| dfpexpr '[' expr ']'
| ID { $$ = new IdentifierExpression(std::move(*$1)); delete $1; }
;

fexpr:
dfpexpr '(' oexpr_list ')' { $$ = new InvocationExpression($1, std::move(*$3)); delete $3; }
;

oexpr_list:
%empty { $$ = new std::vector<Expression*>; }
| expr_list
;

expr_list:
expr { $$ = new std::vector<Expression*>{ $1 }; }
| expr_list ',' expr { $1->emplace_back($3); $$ = $1; }
;

kv_list:
expr ':' expr { $$ = new std::vector<std::pair<Expression*, Expression*>>{ { $1, $3 }}; }
| kv_list ',' expr ':' expr { $1->emplace_back(std::make_pair($3, $5)); }
;

oid_list:
%empty { $$ = new std::vector<std::pair<std::string, std::string>>; }
| id_list
;

id_list:
ID otype { $$ = new std::vector<std::pair<std::string, std::string>>{ { std::move(*$1), std::move(*$2) } }; delete $1; delete $2; }
| id_list ',' ID otype { $1->emplace_back(std::make_pair(std::move(*$3), std::move(*$4))); delete $3; delete $4; $$ = $1; }
;

%%

#pragma warning(pop)

extern int yylineno;

void yyerror(char const* message) {
	fprintf(stderr, "problem in line %d: %s\n", yylineno, message);
}
