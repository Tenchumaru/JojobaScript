%{
#include "Expression.h"
#include "Statement.h"

#pragma warning(push)
#pragma warning(disable: 4127 4244 4702)

namespace {
	enum class ReturnType { Unspecified, Return, Yield };

	std::vector<ReturnType> returnTypeStack{ ReturnType::Unspecified };

	bool Yielding() { return returnTypeStack.back() == ReturnType::Yield; }

	void CheckUniqueness(std::vector<std::pair<std::string, std::string>>* ids) {
		std::unordered_set<std::string> set;
		std::transform(ids->begin(), ids->end(), std::inserter(set, set.end()), [](auto&& pair) { return pair.first; });
		if (ids->size() != set.size()) {
			throw std::runtime_error("ids are not unique");
		}
	}
}
%}

%union {
	Assignment assignment;
	std::vector<std::unique_ptr<Statement>>* block;
	std::pair<std::vector<std::unique_ptr<Statement>>, std::vector<std::unique_ptr<Statement>>>* block_pair;
	bool boolean;
	Expression* expr;
	std::vector<std::unique_ptr<Expression>>* expr_list;
	Statement::Clause* for_clause;
	std::vector<std::unique_ptr<Statement::Clause>>* for_clauses;
	std::string* id;
	std::vector<std::pair<std::string, std::string>>* id_list;
	std::unordered_map<std::string, std::pair<std::unique_ptr<Expression>, std::string>>* idv_list;
	IfStatement::Fragment* if_fragment;
	std::vector<std::unique_ptr<IfStatement::Fragment>>* if_fragments;
	std::pair<std::string, std::string>* import;
	std::tuple<std::string, std::string, std::unique_ptr<Expression>>* initializer;
	std::vector<std::tuple<std::string, std::string, std::unique_ptr<Expression>>>* initializers;
	std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>>* kv_list;
	Value* number;
	size_t n;
	Statement* statement;
}

%token AS ATOP BREAK CASE CATCH CONTINUE DEC DEFAULT DO DORS ELSE EXIT FALLTHROUGH FINALLY FOR FROM FUNCTION IF IMPORT IN INC LIST OBJECT RETHROW RETURN SWITCH THROW TRY UNTIL WHILE YIELD
%token <assignment> ASSIGNMENT
%token <boolean> BOOLEAN VAR
%token <id> ID STRING
%token <number> NUMBER
%right ARROW
%nonassoc '?' ':'
%left AND OR
%left EQ NE LE GE '<' '>'
%left '+' '-'
%left '&' '|' '^'
%left ASR LSR SL
%left '*' '/' '%'
%right<n> '!' '~' NEG
%right SS
%nonassoc AWAIT
%left '(' '.' '['
%type<block> block cases oelse
%type<block_pair> catch_finally
%type<boolean> di uw
%type<expr> bexpr expr lexpr sexpr
%type<expr_list> expr_list lexpr_list oexpr_list
%type<for_clause> for_clause
%type<for_clauses> for_clauses ofor_clauses oforexpr_list
%type<id> otype otype_list type type_list
%type<id_list> id_list imports oid_list
%type<idv_list> idv_list oidv_list
%type<if_fragment> onlyif
%type<if_fragments> oelseifs
%type<import> import
%type<initializer> initializer
%type<initializers> initializers
%type<kv_list> kv_list okv_list
%type<n> obreaks
%type<statement> case oinitializers statement

%%

program:
block {
	if (Yielding()) {
		throw std::runtime_error("cannot yield at the top level");
	}
	program.reset(new FunctionStatement("", "", {}, std::move(*$1), false)); delete $1;
}
;

block:
%empty { $$ = new std::vector<std::unique_ptr<Statement>>; }
| block statement { $1->emplace_back($2); $$ = $1; }
;

