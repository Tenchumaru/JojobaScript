#pragma once

#include "ValueReference.h"

class Context;

std::int64_t AdjustIndex(std::int64_t index, size_t size);

class Statement;

class Expression {
public:
	virtual ~Expression() = 0;
	virtual ValueReference GetReference(std::shared_ptr<Context> context);
	virtual Value GetValue(std::shared_ptr<Context> context) = 0;
};

class AwaitExpression : public Expression {
public:
	AwaitExpression(Expression* expression) : expression(expression) {}
	AwaitExpression(AwaitExpression&&) = default;
	~AwaitExpression() = default;
	Value GetValue(std::shared_ptr<Context> context) override;

private:
	std::unique_ptr<Expression> expression;
};

class BinaryExpression : public Expression {
public:
	BinaryExpression(Expression* leftExpression, int operation, Expression* rightExpression) : leftExpression(leftExpression), operation(operation), rightExpression(rightExpression) {}
	BinaryExpression(BinaryExpression&&) = default;
	~BinaryExpression() = default;
	Value GetValue(std::shared_ptr<Context> context) override;

private:
	std::unique_ptr<Expression> leftExpression;
	int operation;
	std::unique_ptr<Expression> rightExpression;
};

class DictionaryExpression : public Expression {
public:
	DictionaryExpression(std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>>&& keyValuePairs) : keyValuePairs(std::move(keyValuePairs)) {}
	DictionaryExpression(DictionaryExpression&&) = default;
	~DictionaryExpression() = default;
	Value GetValue(std::shared_ptr<Context> context) override;

private:
	std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>> keyValuePairs;
};

class DictionaryComprehensionExpression : public Expression {
public:
	DictionaryComprehensionExpression(Expression* keyExpression, Expression* valueExpression, std::vector<std::pair<std::string, std::string>>&& ids, Expression* sourceExpression) : keyExpression(keyExpression), valueExpression(valueExpression), ids(std::move(ids)), sourceExpression(sourceExpression) {}
	DictionaryComprehensionExpression(DictionaryComprehensionExpression&&) = default;
	~DictionaryComprehensionExpression() = default;
	Value GetValue(std::shared_ptr<Context> outerContext) override;

private:
	std::unique_ptr<Expression> keyExpression;
	std::unique_ptr<Expression> valueExpression;
	std::vector<std::pair<std::string, std::string>> ids;
	std::unique_ptr<Expression> sourceExpression;
};

class DotExpression : public Expression {
public:
	DotExpression(Expression* expression, std::string&& id) : expression(expression), id(std::move(id)) {}
	DotExpression(DotExpression&&) = default;
	~DotExpression() = default;
	ValueReference GetReference(std::shared_ptr<Context> context) override;
	Value GetValue(std::shared_ptr<Context> context) override;

private:
	std::unique_ptr<Expression> expression;
	std::string id;
};

class GeneratorExpression : public Expression {
public:
	GeneratorExpression(Expression* targetExpression, std::vector<std::pair<std::string, std::string>>&& ids, Expression* sourceExpression) : targetExpression(targetExpression), ids(std::move(ids)), sourceExpression(sourceExpression) {}
	GeneratorExpression(GeneratorExpression&&) = default;
	~GeneratorExpression() = default;
	Value GetValue(std::shared_ptr<Context> context) override;

private:
	std::unique_ptr<Expression> targetExpression;
	std::vector<std::pair<std::string, std::string>> ids;
	std::unique_ptr<Expression> sourceExpression;
};

class IdentifierExpression : public Expression {
public:
	IdentifierExpression(std::string&& id) : id(std::move(id)) {}
	IdentifierExpression(IdentifierExpression&&) = default;
	~IdentifierExpression() = default;
	ValueReference GetReference(std::shared_ptr<Context> context) override;
	Value GetValue(std::shared_ptr<Context> context) override;
	bool IsConstant(std::shared_ptr<Context> context) const;

private:
	std::string id;
};

class IndexExpression : public Expression {
public:
	IndexExpression(Expression* indexedExpression, Expression* indexingExpression) : indexedExpression(indexedExpression), indexingExpression(indexingExpression), isRange(false) {}
	IndexExpression(Expression* indexedExpression, Expression* indexingExpression, Expression* indexingEndExpression) : indexedExpression(indexedExpression), indexingExpression(indexingExpression), indexingEndExpression(indexingEndExpression), isRange(true) {}
	IndexExpression(IndexExpression&&) = default;
	~IndexExpression() = default;
	ValueReference GetReference(std::shared_ptr<Context> context) override;
	Value GetValue(std::shared_ptr<Context> context) override;

private:
	std::unique_ptr<Expression> indexedExpression;
	std::unique_ptr<Expression> indexingExpression;
	std::unique_ptr<Expression> indexingEndExpression;
	bool isRange;

	std::pair<std::int64_t, std::int64_t> GetIndices(size_t size, std::shared_ptr<Context> context);
};

