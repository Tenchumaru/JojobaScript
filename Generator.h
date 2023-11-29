#pragma once

#include "Iterator.h"

class Context;
class Expression;

class Generator {
public:
	Generator(std::shared_ptr<Context> outerContext, std::unique_ptr<Expression> const& targetExpression, std::vector<std::pair<std::string, std::string>> const& ids, std::unique_ptr<Expression> const& sourceExpression) : targetExpression(targetExpression), iterator(outerContext, ids, sourceExpression) {}
	Generator(Generator&&) = default;
	~Generator() = default;
	Value operator++();

private:
	std::unique_ptr<Expression> const& targetExpression;
	Iterator iterator;
};
