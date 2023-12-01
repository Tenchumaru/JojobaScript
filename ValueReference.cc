#include "pch.h"
#include "ValueReference.h"
#include "Value.cc.inl"

ValueReference::operator Value() {
	if (string) {
		if (index.has_value()) {
			return Value(std::string(1, (*string)[index.value()]));
		}
		return Value(*string);
	}
	return *reference;
}

void ValueReference::operator=(Value const& value) {
	if (reference) {
		*reference = value;
	} else if (string) {
		if (std::holds_alternative<std::string>(value)) {
			auto&& rhs = std::get<std::string>(value);
			if (index.has_value()) {
				*string = string->substr(0, index.value()) + rhs + string->substr(index.value() + 1);
			} else {
				*string = rhs;
			}
		} else {
			throw std::runtime_error("cannot assign non-string to string");
		}
	}
}
