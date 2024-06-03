#pragma once

#include "Value.h.inl"

class Awaitable {
public:
	Awaitable(void* handle, std::function<Value(void*)> fn) : handle(handle), fn(fn) {}
	Awaitable(Awaitable&&) = default;
	~Awaitable() = default;
	Value Await();

private:
	void* handle;
	std::function<Value(void*)> fn;
};