statement:
FUNCTION ID '(' { returnTypeStack.push_back({}); } oid_list ')' otype '{' block '}' {
	$$ = new FunctionStatement(std::move(*$2), std::move(*$7), std::move(*$5), std::move(*$9), Yielding());
	returnTypeStack.pop_back();
	delete $2; delete $5; delete $7; delete $9;
}
| VAR initializers { $$ = new VarStatement(std::move(*$2), $1); delete $2; }
| obreaks BREAK { $$ = new BreakStatement($1); }
| obreaks CONTINUE { $$ = new ContinueStatement($1); }
| FALLTHROUGH { $$ = new FallthroughStatement(); }
| DO '{' block '}' uw bexpr { $$ = new DoStatement(std::move(*$3), $6, $5); delete $3; }
| FOR oinitializers ';' oforexpr_list ';' ofor_clauses '{' block '}' {
	$$ = new ForStatement(std::unique_ptr<Statement>($2), std::move(*$4), std::move(*$6), std::move(*$8));
	delete $4; delete $6; delete $8;
}
| FOR id_list IN expr '{' block '}' {
	CheckUniqueness($2);
	$$ = new RangeForStatement(std::move(*$2), $4, std::move(*$6)); delete $2; delete $6;
}
| onlyif oelseifs oelse {
	$2->emplace($2->begin(), $1);
	$$ = new IfStatement(std::move(*$2), std::move(*$3)); delete $2; delete $3;
}
| EXIT {
	if (Yielding()) {
		throw std::runtime_error("cannot exit and yield in the same function");
	}
	returnTypeStack.back() = ReturnType::Return;
	$$ = new ReturnStatement(nullptr);
}
| RETURN expr {
	if (Yielding()) {
		throw std::runtime_error("cannot return and yield in the same function");
	}
	returnTypeStack.back() = ReturnType::Return;
	$$ = new ReturnStatement($2);
}
| SWITCH oinitializers expr '{' cases '}' {
	$$ = new SwitchStatement(std::unique_ptr<Statement>($2), std::unique_ptr<Expression>($3), std::move(*$5)); delete $5;
}
| RETHROW { $$ = new ThrowStatement(nullptr); }
| THROW expr { $$ = new ThrowStatement($2); }
| TRY '{' block '}' catch_finally {
	$$ = new TryStatement(std::move(*$3), std::move($5->first), std::move($5->second)); delete $3; delete $5;
}
| uw oinitializers bexpr '{' block '}' {
	$$ = new WhileStatement(std::unique_ptr<Statement>($2), std::unique_ptr<Expression>($3), std::move(*$5), $1); delete $5;
}
| YIELD expr {
	if (returnTypeStack.back() == ReturnType::Return) {
		throw std::runtime_error("cannot return and yield in the same function");
	}
	returnTypeStack.back() = ReturnType::Yield;
	$$ = new YieldStatement($2);
}
| FROM STRING IMPORT imports { $$ = new ImportStatement(std::move(*$2), std::move(*$4)); delete $2; delete $4; }
| IMPORT STRING { $$ = new ImportStatement(std::move(*$2)); delete $2; }
| IMPORT STRING AS ID { $$ = new ImportStatement(std::move(*$2), std::move(*$4)); delete $2; delete $4; }
| lexpr_list ASSIGNMENT expr_list { $$ = new AssignmentStatement(std::move(*$1), $2, std::move(*$3)); delete $1; delete $3; }
| sexpr { $$ = new ExpressionStatement($1); }
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
initializer {
	$$ = new std::vector<std::tuple<std::string, std::string, std::unique_ptr<Expression>>>;
	$$->emplace_back(std::move(*$1)); delete $1;
}
| initializers ',' initializer { $1->emplace_back(std::move(*$3)); delete $3; }
;

initializer:
ID otype ASSIGNMENT expr {
	if ($3 != Assignment()) throw std::runtime_error("invalid initializing assignment");
	$$ = new std::tuple(std::move(*$1), std::move(*$2), std::unique_ptr<Expression>($4)); delete $1; delete $2;
}
;

obreaks:
%empty { $$ = 0; }
| obreaks BREAK ',' { $$ = $1 + 1; }
;

