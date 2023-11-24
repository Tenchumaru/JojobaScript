#pragma once

#include <memory>
#include <string>
#include <unordered_map>

using Value = int;

class Context {
public:
	Context(std::shared_ptr<Context> outerContext) : outerContext(std::move(outerContext)) {}
	Context(Context&&) = default;
	~Context() = default;
	void AddValue(std::string const& key, Value value);
	int GetValue(std::string const& key);
	void SetValue(std::string const& key, Value value);

private:
	std::unordered_map<std::string, Value> values;
	std::shared_ptr<Context> outerContext;
};
