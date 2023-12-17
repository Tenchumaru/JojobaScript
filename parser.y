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

%token AS BREAK CASE CATCH CONTINUE DEC DEFAULT DO ELSE FALLTHROUGH FINALLY FOR FROM FUNCTION IF IMPORT IN INC RETURN SWITCH THROW TRY UNTIL WHILE YIELD
%token <assignment> ASSIGNMENT
%token <boolean> BOOLEAN VAR
%token <id> ID STRING
%token <n> '!' '~'
%token <number> NUMBER
%nonassoc '?' ':'
%left AND OR
%left EQ NE LE GE '<' '>'
%left '+' '-'
%left '&' '|' '^'
%left ASR LSR SL
%left '*' '/' '%'
%precedence NEG /* negation:  unary minus, bit-wise complement, logical complement */
%right SS
%nonassoc AWAIT
%left '.' '['
%right ARROW
%type<block> block cases oelse
%type<block_pair> catch_finally
%type<boolean> di uw
%type<expr> bexpr cexpr expr iexpr lexpr oexpr
%type<expr_list> expr_list lexpr_list oexpr_list
%type<for_clause> for_clause for_initializer
%type<for_clauses> condition_list for_clauses for_initializers ofor_clauses ofor_initializers oforexpr_list switch_list
%type<id> otype otype_list type type_list
%type<id_list> id_list imports oid_list
%type<idv_list> idv_list oidv_list
%type<if_fragment> elseif_statement
%type<if_fragments> oelseif_statements
%type<import> import
%type<initializer> initializer
%type<initializers> initializers
%type<kv_list> kv_list okv_list
%type<n> obreaks
%type<statement> case statement

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
| DO '{' block '}' uw expr { $$ = new DoStatement(std::move(*$3), $6, $5); delete $3; }
| FOR ofor_initializers ';' oforexpr_list ';' ofor_clauses '{' block '}' {
	$$ = new ForStatement(std::move(*$2), std::move(*$4), std::move(*$6), std::move(*$8));
	delete $2; delete $4; delete $6; delete $8;
}
| FOR id_list IN expr '{' block '}' {
	CheckUniqueness($2);
	$$ = new RangeForStatement(std::move(*$2), $4, std::move(*$6)); delete $2; delete $6;
}
| IF condition_list '{' block '}' oelseif_statements oelse {
	auto p = std::make_unique<IfStatement::Fragment>(std::move(*$2), std::move(*$4)); delete $2; delete $4;
	$6->emplace($6->begin(), std::move(p));
	$$ = new IfStatement(std::move(*$6), std::move(*$7)); delete $6; delete $7;
}
| RETURN oexpr {
	if (Yielding()) {
		throw std::runtime_error("cannot return and yield in the same function");
	}
	returnTypeStack.back() = ReturnType::Return;
	$$ = new ReturnStatement($2);
}
| SWITCH switch_list '{' cases '}' { $$ = new SwitchStatement(std::move(*$2), std::move(*$4)); delete $2; delete $4; }
| THROW { $$ = new ThrowStatement(nullptr); }
| THROW expr { $$ = new ThrowStatement($2); }
| TRY '{' block '}' catch_finally { $$ = new TryStatement(std::move(*$3), std::move($5->first), std::move($5->second)); delete $3; delete $5; }
| uw condition_list '{' block '}' { $$ = new WhileStatement(std::move(*$2), std::move(*$4), $1); delete $2; delete $4; }
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
| '@' lexpr_list ASSIGNMENT expr_list { $$ = new AssignmentStatement(std::move(*$2), $3, std::move(*$4)); delete $2; delete $4; }
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
| VAR initializer { $$ = new Statement::VarClause(std::move(*$2), $1); delete $2; }
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
| '(' lexpr_list ASSIGNMENT expr_list ')' { $$ = new Statement::AssignmentClause(std::move(*$2), $3, std::move(*$4)); delete $2; delete $4; }
| di lexpr { $$ = new Statement::DiClause($2, $1); }
| '@' expr { $$ = new Statement::ExpressionClause($2); }
;

oforexpr_list:
%empty { $$ = new std::vector<std::unique_ptr<Statement::Clause>>; }
| bexpr {
	$$ = new std::vector<std::unique_ptr<Statement::Clause>>;
	$$->emplace_back(std::make_unique<Statement::ExpressionClause>($1));
}
| for_clauses ',' bexpr { $1->emplace_back(std::make_unique<Statement::ExpressionClause>($3)); $$ = $1; }
;

switch_list:
expr {
	$$ = new std::vector<std::unique_ptr<Statement::Clause>>;
	$$->emplace_back(std::make_unique<Statement::ExpressionClause>($1));
}
| for_initializers ',' expr { $1->emplace_back(std::make_unique<Statement::ExpressionClause>($3)); $$ = $1; }
;