uw:
UNTIL { $$ = false; }
| WHILE { $$ = true; }
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
lexpr ASSIGNMENT expr { $$ = new Statement::AssignmentClause($1, $2, $3); }
| '(' lexpr_list ASSIGNMENT expr_list ')' {
	$$ = new Statement::AssignmentClause(std::move(*$2), $3, std::move(*$4)); delete $2; delete $4;
}
| di lexpr { $$ = new Statement::DiClause($2, $1); }
| sexpr { $$ = new Statement::ExpressionClause($1); }
;

oforexpr_list:
%empty { $$ = new std::vector<std::unique_ptr<Statement::Clause>>; }
| bexpr {
	$$ = new std::vector<std::unique_ptr<Statement::Clause>>;
	$$->emplace_back(std::make_unique<Statement::ExpressionClause>($1));
}
| for_clauses ',' bexpr { $1->emplace_back(std::make_unique<Statement::ExpressionClause>($3)); $$ = $1; }
;

catch_finally:
CATCH '{' block '}' { $$ = new std::pair(std::move(*$3), std::vector<std::unique_ptr<Statement>>{}); delete $3; }
| CATCH '{' block '}' FINALLY '{' block '}' { $$ = new std::pair(std::move(*$3), std::move(*$7)); delete $3; delete $7; }
| FINALLY '{' block '}' { $$ = new std::pair(std::vector<std::unique_ptr<Statement>>{}, std::move(*$3)); delete $3; }
;

onlyif:
IF oinitializers bexpr '{' block '}' { $$ = new IfStatement::Fragment($2, $3, std::move(*$5)); delete $5; }
;

oelseifs:
%empty { $$ = new std::vector<std::unique_ptr<IfStatement::Fragment>>; }
| oelseifs ELSE onlyif { $1->emplace_back($3); $$ = $1; }
;

oelse:
%empty { $$ = new std::vector<std::unique_ptr<Statement>>; }
| ELSE '{' block '}' { $$ = $3; }
;

oinitializers:
%empty { $$ = nullptr; }
| VAR '(' initializers ')' { $$ = new VarStatement(std::move(*$3), $1); delete $3; }
;

cases:
case { $$ = new std::vector<std::unique_ptr<Statement>>; $$->emplace_back($1); }
| cases case { $1->emplace_back($2); $$ = $1; }
;

case:
CASE expr ':' block { $$ = new SwitchStatement::Case($2, std::move(*$4)); delete $4; }
| DEFAULT ':' block { $$ = new SwitchStatement::DefaultCase(std::move(*$3)); delete $3; }
;

imports:
import { $$ = new std::vector<std::pair<std::string, std::string>>; $$->emplace_back(std::move(*$1)); delete $1; }
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
ATOP id_list ')' otype { returnTypeStack.push_back({}); } '{' block '}' {
	$$ = new LambdaExpression(std::move(*$4), std::move(*$2), std::move(*$7), Yielding());
	delete $2; delete $4; delete $7;
}
| ATOP id_list ')' otype ARROW expr { $$ = new LambdaExpression(std::move(*$4), std::move(*$2), $6); delete $2; delete $4; }
| LIST oexpr_list ']' { $$ = new ListExpression(std::move(*$2)); delete $2; }
| LIST expr FOR id_list IN expr ']' {
	CheckUniqueness($4);
	$$ = new ListComprehensionExpression($2, std::move(*$4), $6); delete $4;
}
| '(' expr FOR id_list IN expr ')' {
	CheckUniqueness($4);
	$$ = new GeneratorExpression($2, std::move(*$4), $6); delete $4;
}
| DORS expr ':' expr FOR id_list IN expr '}' {
	CheckUniqueness($6);
	$$ = new DictionaryComprehensionExpression($2, $4, std::move(*$6), $8); delete $6;
}
| DORS expr FOR id_list IN expr '}' {
	CheckUniqueness($4);
	$$ = new SetComprehensionExpression($2, std::move(*$4), $6); delete $4;
}
| DORS expr_list '}' { $$ = new SetExpression(std::move(*$2)); delete $2; }
| DORS okv_list '}' { $$ = new DictionaryExpression(std::move(*$2)); delete $2; }
| OBJECT oidv_list '}' { $$ = new ObjectExpression(std::move(*$2)); delete $2; }
| bexpr
;

