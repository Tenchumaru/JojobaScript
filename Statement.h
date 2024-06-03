#pragma once

#include "Expression.h"

enum class Assignment { AA = 1, ASRA, DA, SLA, LSRA, MA, OA, PA, SA, TA, XA, Size };

enum class FunctionType { Unspecified, Standard, Asynchronous, Generator };

class Expression;

class Statement {
public:
	enum class RunResult { Next, Break, Continue, Fallthrough, Return, Throw, Yield };

	using RunResultValue = std::variant<size_t, Value>;

	class Clause {
	public:
		virtual ~Clause() = 0;
		virtual Value Run(std::shared_ptr<Context> context) const = 0;
	};

	class AssignmentClause : public Clause {
	public:
		AssignmentClause(Expression* targetExpression, Assignment assignment, Expression* sourceExpression);
		AssignmentClause(std::vector<std::unique_ptr<Expression>>&& targetExpressions, Assignment assignment, std::vector<std::unique_ptr<Expression>>&& sourceExpressions) : targetExpressions(std::move(targetExpressions)), sourceExpressions(std::move(sourceExpressions)), assignment(assignment) {}
		AssignmentClause(AssignmentClause&&) = default;
		~AssignmentClause() = default;
		Value Run(std::shared_ptr<Context> context) const override;

	private:
		std::vector<std::unique_ptr<Expression>> targetExpressions;
		std::vector<std::unique_ptr<Expression>> sourceExpressions;
		Assignment assignment;
	};

	class DiClause : public Clause {
	public:
		DiClause(Expression* expression, bool isIncrement) : expression(expression), isIncrement(isIncrement) {}
		DiClause(DiClause&&) = default;
		~DiClause() = default;
		Value Run(std::shared_ptr<Context> context) const override;

	private:
		std::unique_ptr<Expression> expression;
		bool isIncrement;
	};

	class ExpressionClause : public Clause {
	public:
		ExpressionClause(Expression* expression) : expression(expression) {}
		ExpressionClause(ExpressionClause&&) = default;
		~ExpressionClause() = default;
		Value Run(std::shared_ptr<Context> context) const override;

	private:
		std::unique_ptr<Expression> expression;
	};

	class VarClause : public Clause {
	public:
		VarClause(std::tuple<std::string, std::string, std::unique_ptr<Expression>>&& initializer, bool isConstant) : initializer(std::move(initializer)), isConstant(isConstant) {}
		VarClause(VarClause&&) = default;
		~VarClause() = default;
		Value Run(std::shared_ptr<Context> context) const override;

	private:
		std::tuple<std::string, std::string, std::unique_ptr<Expression>> initializer;
		bool isConstant;
	};

	Statement() = default;
	virtual ~Statement() = 0;
	virtual std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> context) const = 0;
};

class AssignmentStatement : public Statement {
public:
	AssignmentStatement(std::vector<std::unique_ptr<Expression>>&& targetExpressions, Assignment assignment, std::vector<std::unique_ptr<Expression>>&& sourceExpressions) : targetExpressions(std::move(targetExpressions)), sourceExpressions(std::move(sourceExpressions)), assignment(assignment) {}
	AssignmentStatement(AssignmentStatement&&) = default;
	~AssignmentStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> context) const override;

private:
	std::vector<std::unique_ptr<Expression>> targetExpressions;
	std::vector<std::unique_ptr<Expression>> sourceExpressions;
	Assignment assignment;
};

class BlockStatement : public Statement {
public:
	BlockStatement(std::vector<std::unique_ptr<Statement>>&& statements) : statements(std::move(statements)) {}
	virtual ~BlockStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> context) const override;
	bool Run(std::shared_ptr<Context> context, std::pair<RunResult, RunResultValue>& runResult, bool allowFallthrough = false) const;

protected:
	std::vector<std::unique_ptr<Statement>> statements;
};

class BreakStatement : public Statement {
public:
	BreakStatement(size_t nPrecedingBreaks) : nPrecedingBreaks(nPrecedingBreaks) {}
	BreakStatement(BreakStatement&&) = default;
	~BreakStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> context) const override;

private:
	size_t nPrecedingBreaks;
};

