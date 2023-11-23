%{
#include <cstdio>
#include <format>
#include <stdexcept>
#include "Expression.h"
#include "JojobaScript.h"
#include "Statement.h"
#include "scanner.h"

#pragma warning(push)
#pragma warning(disable: 4127 4244 4702)
%}

%union {
	Assignment assignment;
	std::vector<std::unique_ptr<Statement>>* block;
	bool boolean;
	SwitchStatement::Case* case_;
	std::vector<SwitchStatement::Case>* cases;
	Expression* expr;
	std::vector<std::unique_ptr<Expression>>* expr_list;
	InvocationExpression* fexpr;
	Statement::Clause* for_clause;
	std::vector<std::unique_ptr<Statement::Clause>>* for_clauses;
	std::string* id;
	IfStatement* if_statement;
	std::vector<std::unique_ptr<IfStatement>>* if_statements;
	std::pair<std::string, std::string>* import;
	std::tuple<std::string, std::string, std::unique_ptr<Expression>>* initializer;
	std::vector<std::tuple<std::string, std::string, std::unique_ptr<Expression>>>* initializers;
	std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>>* kv_list;
	std::variant<std::int64_t, double>* number;
	Statement* statement;
	std::vector<std::pair<std::string, std::string>>* id_list;
}

%token AS BREAK CASE CONTINUE DEC DEFAULT DO ELSE FOR FROM FUNCTION IF IMPORT IN INC RETURN SWITCH UNTIL VAR WHILE YIELD
%token <assignment> ASSIGNMENT
%token <boolean> BOOLEAN
%token <number> NUMBER
%token <id> ID STRING
%nonassoc '?' ':'
%left AND OR
%left EQ NE LE GE '<' '>'
%left '+' '-'
%left '&' '|' '^'
%left ASR LSR LSL
%left '*' '/' '%'
%precedence NEG /* negation:  unary minus, bit-wise complement, logical complement */
%right SS
%nonassoc AWAIT
%left '.' '['
%right ARROW
%type<block> block oelse
%type<case_> case
%type<cases> cases
%type<expr> bexpr iexpr cexpr lexpr expr
%type<expr_list> expr_list oexpr_list
%type<fexpr> fexpr
%type<for_clause> for_clause for_initializer
%type<for_clauses> condition_list for_clauses for_initializers ofor_clauses ofor_initializers oforexpr_list switch_list
%type<id> otype otype_list type type_list
%type<if_statement> elseif_statement
%type<if_statements> oelseif_statements
%type<import> import
%type<initializer> initializer
%type<initializers> initializers
%type<kv_list> kv_list okv_list
%type<statement> statement
%type<boolean> di uw
%type<id_list> id_list imports oid_list

%%

program:
block { FunctionStatement::program = new FunctionStatement("", "", {}, std::move(*$1)); delete $1; }
;

block:
%empty { $$ = new std::vector<std::unique_ptr<Statement>>; }
| block statement { $1->emplace_back($2); $$ = $1; }
;

statement:
FUNCTION ID '(' oid_list ')' otype '{' block '}' {
	$$ = new FunctionStatement(std::move(*$2), std::move(*$6), std::move(*$4), std::move(*$8));
	delete $8;
}
| VAR initializers { $$ = new VarStatement(std::move(*$2)); delete $2; }
| BREAK { $$ = new BreakStatement; }
| CONTINUE { $$ = new ContinueStatement; }
| DO '{' block '}' uw expr { $$ = new DoStatement(std::move(*$3), $6, $5); delete $3; }
| FOR ofor_initializers ';' oforexpr_list ';' ofor_clauses '{' block '}' {
	$$ = new ForStatement(std::move(*$2), std::move(*$4), std::move(*$6), std::move(*$8));
	delete $2; delete $4; delete $6; delete $8;
}
| FOR id_list IN expr '{' block '}' {
	$$ = new RangeForStatement(std::move(*$2), $4, std::move(*$6));
	delete $2; delete $6;
}
| IF condition_list '{' block '}' oelseif_statements oelse {
	$$ = new IfStatement(std::move(*$2), std::move(*$4), std::move(*$6), std::move(*$7));
	delete $2; delete $4; delete $6; delete $7;
}
| RETURN expr { $$ = new ReturnStatement($2); }
| SWITCH switch_list '{' cases '}' { $$ = new SwitchStatement(std::move(*$2), std::move(*$4)); delete $2; delete $4; }
| uw condition_list '{' block '}' { $$ = new WhileStatement(std::move(*$2), std::move(*$4), $1); delete $2; delete $4; }
| YIELD expr { $$ = new YieldStatement($2); }
| FROM STRING IMPORT imports { $$ = new ImportStatement(std::move(*$2), std::move(*$4)); delete $2; delete $4; }
| IMPORT STRING { $$ = new ImportStatement(std::move(*$2)); delete $2; }
| IMPORT STRING AS ID { $$ = new ImportStatement(std::move(*$2), std::move(*$4)); delete $2; delete $4; }
| '@' lexpr ASSIGNMENT expr { $$ = new AssignmentStatement($2, $3, $4); }
| '@' expr { $$ = new ExpressionStatement($2); }
| di lexpr { $$ = new IncrementStatement($2, $1); }
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
initializer { $$ = new std::vector<std::tuple<std::string, std::string, std::unique_ptr<Expression>>>; $$->emplace_back(std::move(*$1)); delete $1; }
| initializers ',' initializer { $1->emplace_back(std::move(*$3)); delete $3; }
;

