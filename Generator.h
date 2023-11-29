#pragma once

#include "Iterator.h"

class Context;
class Expression;
class Statement;

class Generator {
public:
	virtual ~Generator() = 0;
	virtual Value operator++() = 0;
};

class FunctionGenerator : public Generator {
public:
	FunctionGenerator(std::shared_ptr<Context> context, std::vector<std::unique_ptr<Statement>> const& statements) : context(context), statements(statements), it(this->statements.begin()) {}
	~FunctionGenerator() = default;
	Value operator++() override;

private:
	std::shared_ptr<Context> context;
	std::vector<std::unique_ptr<Statement>> const& statements;
	std::vector<std::unique_ptr<Statement>>::const_iterator it;
};

class IteratorGenerator : public Generator {
public:
	IteratorGenerator(std::shared_ptr<Context> outerContext, std::unique_ptr<Expression> const& targetExpression, std::vector<std::pair<std::string, std::string>> const& ids, std::unique_ptr<Expression> const& sourceExpression) : targetExpression(targetExpression), iterator(outerContext, ids, sourceExpression) {}
	IteratorGenerator(IteratorGenerator&&) = default;
	~IteratorGenerator() = default;
	Value operator++() override;

private:
	std::unique_ptr<Expression> const& targetExpression;
	Iterator iterator;
};
