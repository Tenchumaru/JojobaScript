#include "pch.h"
#include "Context.h"
#include "Value.cc.inl"

void Context::AddValue(std::string const& key, Value const& value) {
	auto it = values.find(key);
	if (it == values.end()) {
		values.insert({ key, value });
	} else {
		throw std::runtime_error("cannot add known value");
	}
}

Value& Context::GetReference(std::string const& key) {
	auto it = values.find(key);
	if (it != values.end()) {
		return it->second;
	} else if (outerContext) {
		return outerContext->GetReference(key);
	}
	throw std::runtime_error("cannot find value");
}

Value Context::GetValue(std::string const& key) {
	auto it = values.find(key);
	if (it != values.end()) {
		return it->second;
	} else if (outerContext) {
		return outerContext->GetValue(key);
	}
	throw std::runtime_error("cannot find value");
}

void Context::SetValue(std::string const& key, Value const& value) {
	// TODO:  add the "const" concept and refuse to set such values.
	auto it = values.find(key);
	if (it != values.end()) {
		it->second = value;
	} else if (outerContext) {
		return outerContext->SetValue(key, value);
	} else {
		throw std::runtime_error("cannot find key");
	}
}
