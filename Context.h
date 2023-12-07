#pragma once

#include "Value.h.inl"

bool AsBoolean(Value const& value);

class Context {
public:
	static std::unordered_map<std::string, std::shared_ptr<Context>> modules;

	Context(std::shared_ptr<Context> outerContext) : outerContext(std::move(outerContext)) {}
	Context(Context&&) = default;
	~Context() = default;
	void AddContext(std::string const& moduleName, std::shared_ptr<Context> context);
	void AddValue(std::string const& key, Value const& value, bool isConstant);
	Value& GetReference(std::string const& key);
	Value GetValue(std::string const& key);
	bool IsConstant(std::string const& key);
	void SetValue(std::string const& key, Value const& value);

private:
	std::unordered_map<std::string, std::pair<Value, bool>> values;
	std::shared_ptr<Context> outerContext;
};
