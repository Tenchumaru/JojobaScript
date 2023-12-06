#include "pch.h"
#include "parser.h"
#include "Statement.h"
#include "yy.h"

extern FILE* yyin;
extern int yylex();
void yyerror(char const* message);
static std::unique_ptr<FunctionStatement> program;

#include "parser.inl"

bool ParseFile(FILE* inputFile, std::unique_ptr<FunctionStatement>& program_) {
	yyin = inputFile;
	if (yyparse()) {
		return false;
	}
	program_ = std::move(program);
	return true;
}
