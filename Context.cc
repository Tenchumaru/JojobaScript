#include <stdexcept>
#include "Context.h"

void Context::AddValue(std::string const& key, Value value) {
	auto it = values.find(key);
	if (it == values.end()) {
		values.insert({ key, value });
	} else {
		throw std::logic_error("cannot add known value");
	}
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
