#include "pch.h"
#include "ValueReference.h"
#include "Value.cc.inl"

ValueReference::operator Value() {
	if (string) {
		return string->substr(index, endIndex - index);
	}
	return *reference;
}

void ValueReference::operator=(Value const& value) {
	if (reference) {
		*reference = value;
	} else if (string) {
		if (std::holds_alternative<std::string>(value)) {
			auto&& rhs = std::get<std::string>(value);
			*string = string->substr(0, index) + rhs + string->substr(endIndex);
		} else {
			throw std::runtime_error("cannot assign non-string to string");
		}
	} else if (list) {
		if (std::holds_alternative<std::shared_ptr<List>>(value)) {
			List const& rhs = *std::get<std::shared_ptr<List>>(value);
			size_t nsources = rhs.size();
			size_t ntargets = endIndex - index;
			if (nsources < ntargets) {
				list->erase(list->begin() + endIndex - ntargets + nsources, list->begin() + endIndex);
			} else if (ntargets < nsources) {
				list->insert(list->begin() + endIndex, nsources - ntargets, nullptr);
			}
			auto it = list->begin() + index;
			for (auto const& sourceValue : rhs) {
				*it++ = sourceValue;
			}
		} else {
			throw std::runtime_error("cannot assign non-list to list");
		}
	} else {
		throw std::logic_error("unexpected empty ValueReference");
	}
}

void ValueReference::AdjustIndices(std::int64_t adjustingIndex, std::int64_t adjustingEndIndex) {
	if (index + adjustingEndIndex >= endIndex) {
		throw std::runtime_error("index out of range");
	}
	endIndex = index + adjustingEndIndex;
	index += adjustingIndex;
}

bool ValueReference::get_IsIndexed() const {
	return list || string;
}

size_t ValueReference::get_Size() const {
	if (list || string) {
		return endIndex - index;
	}
	throw std::logic_error("unexpected ValueReference::Size invocation");
}
