#pragma once

#include <memory>

class Context {
public:
	Context(std::shared_ptr<Context> outerContext) : outerContext(std::move(outerContext)) {}
	Context(Context&&) = default;
	~Context() = default;

private:
	std::shared_ptr<Context> outerContext;
};
