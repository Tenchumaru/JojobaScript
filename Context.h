#pragma once

#include "Value.h.inl"

bool AsBoolean(Value const& value);

class Context {
public:
	Context(std::shared_ptr<Context> outerContext) : outerContext(std::move(outerContext)) {}
	Context(Context&&) = default;
	~Context() = default;
	void AddValue(std::string const& key, Value const& value);
	Value& GetReference(std::string const& key);
	Value GetValue(std::string const& key);

private:
	std::unordered_map<std::string, Value> values;
	std::shared_ptr<Context> outerContext;
};
