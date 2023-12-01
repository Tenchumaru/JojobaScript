#pragma once

#include "Value.h.inl"

bool AsBoolean(Value const& value);

class ValueReference {
public:
	ValueReference() = default; // TODO:  I suspect this is illegal due to the reference member.
	ValueReference(ValueReference const&) = delete;
	ValueReference(ValueReference&&) = default;
	ValueReference(Value& reference) : reference(&reference) {}
	ValueReference(std::string& string, std::int64_t index) : string(&string), index(index) {}
	~ValueReference() = default;
	operator Value();
	void operator=(Value const& value);
	template<typename T>
	bool holds_alternative() const { return internal_holds_alternative<T>(); }
	template<>
	bool holds_alternative<std::string>() const { return string || internal_holds_alternative<std::string>(); }
	template<typename T>
	T& get() { return std::get<T>(*reference); }

private:
	Value* reference = nullptr;
	std::string* string = nullptr;
	std::optional<int64_t> index;

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
	bool holds_alternative(ValueReference const& indexer) { return indexer.holds_alternative<T>(); }
	template<typename T>
	T& get(ValueReference& indexer) { return indexer.get<T>(); }
}
