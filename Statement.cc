#include "Statement.h"
#include "Expression.h"

std::vector<std::unique_ptr<Statement>> Statement::program;

Statement::~Statement() {}

ForStatement::Clause::~Clause() {}

IfStatement::IfStatement(IfStatement* ifStatement, std::vector<std::unique_ptr<IfStatement>>&& elseIfStatements, std::vector<std::unique_ptr<Statement>>&& elseStatements) : expression(std::move(ifStatement->expression)), statements(std::move(ifStatement->statements)), elseIfStatements(std::move(elseIfStatements)), elseStatements(std::move(elseStatements)) {
	delete ifStatement;
}