initializer:
ID otype { $$ = new std::tuple(std::move(*$1), std::move(*$2), std::unique_ptr<Expression>()); delete $1; delete $2; }
| ID otype ASSIGNMENT expr {
	if ($3 != Assignment()) throw std::logic_error("invalid initializing assignment");
	$$ = new std::tuple(std::move(*$1), std::move(*$2), std::unique_ptr<Expression>($4)); delete $1; delete $2;
}
;

uw:
UNTIL { $$ = false; }
| WHILE { $$ = true; }
;

ofor_initializers:
%empty { $$ = new std::vector<std::unique_ptr<Statement::Clause>>; }
| for_initializers
;

for_initializers:
for_initializer { $$ = new std::vector<std::unique_ptr<Statement::Clause>>; $$->emplace_back($1); }
| for_initializers ',' for_initializer { $1->emplace_back($3); $$ = $1; }
;

for_initializer:
for_clause
| VAR initializer { $$ = new Statement::AssignmentClause(std::move(*$2)); delete $2; }
;

ofor_clauses:
%empty { $$ = new std::vector<std::unique_ptr<Statement::Clause>>; }
| for_clauses
;

for_clauses:
for_clause { $$ = new std::vector<std::unique_ptr<Statement::Clause>>; $$->emplace_back($1); }
| for_clauses ',' for_clause { $1->emplace_back($3); $$ = $1; }
;

for_clause:
'@' lexpr ASSIGNMENT expr { $$ = new Statement::AssignmentClause($2, $3, $4); }
| di lexpr { $$ = new Statement::DiClause($2, $1); }
| '@' expr { $$ = new Statement::ExpressionClause($2); }
;

oforexpr_list:
%empty { $$ = new std::vector<std::unique_ptr<Statement::Clause>>; }
| bexpr { $$ = new std::vector<std::unique_ptr<Statement::Clause>>; $$->emplace_back(std::make_unique<Statement::ExpressionClause>($1)); }
| for_clauses ',' bexpr { $1->emplace_back(std::make_unique<Statement::ExpressionClause>($3)); $$ = $1; }
;

switch_list:
expr { $$ = new std::vector<std::unique_ptr<Statement::Clause>>; $$->emplace_back(new Statement::ExpressionClause($1)); }
| for_initializers ',' expr { $1->emplace_back(new Statement::ExpressionClause($3)); $$ = $1; }
;

condition_list:
bexpr { $$ = new std::vector<std::unique_ptr<Statement::Clause>>; $$->emplace_back(new Statement::ExpressionClause($1)); }
| for_initializers ',' bexpr { $1->emplace_back(new Statement::ExpressionClause($3)); $$ = $1; }
;

oelseif_statements:
%empty { $$ = new std::vector<std::unique_ptr<IfStatement>>; }
| oelseif_statements elseif_statement { $1->emplace_back($2); $$ = $1; }
;

oelse:
%empty { $$ = new std::vector<std::unique_ptr<Statement>>; }
| ELSE '{' block '}' { $$ = $3; }
;

elseif_statement:
ELSE IF condition_list '{' block '}' { $$ = new IfStatement(std::move(*$3), std::move(*$5)); delete $3; delete $5; }
;

cases:
case { $$ = new std::vector<SwitchStatement::Case>; $$->emplace_back(std::move(*$1)); delete $1; }
| cases case { $1->emplace_back(std::move(*$2)); delete $2; $$ = $1; }
;

case:
CASE expr ':' block { $$ = new SwitchStatement::Case($2, std::move(*$4)); delete $4; }
| DEFAULT ':' block { $$ = new SwitchStatement::Case(nullptr, std::move(*$3)); delete $3; }
;

