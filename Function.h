#pragma once

#include "Context.h"

class Statement;

class Function {
public:
	Function(std::vector<std::pair<std::string, std::string>> const& parameters, std::vector<std::unique_ptr<Statement>> const& statements, std::shared_ptr<Context> outerContext, bool yielding) : parameters(parameters), statements(statements), outerContext(std::move(outerContext)), yielding(yielding) {}
	Function(Function&&) = default;
	~Function() = default;
	Value Invoke(std::vector<Value>&& arguments);

private:
	std::vector<std::pair<std::string, std::string>> const& parameters;
	std::vector<std::unique_ptr<Statement>> const& statements;
	std::shared_ptr<Context> outerContext;
	bool yielding;
};
