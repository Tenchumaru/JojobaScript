#include "pch.h"
#include "ValueReference.h"
#include "FiberRunner.h"

ValueReference::operator Value() {
	if (list) {
		auto partialList = std::make_shared<List>();
		for (std::int64_t i = index; i < endIndex; ++i) {
			partialList->push_back((*list)[i]);
		}
		return partialList;
	} else if (string) {
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

void ValueReference::operator&=(Value const& value) {
	if (holds_alternative<std::int64_t>() && std::holds_alternative<std::int64_t>(value)) {
		get<std::int64_t>() &= std::get<std::int64_t>(value);
	} else {
		throw std::runtime_error("cannot perform integral compound assignment on a non-integral value");
	}
}

void ValueReference::operator>>=(Value const& value) {
	if (holds_alternative<std::int64_t>() && std::holds_alternative<std::int64_t>(value)) {
		get<std::int64_t>() &= std::get<std::int64_t>(value);
	} else {
		throw std::runtime_error("cannot perform integral compound assignment on a non-integral value");
	}
}

void ValueReference::operator/=(Value const& value) {
	if (holds_alternative<std::int64_t>() && std::holds_alternative<std::int64_t>(value)) {
		auto rhs = std::get<std::int64_t>(value);
		if (rhs != 0) {
			get<std::int64_t>() /= rhs;
		} else {
			throw std::runtime_error("integer division by zero");
		}
	} else if (holds_alternative<double>() && std::holds_alternative<double>(value)) {
		auto rhs = std::get<double>(value);
		if (rhs != 0) {
			get<double>() /= rhs;
		} else {
			throw std::runtime_error("real division by zero");
		}
	} else {
		throw std::runtime_error("cannot perform numeric compound assignment on a non-numeric value");
	}
}

void ValueReference::operator<<=(Value const& value) {
	if (holds_alternative<std::int64_t>() && std::holds_alternative<std::int64_t>(value)) {
		get<std::int64_t>() &= std::get<std::int64_t>(value);
	} else {
		throw std::runtime_error("cannot perform integral compound assignment on a non-integral value");
	}
}

void ValueReference::operator%=(Value const& value) {
	if (holds_alternative<std::int64_t>() && std::holds_alternative<std::int64_t>(value)) {
		get<std::int64_t>() &= std::get<std::int64_t>(value);
	} else {
		throw std::runtime_error("cannot perform integral compound assignment on a non-integral value");
	}
}

void ValueReference::operator|=(Value const& value) {
	if (holds_alternative<std::int64_t>() && std::holds_alternative<std::int64_t>(value)) {
		get<std::int64_t>() &= std::get<std::int64_t>(value);
	} else {
		throw std::runtime_error("cannot perform integral compound assignment on a non-integral value");
	}
}

void ValueReference::operator+=(Value const& value) {
	if (list) {
		if (std::holds_alternative<std::shared_ptr<List>>(value)) {
			List const& sourceList = *std::get<std::shared_ptr<List>>(value);
			list->insert(list->begin() + endIndex, sourceList.begin(), sourceList.end());
		} else {
			list->insert(list->begin() + endIndex, value);
		}
	} else if (string) {
		if (std::holds_alternative<std::string>(value)) {
			std::string const& sourceString = std::get<std::string>(value);
			string->insert(string->begin() + endIndex, sourceString.begin(), sourceString.end());
		} else {
			throw std::runtime_error("cannot perform additive string assignment with non-string value");
		}
	} else if (holds_alternative<std::int64_t>() && std::holds_alternative<std::int64_t>(value)) {
		get<std::int64_t>() += std::get<std::int64_t>(value);
	} else if (holds_alternative<double>() && std::holds_alternative<double>(value)) {
		get<double>() += std::get<double>(value);
	} else if (holds_alternative<double>() && std::holds_alternative<std::int64_t>(value)) {
		get<double>() += std::get<std::int64_t>(value);
	} else if (holds_alternative<std::string>() && std::holds_alternative<std::string>(value)) {
		get<std::string>() += std::get<std::string>(value);
	} else if (holds_alternative<std::shared_ptr<List>>()) {
		List& targetList = *get<std::shared_ptr<List>>();
		if (std::holds_alternative<std::shared_ptr<List>>(value)) {
			List const& sourceList = *std::get<std::shared_ptr<List>>(value);
			targetList.insert(targetList.end(), sourceList.begin(), sourceList.end());
		} else {
			targetList.insert(targetList.end(), value);
		}
	} else {
		throw std::runtime_error("cannot perform additive assignment");
	}
}

void ValueReference::operator-=(Value const& value) {
	if (holds_alternative<std::int64_t>() && std::holds_alternative<std::int64_t>(value)) {
		get<std::int64_t>() -= std::get<std::int64_t>(value);
	} else if (holds_alternative<double>() && std::holds_alternative<double>(value)) {
		get<double>() -= std::get<double>(value);
	} else if (holds_alternative<double>() && std::holds_alternative<std::int64_t>(value)) {
		get<double>() -= std::get<std::int64_t>(value);
	} else {
		throw std::runtime_error("cannot perform numeric compound assignment on a non-numeric value");
	}
}

void ValueReference::operator*=(Value const& value) {
	if (list && std::holds_alternative<std::int64_t>(value)) {
		std::int64_t n = std::get<std::int64_t>(value);
		if (n < 0) {
			throw std::runtime_error("cannot multiply list by negative value");
		} else if (n == 0) {
			list->clear();
		} else {
			// Create a copy in case the source list is the target list.
			List sourceList;
			for (std::int64_t i = index; i < endIndex; ++i) {
				sourceList.push_back((*list)[i]);
			}
			while (--n) {
				list->insert(list->begin() + endIndex, sourceList.begin(), sourceList.end());
			}
		}
	} else if (string && std::holds_alternative<std::int64_t>(value)) {
		std::int64_t n = std::get<std::int64_t>(value);
		if (n < 0) {
			throw std::runtime_error("cannot multiply string by negative value");
		} else if (n == 0) {
			string->clear();
		} else {
			// Create a copy in case the source string is the target string.
			std::string sourceString = string->substr(index, endIndex - index);
			while (--n) {
				string->insert(string->begin() + endIndex, sourceString.begin(), sourceString.end());
			}
		}
	} else if (holds_alternative<std::int64_t>() && std::holds_alternative<std::int64_t>(value)) {
		get<std::int64_t>() *= std::get<std::int64_t>(value);
	} else if (holds_alternative<double>() && std::holds_alternative<double>(value)) {
		get<double>() *= std::get<double>(value);
	} else if (holds_alternative<double>() && std::holds_alternative<std::int64_t>(value)) {
		get<double>() *= std::get<std::int64_t>(value);
	} else if (holds_alternative<std::string>() && std::holds_alternative<std::int64_t>(value)) {
		std::int64_t n = std::get<std::int64_t>(value);
		std::string& targetString = get<std::string>();
		if (n < 0) {
			throw std::runtime_error("cannot multiply string by negative value");
		} else if (n == 0) {
			targetString.clear();
		} else {
			// Create a copy in case the source string is the target string.
			std::string sourceString = get<std::string>();
			while (--n) {
				targetString.insert(targetString.end(), sourceString.begin(), sourceString.end());
			}
		}
	} else if (holds_alternative<std::shared_ptr<List>>() && std::holds_alternative<std::int64_t>(value)) {
		std::int64_t n = std::get<std::int64_t>(value);
		List& targetList = *get<std::shared_ptr<List>>();
		if (n < 0) {
			throw std::runtime_error("cannot multiply list by negative value");
		} else if (n == 0) {
			targetList.clear();
		} else {
			// Create a copy in case the source list is the target list.
			List sourceList = *get<std::shared_ptr<List>>();
			while (--n) {
				targetList.insert(targetList.end(), sourceList.begin(), sourceList.end());
			}
		}
	} else {
		throw std::runtime_error("cannot perform multiplicative assignment");
	}
}

void ValueReference::operator^=(Value const& value) {
	if (holds_alternative<std::int64_t>() && std::holds_alternative<std::int64_t>(value)) {
		get<std::int64_t>() &= std::get<std::int64_t>(value);
	} else {
		throw std::runtime_error("cannot perform integral compound assignment on a non-integral value");
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

Value Awaitable::Await() {
	FiberRunner::get_Instance().Await(handle);
	return fn(handle);
}
