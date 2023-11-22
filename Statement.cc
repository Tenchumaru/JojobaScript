#include "Statement.h"
#include "Expression.h"

FunctionStatement* FunctionStatement::program;

Statement::~Statement() {}

ForStatement::Clause::~Clause() {}
