#include "Statement.h"
#include "Utilities.h"

Statement::~Statement() {}

AssignmentStatement::~AssignmentStatement() {
	Delete(targetExpression);
	Delete(sourceExpression);
}

DoStatement::~DoStatement() {
	DeleteElements(statements);
	Delete(expression);
}

ForStatement::~ForStatement() {
	DeleteElements(initializerClauses);
	DeleteElements(expressions);
	DeleteElements(updaterClauses);
	DeleteElements(statements);
}

ForStatement::Clause::~Clause() {}

ForStatement::AssignmentClause::~AssignmentClause() {
	Delete(targetExpression);
	Delete(sourceExpression);
}

ForStatement::DiClause::~DiClause() {
	Delete(expression);
}

ForStatement::InvocationClause::~InvocationClause() {
	Delete(expression);
}

FunctionStatement::~FunctionStatement() {
	DeleteElements(statements);
}

IfStatement::IfStatement(IfStatement* ifStatement, std::vector<IfStatement*>&& elseIfStatements, std::vector<Statement*>&& elseStatements) : expression(ifStatement->expression), statements(std::move(ifStatement->statements)), elseIfStatements(std::move(elseIfStatements)), elseStatements(std::move(elseStatements)) {
	delete ifStatement;
}

IfStatement::~IfStatement() {
	Delete(expression);
	DeleteElements(statements);
	DeleteElements(elseIfStatements);
	DeleteElements(elseStatements);
}

IncrementStatement::~IncrementStatement() {
	Delete(expression);
}

InvocationStatement::~InvocationStatement() {
	Delete(expression);
}

RangeForStatement::~RangeForStatement() {
	Delete(expression);
	DeleteElements(statements);
}

ReturnStatement::~ReturnStatement() {
	Delete(expression);
}

SwitchStatement::~SwitchStatement() {
	Delete(expression);
}

SwitchStatement::Case::~Case() {
	Delete(expression);
	DeleteElements(statements);
}

VarStatement::~VarStatement() {
	std::for_each(initializers.begin(), initializers.end(), [](auto& tuple) { delete std::get<2>(tuple); });
	initializers.clear();
}

WhileStatement::~WhileStatement() {
	Delete(expression);
	DeleteElements(statements);
}

YieldStatement::~YieldStatement() {
	Delete(expression);
}
