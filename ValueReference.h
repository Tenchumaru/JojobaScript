#pragma once

#include "Value.h.inl"

bool AsBoolean(Value const& value);

class ValueReference {
public:
	ValueReference(ValueReference const&) = delete;
	ValueReference(ValueReference&&) = default;
	ValueReference(Value& reference) : reference(&reference) {}
	ValueReference(List& list, std::int64_t index, std::int64_t endIndex) : list(&list), index(index), endIndex(endIndex) {}
	ValueReference(std::string& string, std::int64_t index, std::int64_t endIndex) : string(&string), index(index), endIndex(endIndex) {}
	~ValueReference() = default;
	operator Value();
	void operator=(Value const& value);
	void operator&=(Value const& value);
	void operator>>=(Value const& value);
	void operator/=(Value const& value);
	void operator<<=(Value const& value);
	void operator%=(Value const& value);
	void operator|=(Value const& value);
	void operator+=(Value const& value);
	void operator-=(Value const& value);
	void operator*=(Value const& value);
	void operator^=(Value const& value);
	void AdjustIndices(std::int64_t adjustingIndex, std::int64_t adjustingEndIndex);
	template<typename T>
	T& get() {
		if (!reference) {
			throw std::logic_error("unexpected null reference");
		}
		return std::get<T>(*reference);
	}
	bool get_IsIndexed() const;
	size_t get_Size() const;
	template<typename T>
	bool holds_alternative() const { return reference && std::holds_alternative<T>(*reference); }
	__declspec(property(get = get_IsIndexed)) bool IsIndexed;
	__declspec(property(get = get_Size)) size_t Size;

private:
	List* list = nullptr;
	Value* reference = nullptr;
	std::string* string = nullptr;
	std::int64_t index = 0;
	std::int64_t endIndex = 0;
};

namespace std {
	template<typename T>
	bool holds_alternative(ValueReference const& reference) { return reference.holds_alternative<T>(); }
	template<typename T>
	T& get(ValueReference& reference) { return reference.get<T>(); }
}

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
