#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "JojobaScript.h"

class Generator {
public:
	Generator(std::shared_ptr<Context> context, std::unique_ptr<Expression>& targetExpression, std::vector<std::pair<std::string, std::string>> const& ids, Value&& sourceValue) : context(context), targetExpression(targetExpression), ids(ids), sourceValue(std::move(sourceValue)) {}
	Generator(Generator&&) = default;
	~Generator() = default;
	Value operator++();

private:
	std::shared_ptr<Context> context;
	std::unique_ptr<Expression>& targetExpression;
	std::vector<std::pair<std::string, std::string>> const& ids;
	Value sourceValue;
};
