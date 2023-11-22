#pragma once

#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

class Statement;

class Expression {
public:
	virtual ~Expression() = 0;

private:

};

class AwaitExpression : public Expression {
public:
	AwaitExpression(Expression* expression) : expression(expression) {}
	AwaitExpression(AwaitExpression&&) = default;
	~AwaitExpression() = default;

private:
	std::unique_ptr<Expression> expression;
};

class BinaryExpression : public Expression {
public:
	BinaryExpression(Expression* leftExpression, int operation, Expression* rightExpression) : leftExpression(leftExpression), operation(operation), rightExpression(rightExpression) {}
	BinaryExpression(BinaryExpression&&) = default;
	~BinaryExpression() = default;

private:
	std::unique_ptr<Expression> leftExpression;
	int operation;
	std::unique_ptr<Expression> rightExpression;
};

class BooleanExpression : public Expression {
public:
	BooleanExpression(bool value) : value(value) {}
	BooleanExpression(BooleanExpression&&) = default;
	~BooleanExpression() = default;

private:
	bool value;
};

class DictionaryExpression : public Expression {
public:
	DictionaryExpression(std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>>&& keyValuePairs) : keyValuePairs(std::move(keyValuePairs)) {}
	DictionaryExpression(DictionaryExpression&&) = default;
	~DictionaryExpression() = default;

private:
	std::vector<std::pair<std::unique_ptr<Expression>, std::unique_ptr<Expression>>> keyValuePairs;
};

class DictionaryComprehensionExpression : public Expression {
public:
	DictionaryComprehensionExpression(Expression* keyExpression, Expression* valueExpression, std::vector<std::pair<std::string, std::string>>&& ids, Expression* sourceExpression) : keyExpression(keyExpression), valueExpression(valueExpression), ids(std::move(ids)), sourceExpression(sourceExpression) {}
	DictionaryComprehensionExpression(DictionaryComprehensionExpression&&) = default;
	~DictionaryComprehensionExpression() = default;

private:
	std::unique_ptr<Expression> keyExpression;
	std::unique_ptr<Expression> valueExpression;
	std::vector<std::pair<std::string, std::string>> ids;
	std::unique_ptr<Expression> sourceExpression;
};

class IdentifierExpression : public Expression {
public:
	IdentifierExpression(std::string&& id) : id(std::move(id)) {}
	IdentifierExpression(IdentifierExpression&&) = default;
	~IdentifierExpression() = default;

private:
	std::string id;
};

class InvocationExpression : public Expression {
public:
	InvocationExpression(Expression* expression, std::vector<std::unique_ptr<Expression>>&& arguments) : expression(std::move(expression)), arguments(std::move(arguments)) {}
	InvocationExpression(InvocationExpression&&) = default;
	~InvocationExpression() = default;

private:
	std::unique_ptr<Expression> expression;
	std::vector<std::unique_ptr<Expression>> arguments;
};

class LambdaExpression : public Expression {
public:
	LambdaExpression(std::vector<std::pair<std::string, std::string>>&& ids, std::vector<std::unique_ptr<Statement>>&& statements) : ids(std::move(ids)), statements(std::move(statements)) {}
	LambdaExpression(std::vector<std::pair<std::string, std::string>>&& ids, Expression* expression) : ids(std::move(ids)), expression(expression) {}
	LambdaExpression(LambdaExpression&&) = default;
	~LambdaExpression() = default;

private:
	std::vector<std::pair<std::string, std::string>> ids;
	std::vector<std::unique_ptr<Statement>> statements;
	Expression* expression = nullptr;
};

class ListExpression : public Expression {
public:
	ListExpression(std::vector<std::unique_ptr<Expression>>&& expressions) : expressions(std::move(expressions)) {}
	ListExpression(ListExpression&&) = default;
	~ListExpression() = default;

private:
	std::vector<std::unique_ptr<Expression>> expressions;
};

class ListComprehensionExpression : public Expression {
public:
	ListComprehensionExpression(Expression* targetExpression, std::vector<std::pair<std::string, std::string>>&& ids, Expression* sourceExpression) : targetExpression(targetExpression), ids(std::move(ids)), sourceExpression(sourceExpression) {}
	ListComprehensionExpression(ListComprehensionExpression&&) = default;
	~ListComprehensionExpression() = default;

private:
	std::unique_ptr<Expression> targetExpression;
	std::vector<std::pair<std::string, std::string>> ids;
	std::unique_ptr<Expression> sourceExpression;
};

class NumericExpression : public Expression {
public:
	NumericExpression(std::variant<std::int64_t, double>&& value) : value(std::move(value)) {}
	NumericExpression(NumericExpression&&) = default;
	~NumericExpression() = default;

private:
	std::variant<std::int64_t, double> value;
};

class SetExpression : public Expression {
public:
	SetExpression(std::vector<std::unique_ptr<Expression>>&& expressions) : expressions(std::move(expressions)) {}
	SetExpression(SetExpression&&) = default;
	~SetExpression() = default;

private:
	std::vector<std::unique_ptr<Expression>> expressions;
};

class TernaryExpression : public Expression {
public:
	TernaryExpression(Expression* expression, Expression* trueExpression, Expression* falseExpression) : expression(expression), trueExpression(trueExpression), falseExpression(falseExpression) {}
	TernaryExpression(TernaryExpression&&) = default;
	~TernaryExpression() = default;

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

private:
	std::unique_ptr<Expression> expression;
	int operation;
};
