#include "JojobaScript.h"

std::unique_ptr<FunctionStatement> FunctionStatement::program;

Statement::~Statement() {}

ForStatement::Clause::~Clause() {}
