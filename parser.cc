#include "pch.h"
#include "parser.h"
#include "Statement.h"
#include "yy.h"

extern FILE* yyin;
extern int yylex();

namespace {
	std::unique_ptr<FunctionStatement> program;
	std::string parserError;

	void yyerror(char const* message) {
		parserError = message;
	}
}

#include "parser.inl"

std::string ParseFile(FILE* inputFile, std::unique_ptr<FunctionStatement>& program_) {
	yyin = inputFile;
	if (yyparse()) {
		return std::move(parserError);
	}
	program_ = std::move(program);
	return {};
}
