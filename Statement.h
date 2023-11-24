#pragma once

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

enum class Assignment { AA = 1, ASRA, DA, SLA, LSRA, MA, OA, PA, SA, TA, XA };

enum class FunctionType { Unspecified, Standard, Asynchronous, Generator };

class Expression;

class Statement {
public:
	class Clause {
	public:
		virtual ~Clause() = 0;
	};

	class AssignmentClause : public Clause {
	public:
		AssignmentClause(Expression* targetExpression, Assignment assignment, Expression* sourceExpression) : targetExpression(targetExpression), sourceExpression(sourceExpression), assignment(assignment) {}
		AssignmentClause(std::tuple<std::string, std::string, std::unique_ptr<Expression>>&& initializer) : initializer(std::move(initializer)), assignment() {}
		AssignmentClause(AssignmentClause&&) = default;
		~AssignmentClause() = default;

	private:
		std::tuple<std::string, std::string, std::unique_ptr<Expression>> initializer;
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

	class ExpressionClause : public Clause {
	public:
		ExpressionClause(Expression* expression) : expression(expression) {}
		ExpressionClause(ExpressionClause&&) = default;
		~ExpressionClause() = default;

	private:
		std::unique_ptr<Expression> expression;
	};

	Statement() = default;
	virtual ~Statement() = 0;
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

class BreakStatement : public Statement {
public:
	BreakStatement(int nPrecedingBreaks) : nPrecedingBreaks(nPrecedingBreaks) {}
	BreakStatement(BreakStatement&&) = default;
	~BreakStatement() = default;

private:
	int nPrecedingBreaks;
};

class ContinueStatement : public Statement {
public:
	ContinueStatement(int nPrecedingBreaks) : nPrecedingBreaks(nPrecedingBreaks) {}
	ContinueStatement(ContinueStatement&&) = default;
	~ContinueStatement() = default;

private:
	int nPrecedingBreaks;
};

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

class ExpressionStatement : public Statement {
public:
	ExpressionStatement(Expression* expression) : expression(expression) {}
	ExpressionStatement(ExpressionStatement&&) = default;
	~ExpressionStatement() = default;

private:
	std::unique_ptr<Expression> expression;
};

class ForStatement : public Statement {
public:
	ForStatement(std::vector<std::unique_ptr<Statement::Clause>>&& initializerClauses, std::vector<std::unique_ptr<Statement::Clause>>&& expressionClauses, std::vector<std::unique_ptr<Statement::Clause>>&& updaterClauses, std::vector<std::unique_ptr<Statement>>&& statements) : initializerClauses(std::move(initializerClauses)), expressionClauses(std::move(expressionClauses)), updaterClauses(std::move(updaterClauses)), statements(std::move(statements)) {}
	ForStatement(ForStatement&&) = default;
	~ForStatement() = default;

private:
	std::vector<std::unique_ptr<Statement::Clause>> initializerClauses;
	std::vector<std::unique_ptr<Statement::Clause>> expressionClauses;
	std::vector<std::unique_ptr<Statement::Clause>> updaterClauses;
	std::vector<std::unique_ptr<Statement>> statements;
};

class FunctionStatement : public Statement {
public:
	static std::unique_ptr<FunctionStatement> program;

	FunctionStatement(std::string&& name, std::string&& type, std::vector<std::pair<std::string, std::string>>&& parameters, std::vector<std::unique_ptr<Statement>>&& statements) : name(std::move(name)), type(std::move(type)), parameters(std::move(parameters)), statements(std::move(statements)) {}
	FunctionStatement(FunctionStatement&&) = default;
	~FunctionStatement() = default;

private:
	std::string name;
	std::string type;
	std::vector<std::pair<std::string, std::string>> parameters;
	std::vector<std::unique_ptr<Statement>> statements;
};

class IfStatement : public Statement {
public:
	class Fragment {
	public:
		Fragment(std::vector<std::unique_ptr<Statement::Clause>>&& initializerClauses, std::vector<std::unique_ptr<Statement>>&& statements) : initializerClauses(std::move(initializerClauses)), statements(std::move(statements)) {}
		Fragment(Fragment&&) = default;
		~Fragment() = default;

	private:
		std::vector<std::unique_ptr<Statement::Clause>> initializerClauses;
		std::vector<std::unique_ptr<Statement>> statements;
	};

	IfStatement(std::vector<std::unique_ptr<Fragment>>&& fragments, std::vector<std::unique_ptr<Statement>>&& elseStatements) : fragments(std::move(fragments)), elseStatements(std::move(elseStatements)) {}
	IfStatement(IfStatement&&) = default;
	~IfStatement() = default;

private:
	std::vector<std::unique_ptr<Fragment>> fragments;
	std::vector<std::unique_ptr<Statement>> elseStatements;
};

class ImportStatement : public Statement {
public:
	ImportStatement(std::string&& moduleName) : moduleName(std::move(moduleName)) {}
	ImportStatement(std::string&& moduleName, std::string&& alias) : moduleName(std::move(moduleName)), alias(std::move(alias)) {}
	ImportStatement(std::string&& moduleName, std::vector<std::pair<std::string, std::string>>&& pairs) : moduleName(std::move(moduleName)), pairs(std::move(pairs)) {}
	ImportStatement(ImportStatement&&) = default;
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

	SwitchStatement(std::vector<std::unique_ptr<Statement::Clause>>&& initializerClauses, std::vector<SwitchStatement::Case>&& cases) : initializerClauses(std::move(initializerClauses)), cases(std::move(cases)) {}
	SwitchStatement(SwitchStatement&&) = default;
	~SwitchStatement() = default;

private:
	std::vector<std::unique_ptr<Statement::Clause>> initializerClauses;
	std::vector<SwitchStatement::Case> cases;
};

class VarStatement : public Statement {
public:
	VarStatement(std::vector<std::tuple<std::string, std::string, std::unique_ptr<Expression>>>&& initializers) : initializers(std::move(initializers)) {}
	VarStatement(VarStatement&&) = default;
	~VarStatement() = default;

private:
	std::vector<std::tuple<std::string, std::string, std::unique_ptr<Expression>>> initializers;
};

class WhileStatement : public Statement {
public:
	WhileStatement(std::vector<std::unique_ptr<Statement::Clause>>&& initializerClauses, std::vector<std::unique_ptr<Statement>>&& statements, bool isWhile) : initializerClauses(std::move(initializerClauses)), statements(std::move(statements)), isWhile(isWhile) {}
	WhileStatement(WhileStatement&&) = default;
	~WhileStatement() = default;

private:
	std::vector<std::unique_ptr<Statement::Clause>> initializerClauses;
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
