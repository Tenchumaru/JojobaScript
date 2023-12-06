#include "pch.h"
#include "Context.h"

std::unordered_map<std::string, std::shared_ptr<Context>> Context::modules;

void Context::AddContext(std::string const& moduleName, std::shared_ptr<Context> context) {
	// Create an object to hold its defined symbols.
	auto object = std::make_shared<Object>();

	// Add each defined symbol to the context.
	for (auto& pair : context->values) {
		object->insert({ pair.first, pair.second.first });
	}

	// Add the object to the context using the module name as the object name.
	AddValue(moduleName, object, true);
}

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