class ContinueStatement : public Statement {
public:
	ContinueStatement(size_t nPrecedingBreaks) : nPrecedingBreaks(nPrecedingBreaks) {}
	ContinueStatement(ContinueStatement&&) = default;
	~ContinueStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> context) const override;

private:
	size_t nPrecedingBreaks;
};

class DoStatement : public BlockStatement {
public:
	DoStatement(std::vector<std::unique_ptr<Statement>>&& statements, Expression* expression, bool isWhile) : BlockStatement(std::move(statements)), expression(expression), isWhile(isWhile) {}
	DoStatement(DoStatement&&) = default;
	~DoStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> outerContext) const override;

private:
	std::unique_ptr<Expression> expression;
	bool isWhile;
};

class ExpressionStatement : public Statement {
public:
	ExpressionStatement(Expression* expression) : expression(expression) {}
	ExpressionStatement(ExpressionStatement&&) = default;
	~ExpressionStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> context) const override;

private:
	std::unique_ptr<Expression> expression;
};

class FallthroughStatement : public Statement {
public:
	FallthroughStatement() = default;
	FallthroughStatement(FallthroughStatement&&) = default;
	~FallthroughStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> context) const override;
};

class ForStatement : public BlockStatement {
public:
	ForStatement(std::vector<std::unique_ptr<Statement::Clause>>&& initializerClauses, std::vector<std::unique_ptr<Statement::Clause>>&& expressionClauses, std::vector<std::unique_ptr<Statement::Clause>>&& updaterClauses, std::vector<std::unique_ptr<Statement>>&& statements) : BlockStatement(std::move(statements)), initializerClauses(std::move(initializerClauses)), expressionClauses(std::move(expressionClauses)), updaterClauses(std::move(updaterClauses)) {}
	ForStatement(ForStatement&&) = default;
	~ForStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> outerContext) const override;

private:
	std::vector<std::unique_ptr<Statement::Clause>> initializerClauses;
	std::vector<std::unique_ptr<Statement::Clause>> expressionClauses;
	std::vector<std::unique_ptr<Statement::Clause>> updaterClauses;
};

class FunctionStatement : public BlockStatement {
public:
	static std::unordered_map<std::string, std::unique_ptr<FunctionStatement>> programs;

	FunctionStatement(std::string&& name, std::string&& type, std::vector<std::pair<std::string, std::string>>&& parameters, std::vector<std::unique_ptr<Statement>>&& statements, bool yielding) : BlockStatement(std::move(statements)), name(std::move(name)), type(std::move(type)), parameters(std::move(parameters)), yielding(yielding) {}
	FunctionStatement(FunctionStatement&&) = default;
	~FunctionStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> context) const override;
	std::shared_ptr<Context> RunProgram() const;

private:
	std::string name;
	std::string type;
	std::vector<std::pair<std::string, std::string>> parameters;
	bool yielding;
};

class IfStatement : public BlockStatement {
public:
	class Fragment : public BlockStatement {
	public:
		Fragment(Statement* initializers, Expression* expression, std::vector<std::unique_ptr<Statement>>&& statements) : BlockStatement(std::move(statements)), initializers(initializers), expression(expression) {}
		Fragment(Fragment&&) = default;
		~Fragment() = default;
		std::shared_ptr<Context> IsMatch(std::shared_ptr<Context> outerContext) const;

	private:
		std::unique_ptr<Statement> initializers;
		std::unique_ptr<Expression> expression;
	};

	IfStatement(std::vector<std::unique_ptr<Fragment>>&& fragments, std::vector<std::unique_ptr<Statement>>&& elseStatements) : BlockStatement(std::move(elseStatements)), fragments(std::move(fragments)) {}
	IfStatement(IfStatement&&) = default;
	~IfStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> outerContext) const override;

private:
	std::vector<std::unique_ptr<Fragment>> fragments;
};

class ImportStatement : public Statement {
public:
	ImportStatement(std::string&& moduleName) : moduleName(std::move(Validate(std::move(moduleName)))) {}
	ImportStatement(std::string&& moduleName, std::string&& alias) : moduleName(std::move(Validate(std::move(moduleName)))), alias(std::move(alias)) {}
	ImportStatement(std::string&& moduleName, std::vector<std::pair<std::string, std::string>>&& pairs) : moduleName(std::move(Validate(std::move(moduleName)))), pairs(std::move(pairs)) {}
	ImportStatement(ImportStatement&&) = default;
	~ImportStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> context) const override;

