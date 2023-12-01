#pragma once

#include "Value.h.inl"

bool AsBoolean(Value const& value);

class Context {
public:
	Context(std::shared_ptr<Context> outerContext) : outerContext(std::move(outerContext)) {}
	Context(Context&&) = default;
	~Context() = default;
	void AddValue(std::string const& key, Value const& value, bool isConstant);
	Value& GetReference(std::string const& key);
	Value GetValue(std::string const& key);
	bool IsConstant(std::string const& key);

private:
	std::unordered_map<std::string, std::pair<Value, bool>> values;
	std::shared_ptr<Context> outerContext;
};
