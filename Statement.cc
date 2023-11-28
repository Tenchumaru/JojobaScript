#include "pch.h"
#include "JojobaScript.h"

std::unique_ptr<FunctionStatement> FunctionStatement::program;

Statement::~Statement() {}

BlockStatement::~BlockStatement() {}

ForStatement::Clause::~Clause() {}
