#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Expression.h"
#include "Statement.h"

class Context {
public:
	Context(std::shared_ptr<Context> outerContext) : outerContext(std::move(outerContext)) {}
	Context(Context&&) = default;
	~Context() = default;

private:
	std::shared_ptr<Context> outerContext;
};

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

extern std::vector<std::shared_ptr<Context>> contextStack;