imports:
import { $$ = new std::vector{ std::move(*$1) }; delete $1; }
| imports ',' import { $1->emplace_back(std::move(*$3)); delete $3; $$ = $1; }
;

import:
ID { $$ = new std::pair(std::move(*$1), std::string()); delete $1; }
| ID AS ID { $$ = new std::pair(std::move(*$1), std::move(*$3)); delete $1; delete $3; }
;

di:
DEC { $$ = false; }
| INC { $$ = true; }
;

expr:
':' '(' id_list ')' '{' block '}' { $$ = new LambdaExpression(std::move(*$3), std::move(*$6)); }
| ':' '(' id_list ')' ARROW expr { $$ = new LambdaExpression(std::move(*$3), $6); }
| bexpr
| cexpr
;

cexpr:
'[' oexpr_list ']' { $$ = new ListExpression(std::move(*$2)); delete $2; }
| '[' expr FOR id_list IN expr ']' { $$ = new ListComprehensionExpression($2, std::move(*$4), $6); delete $4; }
| '{' expr ':' expr FOR id_list IN expr '}' { $$ = new DictionaryComprehensionExpression($2, $4, std::move(*$6), $8); delete $6; }
| '{' expr_list '}' { $$ = new SetExpression(std::move(*$2)); delete $2; }
| '{' okv_list '}' { $$ = new DictionaryExpression(std::move(*$2)); delete $2; }
;

bexpr:
BOOLEAN { $$ = new BooleanExpression($1); }
| NUMBER { $$ = new NumericExpression(std::move(*$1)); delete $1; }
| AWAIT expr { $$ = new AwaitExpression($2); }
| '-' expr %prec NEG { $$ = new UnaryExpression($2, '-'); }
| '~' expr %prec NEG { $$ = new UnaryExpression($2, '~'); }
| '!' expr %prec NEG { $$ = new UnaryExpression($2, '!'); }
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
| expr LSL expr { $$ = new BinaryExpression($1, LSL, $3); }
| expr LSR expr { $$ = new BinaryExpression($1, LSR, $3); }
| expr ASR expr { $$ = new BinaryExpression($1, ASR, $3); }
| expr '*' expr { $$ = new BinaryExpression($1, '*', $3); }
| expr '/' expr { $$ = new BinaryExpression($1, '/', $3); }
| expr '%' expr { $$ = new BinaryExpression($1, '%', $3); }
| expr SS expr { $$ = new BinaryExpression($1, SS, $3); }
| expr '?' expr ':' expr { $$ = new TernaryExpression($1, $3, $5); }
| iexpr
;

iexpr:
fexpr { $$ = $1; }
| '(' expr ')' { $$ = $2; }
| lexpr
;

fexpr:
iexpr '(' oexpr_list ')' { $$ = new InvocationExpression($1, std::move(*$3)); delete $3; }
;

lexpr:
iexpr '.' ID { $$ = new DotExpression($1, std::move(*$3)); delete $3; }
| cexpr '.' ID { $$ = new DotExpression($1, std::move(*$3)); delete $3; }
| iexpr '[' expr ']' { $$ = new IndexExpression($1, $3); }
| ID { $$ = new IdentifierExpression(std::move(*$1)); delete $1; }
;

oexpr_list:
%empty { $$ = new std::vector<std::unique_ptr<Expression>>; }
| expr_list
;

expr_list:
expr { $$ = new std::vector<std::unique_ptr<Expression>>; $$->emplace_back($1); }
| expr_list ',' expr { $1->emplace_back($3); $$ = $1; }
;

okv_list:
%empty { $$ = new std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>>; }
| kv_list
;

kv_list:
expr ':' expr { $$ = new std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>>; $$->emplace_back(std::make_pair(std::unique_ptr<Expression>($1), std::unique_ptr<Expression>($3))); }
| kv_list ',' expr ':' expr { $1->emplace_back(std::make_pair($3, $5)); }
;

oid_list:
%empty { $$ = new std::vector<std::pair<std::string, std::string>>; }
| id_list
;

id_list:
ID otype { $$ = new std::vector{ std::make_pair(std::move(*$1), std::move(*$2)) }; delete $1; delete $2; }
| id_list ',' ID otype { $1->emplace_back(std::make_pair(std::move(*$3), std::move(*$4))); delete $3; delete $4; $$ = $1; }
;

%%

#pragma warning(pop)

extern int yylineno;

void yyerror(char const* message) {
	fprintf(stderr, "problem in line %d: %s\n", yylineno, message);
}