catch_finally:
CATCH '{' block '}' { $$ = new std::pair(std::move(*$3), std::vector<std::unique_ptr<Statement>>{}); delete $3; }
| CATCH '{' block '}' FINALLY '{' block '}' { $$ = new std::pair(std::move(*$3), std::move(*$7)); delete $3; delete $7; }
| FINALLY '{' block '}' { $$ = new std::pair(std::vector<std::unique_ptr<Statement>>{}, std::move(*$3)); delete $3; }
;

condition_list:
bexpr {
	$$ = new std::vector<std::unique_ptr<Statement::Clause>>;
	$$->emplace_back(std::make_unique<Statement::ExpressionClause>($1));
}
| for_initializers ',' bexpr { $1->emplace_back(std::make_unique<Statement::ExpressionClause>($3)); $$ = $1; }
;

oexpr:
%empty { $$ = nullptr; }
| expr
;

oelseif_statements:
%empty { $$ = new std::vector<std::unique_ptr<IfStatement::Fragment>>; }
| oelseif_statements elseif_statement { $1->emplace_back($2); $$ = $1; }
;

oelse:
%empty { $$ = new std::vector<std::unique_ptr<Statement>>; }
| ELSE '{' block '}' { $$ = $3; }
;

elseif_statement:
ELSE IF condition_list '{' block '}' { $$ = new IfStatement::Fragment(std::move(*$3), std::move(*$5)); delete $3; delete $5; }
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
'#' '(' id_list ')' otype { returnTypeStack.push_back({}); } '{' block '}' {
	$$ = new LambdaExpression(std::move(*$5), std::move(*$3), std::move(*$8), Yielding());
	delete $3; delete $5; delete $8;
}
| '#' '(' id_list ')' otype ARROW expr { $$ = new LambdaExpression(std::move(*$5), std::move(*$3), $7); delete $3; delete $5; }
| bexpr
| cexpr
;

cexpr:
'[' oexpr_list ']' { $$ = new ListExpression(std::move(*$2)); delete $2; }
| '[' expr FOR id_list IN expr ']' {
	CheckUniqueness($4);
	$$ = new ListComprehensionExpression($2, std::move(*$4), $6); delete $4;
}
| '(' expr FOR id_list IN expr ')' {
	CheckUniqueness($4);
	$$ = new GeneratorExpression($2, std::move(*$4), $6); delete $4;
}
| '{' expr ':' expr FOR id_list IN expr '}' {
	CheckUniqueness($6);
	$$ = new DictionaryComprehensionExpression($2, $4, std::move(*$6), $8); delete $6;
}
| '{' expr FOR id_list IN expr '}' {
	CheckUniqueness($4);
	$$ = new SetComprehensionExpression($2, std::move(*$4), $6); delete $4;
}
| '{' expr_list '}' { $$ = new SetExpression(std::move(*$2)); delete $2; }
| '{' okv_list '}' { $$ = new DictionaryExpression(std::move(*$2)); delete $2; }
| '#' '{' oidv_list '}' { $$ = new ObjectExpression(std::move(*$3)); delete $3; }
;

bexpr:
BOOLEAN { $$ = new LiteralExpression($1); }
| NUMBER { $$ = new LiteralExpression(std::move(*$1)); delete $1; }
| STRING { $$ = new LiteralExpression(std::move(*$1)); delete $1; }
| AWAIT expr { $$ = new AwaitExpression($2); }
| '-' expr %prec NEG { $$ = new UnaryExpression($2, '-', 1); }
| '~' expr %prec NEG { $$ = new UnaryExpression($2, '~', $1); }
| '!' expr %prec NEG { $$ = new UnaryExpression($2, '!', $1); }
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
| iexpr
;

iexpr:
iexpr '(' oexpr_list ')' { $$ = new InvocationExpression($1, std::move(*$3)); delete $3; }
| '(' expr ')' { $$ = $2; }
| lexpr
;

lexpr:
iexpr '.' ID { $$ = new DotExpression($1, std::move(*$3)); delete $3; }
| cexpr '.' ID { $$ = new DotExpression($1, std::move(*$3)); delete $3; }
| iexpr '[' expr ']' { $$ = new IndexExpression($1, $3); }
| iexpr '[' ':' expr ']' { $$ = new IndexExpression($1, new LiteralExpression(0), $4); }
| iexpr '[' expr ':' ']' { $$ = new IndexExpression($1, $3, nullptr); }
| iexpr '[' expr ':' expr ']' { $$ = new IndexExpression($1, $3, $5); }
| ID { $$ = new IdentifierExpression(std::move(*$1)); delete $1; }
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
