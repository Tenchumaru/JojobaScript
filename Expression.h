#pragma once

#include <string>
#include <utility>
#include <vector>

class Expression {
public:
	virtual ~Expression() = 0;

private:

};

class AwaitExpression : public Expression {
public:
	AwaitExpression(Expression* expression) : expression(expression) {}
	~AwaitExpression();

private:
	Expression* expression;
};

class BinaryExpression : public Expression {
public:
	BinaryExpression(Expression* leftExpression, int operation, Expression* rightExpression) : leftExpression(leftExpression), operation(operation), rightExpression(rightExpression) {}
	~BinaryExpression();

private:
	Expression* leftExpression;
	int operation;
	Expression* rightExpression;
};

class DictionaryExpression : public Expression {
public:
	DictionaryExpression(std::vector<std::pair<Expression*, Expression*>>&& keyValuePairs) : keyValuePairs(std::move(keyValuePairs)) {}
	~DictionaryExpression();

private:
	std::vector<std::pair<Expression*, Expression*>> keyValuePairs;
};

class DictionaryComprehensionExpression : public Expression {
public:
	DictionaryComprehensionExpression(Expression* keyExpression, Expression* valueExpression, std::vector<std::pair<std::string, std::string>>&& ids, Expression* sourceExpression) : keyExpression(keyExpression), valueExpression(valueExpression), ids(std::move(ids)), sourceExpression(sourceExpression) {}
	~DictionaryComprehensionExpression();

private:
	Expression* keyExpression;
	Expression* valueExpression;
	std::vector<std::pair<std::string, std::string>> ids;
	Expression* sourceExpression;
};

class IdentifierExpression : public Expression {
public:
	IdentifierExpression(std::string&& id) : id(std::move(id)) {}
	~IdentifierExpression() = default;

private:
	std::string id;
};

class InvocationExpression : public Expression {
public:
	InvocationExpression(Expression* expression, std::vector<Expression*>&& arguments) : expression(expression), arguments(std::move(arguments)) {}
	~InvocationExpression();

private:
	Expression* expression;
	std::vector<Expression*> arguments;
};

class ListExpression : public Expression {
public:
	ListExpression(std::vector<Expression*>&& expressions) : expressions(std::move(expressions)) {}
	~ListExpression();

private:
	std::vector<Expression*> expressions;
};

class ListComprehensionExpression : public Expression {
public:
	ListComprehensionExpression(Expression* targetExpression, std::vector<std::pair<std::string, std::string>>&& ids, Expression* sourceExpression) : targetExpression(targetExpression), ids(std::move(ids)), sourceExpression(sourceExpression) {}
	~ListComprehensionExpression();

private:
	Expression* targetExpression;
	std::vector<std::pair<std::string, std::string>> ids;
	Expression* sourceExpression;
};

// TODO:  accept any numeric type by using a variant.
class NumericExpression : public Expression {
public:
	NumericExpression(int value) : value(value) {}
	~NumericExpression() = default;

private:
	int value;
};

class SetExpression : public Expression {
public:
	SetExpression(std::vector<Expression*>&& expressions) : expressions(std::move(expressions)) {}
	~SetExpression();

private:
	std::vector<Expression*> expressions;
};

class TernaryExpression : public Expression {
public:
	TernaryExpression(Expression* expression, Expression* trueExpression, Expression* falseExpression) : expression(expression), trueExpression(trueExpression), falseExpression(falseExpression) {}
	~TernaryExpression();

private:
	Expression* expression;
	Expression* trueExpression;
	Expression* falseExpression;
};

class UnaryExpression : public Expression {
public:
	UnaryExpression(Expression* expression, int operation) : expression(expression), operation(operation) {}
	~UnaryExpression();

private:
	Expression* expression;
	int operation;
};