bexpr:
BOOLEAN { $$ = new LiteralExpression($1); }
| NUMBER { $$ = new LiteralExpression(std::move(*$1)); delete $1; }
| STRING { $$ = new LiteralExpression(std::move(*$1)); delete $1; }
| NEG expr { $$ = new UnaryExpression($2, '-', 1); }
| '~' expr { $$ = new UnaryExpression($2, '~', $1); }
| '!' expr { $$ = new UnaryExpression($2, '!', $1); }
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
| expr '*' expr { $$ = new BinaryExpression($1, '*', $3); }
| expr '/' expr { $$ = new BinaryExpression($1, '/', $3); }
| expr '%' expr { $$ = new BinaryExpression($1, '%', $3); }
| expr ASR expr { $$ = new BinaryExpression($1, ASR, $3); }
| expr LSR expr { $$ = new BinaryExpression($1, LSR, $3); }
| expr SL expr { $$ = new BinaryExpression($1, SL, $3); }
| expr SS expr { $$ = new BinaryExpression($1, SS, $3); }
| expr '?' expr ':' expr { $$ = new TernaryExpression($1, $3, $5); }
| '(' expr ')' { $$ = $2; }
| lexpr
| sexpr
;

lexpr:
expr '.' ID { $$ = new DotExpression($1, std::move(*$3)); delete $3; }
| expr '[' expr ']' { $$ = new IndexExpression($1, $3); }
| expr '[' ':' expr ']' { $$ = new IndexExpression($1, new LiteralExpression(0), $4); }
| expr '[' expr ':' ']' { $$ = new IndexExpression($1, $3, nullptr); }
| expr '[' expr ':' expr ']' { $$ = new IndexExpression($1, $3, $5); }
| ID { $$ = new IdentifierExpression(std::move(*$1)); delete $1; }
;

sexpr:
AWAIT expr { $$ = new AwaitExpression($2); }
| '@' expr '(' oexpr_list ')' { $$ = new InvocationExpression($2, std::move(*$4)); delete $4; }
;

lexpr_list:
lexpr { $$ = new std::vector<std::unique_ptr<Expression>>; $$->emplace_back($1); }
| lexpr_list ',' lexpr { $1->emplace_back($3); $$ = $1; }
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
expr ':' expr {
	$$ = new std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>>;
	$$->emplace_back(std::make_pair(std::unique_ptr<Expression>($1), std::unique_ptr<Expression>($3)));
}
| kv_list ',' expr ':' expr { $1->emplace_back(std::make_pair($3, $5)); }
;

oid_list:
%empty { $$ = new std::vector<std::pair<std::string, std::string>>; }
| id_list { CheckUniqueness($1); $$ = $1; }
;

id_list:
ID otype {
	$$ = new std::vector<std::pair<std::string, std::string>>;
	$$->emplace_back(std::make_pair(std::move(*$1), std::move(*$2))); delete $1; delete $2;
}
| id_list ',' ID otype { $1->emplace_back(std::make_pair(std::move(*$3), std::move(*$4))); delete $3; delete $4; $$ = $1; }
;

oidv_list:
%empty { $$ = new std::unordered_map<std::string, std::pair<std::unique_ptr<Expression>, std::string>>; }
| idv_list
;

idv_list:
ID ':' expr otype {
	$$ = new std::unordered_map<std::string, std::pair<std::unique_ptr<Expression>, std::string>>;
	(*$$)[*$1] = std::make_pair(std::unique_ptr<Expression>($3), std::move(*$4)); delete $1; delete $4;
}
| idv_list ',' ID ':' expr otype {
	(*$1)[*$3] = std::make_pair(std::unique_ptr<Expression>($5), std::move(*$6)); delete $3; delete $6; $$ = $1;
}
;

%%

#pragma warning(pop)

extern int yylineno;

void yyerror(char const* message) {
	fprintf(stderr, "problem in line %d: %s\n", yylineno, message);
}
