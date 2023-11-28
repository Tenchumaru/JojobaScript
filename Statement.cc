#include "pch.h"
#include "Statement.h"

std::unique_ptr<FunctionStatement> FunctionStatement::program;

Statement::~Statement() {}

BlockStatement::~BlockStatement() {}

ForStatement::Clause::~Clause() {}
