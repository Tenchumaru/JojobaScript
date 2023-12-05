#include "pch.h"
#include "parser.h"
#include "Statement.h"
#include "yy.h"

extern FILE* yyin;
static std::unique_ptr<FunctionStatement> program;

#include "parser.inl"

int ParseFile(char const* filePath, std::unique_ptr<FunctionStatement>& program_) {
	if (!filePath || !strcmp("-", filePath)) {
		yyin = stdin;
	} else if (fopen_s(&yyin, filePath, "rt")) {
		return -1;
	}
	if (yyparse()) {
		return 1;
	}
	program_ = std::move(program);
	return 0;
}
