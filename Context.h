#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

class Dictionary;
class List;

using Value = std::variant<nullptr_t, bool, std::int64_t, double, std::string, std::shared_ptr<List>, std::shared_ptr<Dictionary>>;

class Dictionary : public std::unordered_map<Value, Value> {};

class List : public std::vector<Value> {};

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
