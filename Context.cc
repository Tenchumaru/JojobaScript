#include <stdexcept>
#include "Context.h"

bool AsBoolean(Value const& value) {
	switch (value.index()) {
	case 0: // nullptr_t
		throw std::logic_error("cannot use empty value");
	case 1: // bool
		return std::get<1>(value);
	case 2: // std::int64_t
		return std::get<2>(value) != 0;
	case 3: // double
		return std::get<3>(value) != 0.0;
	case 4: // std::string
		return !std::get<4>(value).empty();
	default:
		throw std::logic_error("unexpected value index");
	}
}

void Context::AddValue(std::string const& key, Value value) {
	auto it = values.find(key);
	if (it == values.end()) {
		values.insert({ key, value });
	} else {
		throw std::logic_error("cannot add known value");
	}
}

Value& Context::GetReference(std::string const& key) {
	auto it = values.find(key);
	if (it != values.end()) {
		return it->second;
	} else if (outerContext) {
		return outerContext->GetReference(key);
	}
	throw std::logic_error("cannot find value");
}

Value Context::GetValue(std::string const& key) {
	auto it = values.find(key);
	if (it != values.end()) {
		return it->second;
	} else if (outerContext) {
		return outerContext->GetValue(key);
	}
	throw std::logic_error("cannot find value");
}

void Context::SetValue(std::string const& key, Value value) {
	// TODO:  add the "const" concept and refuse to set such values.
	auto it = values.find(key);
	if (it != values.end()) {
		it->second = value;
	} else if (outerContext) {
		return outerContext->SetValue(key, value);
	}
	throw std::logic_error("cannot find key");
}