private:
	std::string moduleName;
	std::string alias;
	std::vector<std::pair<std::string, std::string>> pairs;

	static std::string Validate(std::string&& moduleName);
};

class IncrementStatement : public Statement {
public:
	IncrementStatement(Expression* expression, bool isIncrement) : expression(expression), isIncrement(isIncrement) {}
	IncrementStatement(IncrementStatement&&) = default;
	~IncrementStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> context) const override;

private:
	std::unique_ptr<Expression> expression;
	bool isIncrement;
};

class RangeForStatement : public BlockStatement {
public:
	RangeForStatement(std::vector<std::pair<std::string, std::string>>&& ids, Expression* expression, std::vector<std::unique_ptr<Statement>>&& statements) : BlockStatement(std::move(statements)), ids(std::move(ids)), expression(expression) {}
	RangeForStatement(RangeForStatement&&) = default;
	~RangeForStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> outerContext) const override;

private:
	std::vector<std::pair<std::string, std::string>> ids;
	std::unique_ptr<Expression> expression;
};

class ReturnStatement : public Statement {
public:
	ReturnStatement(Expression* expression) : expression(expression) {}
	ReturnStatement(ReturnStatement&&) = default;
	~ReturnStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> context) const override;

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
		bool IsMatch(Value value, std::shared_ptr<Context> outerContext) const;

	private:
		std::unique_ptr<Expression> expression;
	};

	SwitchStatement(std::vector<std::unique_ptr<Statement::Clause>>&& initializerClauses, std::vector<std::unique_ptr<Statement>>&& cases) : BlockStatement(std::move(cases)), initializerClauses(std::move(initializerClauses)) {}
	SwitchStatement(SwitchStatement&&) = default;
	~SwitchStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> outerContext) const override;

private:
	std::vector<std::unique_ptr<Statement::Clause>> initializerClauses;
};

class ThrowStatement : public Statement {
public:
	ThrowStatement(Expression* expression) : expression(expression) {}
	ThrowStatement(ThrowStatement&&) = default;
	~ThrowStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> context) const override;

private:
	std::unique_ptr<Expression> expression;
};

class TryStatement : public BlockStatement {
public:
	TryStatement(std::vector<std::unique_ptr<Statement>>&& tryStatements, std::vector<std::unique_ptr<Statement>>&& catchStatements, std::vector<std::unique_ptr<Statement>>&& finallyStatements) : BlockStatement(std::move(tryStatements)), catchStatements(std::move(catchStatements)), finallyStatements(std::move(finallyStatements)) {}
	TryStatement(TryStatement&&) = default;
	~TryStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> outerContext) const override;

private:
	BlockStatement catchStatements;
	BlockStatement finallyStatements;
};

class VarStatement : public Statement {
public:
	VarStatement(std::vector<std::tuple<std::string, std::string, std::unique_ptr<Expression>>>&& initializers, bool isConstant) : initializers(std::move(initializers)), isConstant(isConstant) {}
	VarStatement(VarStatement&&) = default;
	~VarStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> context) const override;

private:
	std::vector<std::tuple<std::string, std::string, std::unique_ptr<Expression>>> initializers;
	bool isConstant;
};

class WhileStatement : public BlockStatement {
public:
	WhileStatement(std::vector<std::unique_ptr<Statement::Clause>>&& initializerClauses, std::vector<std::unique_ptr<Statement>>&& statements, bool isWhile) : BlockStatement(std::move(statements)), initializerClauses(std::move(initializerClauses)), isWhile(isWhile) {}
	WhileStatement(WhileStatement&&) = default;
	~WhileStatement() = default;
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> outerContext) const override;

private:
	std::vector<std::unique_ptr<Statement::Clause>> initializerClauses;
	bool isWhile;
};

class YieldStatement : public Statement {
public:
	YieldStatement(Expression* expression);
	YieldStatement(YieldStatement&&) = default;
	~YieldStatement();
	std::pair<RunResult, RunResultValue> Run(std::shared_ptr<Context> context) const override;

private:
	std::unique_ptr<Expression> expression;
	void* waitHandle;
};
