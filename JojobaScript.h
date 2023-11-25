#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Context.h"
#include "Expression.h"
#include "Statement.h"

class Type {
public:
	Type(std::string&& type) : type(std::move(type)) {}
	Type(Type&&) = default;
	~Type() = default;

private:
	std::string type;
};

class Function {
public:
	Function(std::vector<std::string> const& parameters, std::shared_ptr<Context> outerContext) : parameters(parameters), outerContext(std::move(outerContext)) {}
	Function(Function&&) = default;
	~Function() = default;

private:
	std::vector<std::string> parameters;
	std::shared_ptr<Context> outerContext;
};
