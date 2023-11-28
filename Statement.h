#pragma once

#include "Expression.h"

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

	class ExpressionClause : public Clause {
	public:
		ExpressionClause(Expression* expression) : expression(expression) {}
		ExpressionClause(ExpressionClause&&) = default;
		~ExpressionClause() = default;

	private:
		std::unique_ptr<Expression> expression;
	};

	class VarClause : public Clause {
	public:
		VarClause(std::tuple<std::string, std::string, std::unique_ptr<Expression>>&& initializer) : initializer(std::move(initializer)) {}
		VarClause(VarClause&&) = default;
		~VarClause() = default;

	private:
		std::tuple<std::string, std::string, std::unique_ptr<Expression>> initializer;
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

class BlockStatement : public Statement {
public:
	BlockStatement(std::vector<std::unique_ptr<Statement>>&& statements) : statements(std::move(statements)) {}
	virtual ~BlockStatement() = 0;

protected:
	std::vector<std::unique_ptr<Statement>> statements;
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

class DoStatement : public BlockStatement {
public:
	DoStatement(std::vector<std::unique_ptr<Statement>>&& statements, Expression* expression, bool isWhile) : BlockStatement(std::move(statements)), expression(expression), isWhile(isWhile) {}
	DoStatement(DoStatement&&) = default;
	~DoStatement() = default;

private:
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

class ForStatement : public BlockStatement {
public:
	ForStatement(std::vector<std::unique_ptr<Statement::Clause>>&& initializerClauses, std::vector<std::unique_ptr<Statement::Clause>>&& expressionClauses, std::vector<std::unique_ptr<Statement::Clause>>&& updaterClauses, std::vector<std::unique_ptr<Statement>>&& statements) : BlockStatement(std::move(statements)), initializerClauses(std::move(initializerClauses)), expressionClauses(std::move(expressionClauses)), updaterClauses(std::move(updaterClauses)) {}
	ForStatement(ForStatement&&) = default;
	~ForStatement() = default;

private:
	std::vector<std::unique_ptr<Statement::Clause>> initializerClauses;
	std::vector<std::unique_ptr<Statement::Clause>> expressionClauses;
	std::vector<std::unique_ptr<Statement::Clause>> updaterClauses;
};

class FunctionStatement : public BlockStatement {
public:
	static std::unique_ptr<FunctionStatement> program;

	FunctionStatement(std::string&& name, std::string&& type, std::vector<std::pair<std::string, std::string>>&& parameters, std::vector<std::unique_ptr<Statement>>&& statements) : BlockStatement(std::move(statements)), name(std::move(name)), type(std::move(type)), parameters(std::move(parameters)) {}
	FunctionStatement(FunctionStatement&&) = default;
	~FunctionStatement() = default;

private:
	std::string name;
	std::string type;
	std::vector<std::pair<std::string, std::string>> parameters;
};

class IfStatement : public BlockStatement {
public:
	class Fragment : public BlockStatement {
	public:
		Fragment(std::vector<std::unique_ptr<Statement::Clause>>&& initializerClauses, std::vector<std::unique_ptr<Statement>>&& statements) : BlockStatement(std::move(statements)), initializerClauses(std::move(initializerClauses)) {}
		Fragment(Fragment&&) = default;
		~Fragment() = default;

	private:
		std::vector<std::unique_ptr<Statement::Clause>> initializerClauses;
	};

	IfStatement(std::vector<std::unique_ptr<Fragment>>&& fragments, std::vector<std::unique_ptr<Statement>>&& elseStatements) : BlockStatement(std::move(elseStatements)), fragments(std::move(fragments)) {}
	IfStatement(IfStatement&&) = default;
	~IfStatement() = default;

private:
	std::vector<std::unique_ptr<Fragment>> fragments;
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

class RangeForStatement : public BlockStatement {
public:
	RangeForStatement(std::vector<std::pair<std::string, std::string>>&& ids, Expression* expression, std::vector<std::unique_ptr<Statement>>&& statements) : BlockStatement(std::move(statements)), ids(std::move(ids)), expression(expression) {}
	RangeForStatement(RangeForStatement&&) = default;
	~RangeForStatement() = default;

private:
	std::vector<std::pair<std::string, std::string>> ids;
	std::unique_ptr<Expression> expression;
};

class ReturnStatement : public Statement {
public:
	ReturnStatement() = default;
	ReturnStatement(Expression* expression) : expression(expression) {}
	ReturnStatement(ReturnStatement&&) = default;
	~ReturnStatement() = default;

private:
	std::unique_ptr<Expression> expression;
};

class SwitchStatement : public BlockStatement {
public:
	class DefaultCase : public BlockStatement {
	public:
		DefaultCase(std::vector<std::unique_ptr<Statement>>&& statements) : BlockStatement(std::move(statements)) {}
		DefaultCase(DefaultCase&&) = default;
		~DefaultCase() = default;
	};

	class Case : public DefaultCase {
	public:
		Case(Expression* expression, std::vector<std::unique_ptr<Statement>>&& statements) : DefaultCase(std::move(statements)), expression(expression) {}
		Case(Case&&) = default;
		~Case() = default;

	private:
		std::unique_ptr<Expression> expression;
	};

	SwitchStatement(std::vector<std::unique_ptr<Statement::Clause>>&& initializerClauses, std::vector<std::unique_ptr<Statement>>&& cases) : BlockStatement(std::move(cases)), initializerClauses(std::move(initializerClauses)) {}
	SwitchStatement(SwitchStatement&&) = default;
	~SwitchStatement() = default;

private:
	std::vector<std::unique_ptr<Statement::Clause>> initializerClauses;
};

class VarStatement : public Statement {
public:
	VarStatement(std::vector<std::tuple<std::string, std::string, std::unique_ptr<Expression>>>&& initializers) : initializers(std::move(initializers)) {}
	VarStatement(VarStatement&&) = default;
	~VarStatement() = default;

private:
	std::vector<std::tuple<std::string, std::string, std::unique_ptr<Expression>>> initializers;
};

class WhileStatement : public BlockStatement {
public:
	WhileStatement(std::vector<std::unique_ptr<Statement::Clause>>&& initializerClauses, std::vector<std::unique_ptr<Statement>>&& statements, bool isWhile) : BlockStatement(std::move(statements)), initializerClauses(std::move(initializerClauses)), isWhile(isWhile) {}
	WhileStatement(WhileStatement&&) = default;
	~WhileStatement() = default;

private:
	std::vector<std::unique_ptr<Statement::Clause>> initializerClauses;
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
