#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "Expression.h"
#include "JojobaScript.h"

class Statement {
public:
	Statement() = default;
	virtual ~Statement() = 0;
	static std::vector<std::unique_ptr<Statement>> program;

private:

};

class AssignmentStatement : public Statement {
public:
	AssignmentStatement(Expression* targetExpression, Assignment assignment, Expression* sourceExpression) : targetExpression(targetExpression), sourceExpression(sourceExpression), assignment(assignment) {}
	AssignmentStatement(AssignmentStatement&&) = default;
	~AssignmentStatement() = default;

private:
	std::unique_ptr<Expression> targetExpression;
	std::unique_ptr<Expression> sourceExpression;
	Assignment assignment;
};

class BreakStatement : public Statement {};

class ContinueStatement : public Statement {};

class DoStatement : public Statement {
public:
	DoStatement(std::vector<std::unique_ptr<Statement>>&& statements, Expression* expression, bool isWhile) : statements(std::move(statements)), expression(expression), isWhile(isWhile) {}
	DoStatement(DoStatement&&) = default;
	~DoStatement() = default;

private:
	std::vector<std::unique_ptr<Statement>> statements;
	std::unique_ptr<Expression> expression;
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
		AssignmentClause(Expression* targetExpression, Assignment assignment, Expression* sourceExpression) : targetExpression(targetExpression), sourceExpression(sourceExpression), assignment(assignment) {}
		AssignmentClause(AssignmentClause&&) = default;
		~AssignmentClause() = default;

	private:
		std::unique_ptr<Expression> targetExpression;
		std::unique_ptr<Expression> sourceExpression;
		Assignment assignment;
	};

	class DiClause : public Clause {
	public:
		DiClause(Expression* expression, bool isIncrement) : expression(expression), isIncrement(isIncrement) {}
		DiClause(DiClause&&) = default;
		~DiClause() = default;

	private:
		std::unique_ptr<Expression> expression;
		bool isIncrement;
	};

	class InvocationClause : public Clause {
	public:
		InvocationClause(Expression* expression) : expression(expression) {}
		InvocationClause(InvocationClause&&) = default;
		~InvocationClause() = default;

	private:
		std::unique_ptr<Expression> expression;
	};

	ForStatement(std::vector<std::unique_ptr<ForStatement::Clause>>&& initializerClauses, std::vector<std::unique_ptr<Expression>>&& expressions, std::vector<std::unique_ptr<ForStatement::Clause>>&& updaterClauses, std::vector<std::unique_ptr<Statement>>&& statements) : initializerClauses(std::move(initializerClauses)), expressions(std::move(expressions)), updaterClauses(std::move(updaterClauses)), statements(std::move(statements)) {}
	~ForStatement() = default;

private:
	std::vector<std::unique_ptr<ForStatement::Clause>> initializerClauses;
	std::vector<std::unique_ptr<Expression>> expressions;
	std::vector<std::unique_ptr<ForStatement::Clause>> updaterClauses;
	std::vector<std::unique_ptr<Statement>> statements;
};

class FunctionStatement : public Statement {
public:
	FunctionStatement(std::string&& name, std::string&& type, std::vector<std::pair<std::string, std::string>>&& parameters, std::vector<std::unique_ptr<Statement>>&& statements) : name(std::move(name)), type(std::move(type)), parameters(std::move(parameters)), statements(std::move(statements)) {}
	~FunctionStatement() = default;

private:
	std::string name;
	std::string type;
	std::vector<std::pair<std::string, std::string>> parameters;
	std::vector<std::unique_ptr<Statement>> statements;
};

class IfStatement : public Statement {
public:
	IfStatement(Expression* expression, std::vector<std::unique_ptr<Statement>>&& statements) : expression(expression), statements(std::move(statements)) {}
	IfStatement(IfStatement* ifStatement, std::vector<std::unique_ptr<IfStatement>>&& elseIfStatements, std::vector<std::unique_ptr<Statement>>&& elseStatements);
	IfStatement(IfStatement&&) = default;
	~IfStatement() = default;

private:
	std::unique_ptr<Expression> expression;
	std::vector<std::unique_ptr<Statement>> statements;
	std::vector<std::unique_ptr<IfStatement>> elseIfStatements;
	std::vector<std::unique_ptr<Statement>> elseStatements;
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
	IncrementStatement(IncrementStatement&&) = default;
	~IncrementStatement() = default;

private:
	std::unique_ptr<Expression> expression;
	bool isIncrement;
};

class InvocationStatement : public Statement {
public:
	InvocationStatement(InvocationExpression* expression) : expression(expression) {}
	InvocationStatement(InvocationStatement&&) = default;
	~InvocationStatement() = default;

private:
	std::unique_ptr<InvocationExpression> expression;
};

class RangeForStatement : public Statement {
public:
	RangeForStatement(std::vector<std::pair<std::string, std::string>>&& ids, Expression* expression, std::vector<std::unique_ptr<Statement>>&& statements) : ids(std::move(ids)), expression(expression), statements(std::move(statements)) {}
	RangeForStatement(RangeForStatement&&) = default;
	~RangeForStatement() = default;

private:
	std::vector<std::pair<std::string, std::string>> ids;
	std::unique_ptr<Expression> expression;
	std::vector<std::unique_ptr<Statement>> statements;
};

class ReturnStatement : public Statement {
public:
	ReturnStatement(Expression* expression) : expression(expression) {}
	ReturnStatement(ReturnStatement&&) = default;
	~ReturnStatement() = default;

private:
	std::unique_ptr<Expression> expression;
};

class SwitchStatement : public Statement {
public:
	class Case {
	public:
		Case(Expression* expression, std::vector<std::unique_ptr<Statement>>&& statements) : expression(expression), statements(std::move(statements)) {}
		Case(Case&&) = default;
		~Case() = default;

	private:
		std::unique_ptr<Expression> expression;
		std::vector<std::unique_ptr<Statement>> statements;
	};

	SwitchStatement(Expression* expression, std::vector<SwitchStatement::Case>&& cases) : expression(expression), cases(std::move(cases)) {}
	SwitchStatement(SwitchStatement&&) = default;
	~SwitchStatement() = default;

private:
	std::unique_ptr<Expression> expression;
	std::vector<SwitchStatement::Case> cases;
};

class VarStatement : public Statement {
public:
	VarStatement(std::vector<std::tuple<std::string, std::string, std::unique_ptr<Expression>>>&& initializers) : initializers(std::move(initializers)) {}
	~VarStatement() = default;

private:
	std::vector<std::tuple<std::string, std::string, std::unique_ptr<Expression>>> initializers;
};

class WhileStatement : public Statement {
public:
	WhileStatement(Expression* expression, std::vector<std::unique_ptr<Statement>>&& statements, bool isWhile) : expression(expression), statements(std::move(statements)), isWhile(isWhile) {}
	WhileStatement(WhileStatement&&) = default;
	~WhileStatement() = default;

private:
	std::unique_ptr<Expression> expression;
	std::vector<std::unique_ptr<Statement>> statements;
	bool isWhile;
};

class YieldStatement : public Statement {
public:
	YieldStatement(Expression* expression) : expression(expression) {}
	YieldStatement(YieldStatement&&) = default;
	~YieldStatement() = default;

private:
	std::unique_ptr<Expression> expression;
};
