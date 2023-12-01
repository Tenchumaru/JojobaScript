#include "pch.h"
#include "Context.h"
#include "Value.cc.inl"

void Context::AddValue(std::string const& key, Value const& value, bool isConstant) {
	auto it = values.find(key);
	if (it == values.end()) {
		values.insert({ key, { value, isConstant } });
	} else {
		throw std::runtime_error("cannot add known value");
	}
}

Value& Context::GetReference(std::string const& key) {
	auto it = values.find(key);
	if (it != values.end()) {
		return it->second.first;
	} else if (outerContext) {
		return outerContext->GetReference(key);
	}
	throw std::runtime_error("cannot find value");
}

Value Context::GetValue(std::string const& key) {
	auto it = values.find(key);
	if (it != values.end()) {
		return it->second.first;
	} else if (outerContext) {
		return outerContext->GetValue(key);
	}
	throw std::runtime_error("cannot find value");
}

bool Context::IsConstant(std::string const& key) {
	auto it = values.find(key);
	if (it != values.end()) {
		return it->second.second;
	} else if (outerContext) {
		return outerContext->IsConstant(key);
	}
	throw std::logic_error("cannot find value");
}
