#pragma once

#include "Context.h"

class Statement;

class Function {
public:
	virtual ~Function() = 0;
	virtual Value Invoke(std::vector<Value> const& arguments) = 0;
};

class ScriptFunction : public Function {
public:
	ScriptFunction(std::vector<std::pair<std::string, std::string>> const& parameters, std::vector<std::unique_ptr<Statement>> const& statements, std::shared_ptr<Context> outerContext, bool yielding) : parameters(parameters), statements(statements), outerContext(std::move(outerContext)), yielding(yielding) {}
	ScriptFunction(ScriptFunction&&) = default;
	~ScriptFunction() = default;
	Value Invoke(std::vector<Value> const& arguments) override;

private:
	std::vector<std::pair<std::string, std::string>> const& parameters;
	std::vector<std::unique_ptr<Statement>> const& statements;
	std::shared_ptr<Context> outerContext;
	bool yielding;
};