class InvocationExpression : public Expression {
public:
	InvocationExpression(Expression* expression, std::vector<std::unique_ptr<Expression>>&& arguments) : expression(expression), arguments(std::move(arguments)) {}
	InvocationExpression(InvocationExpression&&) = default;
	~InvocationExpression() = default;
	Value GetValue(std::shared_ptr<Context> context) override;

private:
	std::unique_ptr<Expression> expression;
	std::vector<std::unique_ptr<Expression>> arguments;
};

class LambdaExpression : public Expression {
public:
	LambdaExpression(std::string&& type, std::vector<std::pair<std::string, std::string>>&& parameters, Expression* expression) : LambdaExpression(std::move(type), std::move(parameters), std::move(Convert(expression)), false) {}
	LambdaExpression(std::string&& type, std::vector<std::pair<std::string, std::string>>&& parameters, std::vector<std::unique_ptr<Statement>>&& statements, bool yielding) : type(std::move(type)), parameters(std::move(parameters)), statements(std::move(statements)), yielding(yielding) {}
	LambdaExpression(LambdaExpression&&) = default;
	~LambdaExpression() = default;
	Value GetValue(std::shared_ptr<Context> context) override;

private:
	std::string type;
	std::vector<std::pair<std::string, std::string>> parameters;
	std::vector<std::unique_ptr<Statement>> statements;
	bool yielding;

	static std::vector<std::unique_ptr<Statement>> Convert(Expression* expression);
};

class ListExpression : public Expression {
public:
	ListExpression(std::vector<std::unique_ptr<Expression>>&& expressions) : expressions(std::move(expressions)) {}
	ListExpression(ListExpression&&) = default;
	~ListExpression() = default;
	Value GetValue(std::shared_ptr<Context> context) override;

private:
	std::vector<std::unique_ptr<Expression>> expressions;
};

class ListComprehensionExpression : public Expression {
public:
	ListComprehensionExpression(Expression* targetExpression, std::vector<std::pair<std::string, std::string>>&& ids, Expression* sourceExpression) : targetExpression(targetExpression), ids(std::move(ids)), sourceExpression(sourceExpression) {}
	ListComprehensionExpression(ListComprehensionExpression&&) = default;
	~ListComprehensionExpression() = default;
	Value GetValue(std::shared_ptr<Context> context) override;

private:
	std::unique_ptr<Expression> targetExpression;
	std::vector<std::pair<std::string, std::string>> ids;
	std::unique_ptr<Expression> sourceExpression;
};

class LiteralExpression : public Expression {
public:
	LiteralExpression(Value&& value) : value(std::move(value)) {}
	LiteralExpression(LiteralExpression&&) = default;
	~LiteralExpression() = default;
	Value GetValue(std::shared_ptr<Context> context) override;

private:
	Value value;
};

class SetExpression : public Expression {
public:
	SetExpression(std::vector<std::unique_ptr<Expression>>&& expressions) : expressions(std::move(expressions)) {}
	SetExpression(SetExpression&&) = default;
	~SetExpression() = default;
	Value GetValue(std::shared_ptr<Context> context) override;

private:
	std::vector<std::unique_ptr<Expression>> expressions;
};

class ObjectExpression : public Expression {
public:
	ObjectExpression(std::unordered_map<std::string, std::pair<std::unique_ptr<Expression>, std::string>>&& fields) : fields(std::move(fields)) {}
	ObjectExpression(ObjectExpression&&) = default;
	~ObjectExpression() = default;
	Value GetValue(std::shared_ptr<Context> context) override;

private:
	std::unordered_map<std::string, std::pair<std::unique_ptr<Expression>, std::string>> fields;
};

class SetComprehensionExpression : public Expression {
public:
	SetComprehensionExpression(Expression* targetExpression, std::vector<std::pair<std::string, std::string>>&& ids, Expression* sourceExpression) : targetExpression(targetExpression), ids(std::move(ids)), sourceExpression(sourceExpression) {}
	SetComprehensionExpression(SetComprehensionExpression&&) = default;
	~SetComprehensionExpression() = default;
	Value GetValue(std::shared_ptr<Context> context) override;

private:
	std::unique_ptr<Expression> targetExpression;
	std::vector<std::pair<std::string, std::string>> ids;
	std::unique_ptr<Expression> sourceExpression;
};

class TernaryExpression : public Expression {
public:
	TernaryExpression(Expression* expression, Expression* trueExpression, Expression* falseExpression) : expression(expression), trueExpression(trueExpression), falseExpression(falseExpression) {}
	TernaryExpression(TernaryExpression&&) = default;
	~TernaryExpression() = default;
	Value GetValue(std::shared_ptr<Context> context) override;

private:
	std::unique_ptr<Expression> expression;
	std::unique_ptr<Expression> trueExpression;
	std::unique_ptr<Expression> falseExpression;
};

class UnaryExpression : public Expression {
public:
	UnaryExpression(Expression* expression, int operation) : expression(expression), operation(operation) {}
	UnaryExpression(UnaryExpression&&) = default;
	~UnaryExpression() = default;
	Value GetValue(std::shared_ptr<Context> context) override;

private:
	std::unique_ptr<Expression> expression;
	int operation;
};
