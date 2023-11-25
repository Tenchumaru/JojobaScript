#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

using Value = std::variant<nullptr_t, bool, std::int64_t, double, std::string>;

bool AsBoolean(Value const& value);

class Context {
public:
	Context(std::shared_ptr<Context> outerContext) : outerContext(std::move(outerContext)) {}
	Context(Context&&) = default;
	~Context() = default;
	void AddValue(std::string const& key, Value value);
	Value& GetReference(std::string const& key);
	Value GetValue(std::string const& key);
	void SetValue(std::string const& key, Value value);

private:
	std::unordered_map<std::string, Value> values;
	std::shared_ptr<Context> outerContext;
};
