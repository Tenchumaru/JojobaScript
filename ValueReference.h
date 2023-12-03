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
	template<typename T>
	bool holds_alternative() const { return internal_holds_alternative<T>(); }
	template<>
	bool holds_alternative<std::shared_ptr<List>>() const { return list || internal_holds_alternative<std::shared_ptr<List>>(); }
	template<>
	bool holds_alternative<std::string>() const { return string || internal_holds_alternative<std::string>(); }
	template<typename T>
	T& get() { return std::get<T>(*reference); }
	template<>
	std::string& get<std::string>() { return string ? *string : std::get<std::string>(*reference); }
	void AdjustIndices(std::int64_t adjustingIndex, std::int64_t adjustingEndIndex);
	bool get_IsIndexed() const;
	__declspec(property(get = get_IsIndexed)) bool IsIndexed;
	size_t get_Size() const;
	__declspec(property(get = get_Size)) size_t Size;

private:
	List* list = nullptr;
	Value* reference = nullptr;
	std::string* string = nullptr;
	std::int64_t index = 0;
	std::int64_t endIndex = 0;

	template<typename T>
	bool internal_holds_alternative() const {
		if (!reference) {
			throw std::logic_error("unexpected null reference");
		}
		return std::holds_alternative<T>(*reference);
	}
};

namespace std {
	template<typename T>
	bool holds_alternative(ValueReference const& reference) { return reference.holds_alternative<T>(); }
	template<typename T>
	T& get(ValueReference& reference) { return reference.get<T>(); }
}
