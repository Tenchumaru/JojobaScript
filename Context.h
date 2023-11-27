#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

class Dictionary;
class List;
class Set;
class Generator;

#include "Value.h.g.inl"

class Dictionary : public std::unordered_map<Value, Value> {};

class List : public std::vector<Value> {};

class Set : public std::unordered_set<Value> {};

bool AsBoolean(Value const& value);

class Context {
public:
	Context(std::shared_ptr<Context> outerContext) : outerContext(std::move(outerContext)) {}
	Context(Context&&) = default;
	~Context() = default;
	void AddValue(std::string const& key, Value const& value);
	Value& GetReference(std::string const& key);
	Value GetValue(std::string const& key);
	void SetValue(std::string const& key, Value const& value);

private:
	std::unordered_map<std::string, Value> values;
	std::shared_ptr<Context> outerContext;
};
