#pragma once

#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include "Expression.h"

// TODO:  use std::unique_ptr for pointer private member variables.

class Statement {
public:
	Statement() = default;
	virtual ~Statement() = 0;

private:

};

class AssignmentStatement : public Statement {
public:
	AssignmentStatement(Expression* targetExpression, int assignment, Expression* sourceExpression) : targetExpression(targetExpression), sourceExpression(sourceExpression), assignment(assignment) {}
	~AssignmentStatement();

private:
	Expression* targetExpression;
	Expression* sourceExpression;
	int assignment;
};

class BreakStatement : public Statement {};

class ContinueStatement : public Statement {};

class DoStatement : public Statement {
public:
	DoStatement(std::vector<Statement*>&& statements, Expression* expression, bool isWhile) : statements(std::move(statements)), expression(expression), isWhile(isWhile) {}
	~DoStatement();

private:
	std::vector<Statement*> statements;
	Expression* expression;
	bool isWhile;
};

class ForStatement : public Statement {
public:
	class Clause {
	public:
		virtual ~Clause() = 0;
	};

	class AssignmentClause : public Clause {
	public:
		AssignmentClause(Expression* targetExpression, int assignment, Expression* sourceExpression) : targetExpression(targetExpression), sourceExpression(sourceExpression), assignment(assignment) {}
		~AssignmentClause();

	private:
		Expression* targetExpression;
		Expression* sourceExpression;
		int assignment;
	};

	class DiClause : public Clause {
	public:
		DiClause(Expression* expression, bool isIncrement) : expression(expression), isIncrement(isIncrement) {}
		~DiClause();

	private:
		Expression* expression;
		bool isIncrement;
	};

	class InvocationClause : public Clause {
	public:
		InvocationClause(Expression* expression) : expression(expression) {}
		~InvocationClause();

	private:
		Expression* expression;
	};

	ForStatement(std::vector<ForStatement::Clause*>&& initializerClauses, std::vector<Expression*>&& expressions, std::vector<ForStatement::Clause*>&& updaterClauses, std::vector<Statement*>&& statements) : initializerClauses(std::move(initializerClauses)), expressions(std::move(expressions)), updaterClauses(std::move(updaterClauses)), statements(std::move(statements)) {}
	~ForStatement();

private:
	std::vector<ForStatement::Clause*> initializerClauses;
	std::vector<Expression*> expressions;
	std::vector<ForStatement::Clause*> updaterClauses;
	std::vector<Statement*> statements;
};

class FunctionStatement : public Statement {
public:
	FunctionStatement(std::string&& name, std::string&& type, std::vector<std::pair<std::string, std::string>>&& parameters, std::vector<Statement*>&& statements) : name(std::move(name)), type(std::move(type)), parameters(std::move(parameters)), statements(std::move(statements)) {}
	~FunctionStatement();

private:
	std::string name;
	std::string type;
	std::vector<std::pair<std::string, std::string>> parameters;
	std::vector<Statement*> statements;
};

class IfStatement : public Statement {
public:
	IfStatement(Expression* expression, std::vector<Statement*>&& statements) : expression(expression), statements(std::move(statements)) {}
	IfStatement(IfStatement* ifStatement, std::vector<IfStatement*>&& elseIfStatements, std::vector<Statement*>&& elseStatements);
	~IfStatement();

private:
	Expression* expression;
	std::vector<Statement*> statements;
	std::vector<IfStatement*> elseIfStatements;
	std::vector<Statement*> elseStatements;
};

class ImportStatement : public Statement {
public:
	ImportStatement(std::string&& moduleName) : moduleName(std::move(moduleName)) {}
	ImportStatement(std::string&& moduleName, std::string&& alias) : moduleName(std::move(moduleName)), alias(std::move(alias)) {}
	ImportStatement(std::string&& moduleName, std::vector<std::pair<std::string, std::string>>&& pairs) : moduleName(std::move(moduleName)), pairs(std::move(pairs)) {}
	~ImportStatement() = default;

private:
	std::string moduleName;
	std::string alias;
	std::vector<std::pair<std::string, std::string>> pairs;
};

class IncrementStatement : public Statement {
public:
	IncrementStatement(Expression* expression, bool isIncrement) : expression(expression), isIncrement(isIncrement) {}
	~IncrementStatement();

private:
	Expression* expression;
	bool isIncrement;
};

class InvocationStatement : public Statement {
public:
	InvocationStatement(InvocationExpression* expression) : expression(expression) {}
	~InvocationStatement();

private:
	InvocationExpression* expression;
};

class RangeForStatement : public Statement {
public:
	RangeForStatement(std::vector<std::pair<std::string, std::string>>&& ids, Expression* expression, std::vector<Statement*>&& statements) : ids(std::move(ids)), expression(expression), statements(std::move(statements)) {}
	~RangeForStatement();

private:
	std::vector<std::pair<std::string, std::string>> ids;
	Expression* expression;
	std::vector<Statement*> statements;
};

class ReturnStatement : public Statement {
public:
	ReturnStatement(Expression* expression) : expression(expression) {}
	~ReturnStatement();

private:
	Expression* expression;
};

class SwitchStatement : public Statement {
public:
	class Case {
	public:
		Case(Expression* expression, std::vector<Statement*>&& statements) : expression(expression), statements(std::move(statements)) {}
		~Case();

	private:
		Expression* expression;
		std::vector<Statement*> statements;
	};

	SwitchStatement(Expression* expression, std::vector<SwitchStatement::Case>&& cases) : expression(expression), cases(std::move(cases)) {}
	~SwitchStatement();

private:
	Expression* expression;
	std::vector<SwitchStatement::Case> cases;
};

class VarStatement : public Statement {
public:
	VarStatement(std::vector<std::tuple<std::string, std::string, Expression*>>&& initializers) : initializers(std::move(initializers)) {}
	~VarStatement();

private:
	std::vector<std::tuple<std::string, std::string, Expression*>> initializers;
};

class WhileStatement : public Statement {
public:
	WhileStatement(Expression* expression, std::vector<Statement*>&& statements, bool isWhile) : expression(expression), statements(std::move(statements)), isWhile(isWhile) {}
	~WhileStatement();

private:
	Expression* expression;
	std::vector<Statement*> statements;
	bool isWhile;
};

class YieldStatement : public Statement {
public:
	YieldStatement(Expression* expression) : expression(expression) {}
	~YieldStatement();

private:
	Expression* expression;
};
