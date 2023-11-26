#include <algorithm>
#include <iterator>
#include <stdexcept>
#include "Generator.h"

Generator::~Generator() {}

Value Generator::Evaluate(std::function<void(std::shared_ptr<Context>)> fn) {
	auto context = std::make_shared<Context>(outerContext);
	fn(context);
	return targetExpression->GetValue(context);
}

std::shared_ptr<Generator> Generator::Create(std::shared_ptr<Context> outerContext, std::unique_ptr<Expression> const& targetExpression, std::vector<std::pair<std::string, std::string>> const& ids, Value&& sourceValue) {
	if (std::holds_alternative<std::string>(sourceValue)) {
		if (ids.size() != 1) {
			throw std::runtime_error("too many comprehension identifiers");
		}
		return std::make_shared<StringGenerator>(outerContext, std::cref(targetExpression), std::cref(ids[0]), std::get<std::string>(sourceValue));
	} else if (std::holds_alternative<std::shared_ptr<List>>(sourceValue)) {
		if (ids.size() != 1) {
			throw std::runtime_error("too many comprehension identifiers");
		}
		return std::make_shared<ListGenerator>(outerContext, std::cref(targetExpression), std::cref(ids[0]), std::get<std::shared_ptr<List>>(sourceValue));
	} else if (std::holds_alternative<std::shared_ptr<Dictionary>>(sourceValue)) {
		if (ids.size() != 2) {
			throw std::runtime_error("incorrect number of comprehension identifiers");
		}
		return std::make_shared<DictionaryGenerator>(outerContext, std::cref(targetExpression), std::cref(ids[0]), std::cref(ids[1]), std::get<std::shared_ptr<Dictionary>>(sourceValue));
	} else if (std::holds_alternative<std::shared_ptr<Generator>>(sourceValue)) {
		if (ids.size() != 1) {
			throw std::runtime_error("too many comprehension identifiers");
		}
		return std::make_shared<GeneratorGenerator>(outerContext, std::cref(targetExpression), std::cref(ids[0]), std::get<std::shared_ptr<Generator>>(sourceValue));
	}
	throw std::runtime_error("source value is not iterable");
}

Value DictionaryGenerator::operator++() {
	if (keys.empty()) {
		std::ranges::transform(*sourceValue, std::back_inserter(keys), [](auto const& pair) { return pair.first; });
		keyIt = keys.begin();
	} else if (keyIt != keys.end()) {
		++keyIt;
	}
	if (keyIt != keys.end()) {
		if (keys.size() == sourceValue->size()) {
			auto valueIt = sourceValue->find(*keyIt);
			if (valueIt != sourceValue->end()) {
				Value nextValue = valueIt->second;
				return Evaluate([this, valueIt](std::shared_ptr<Context> context) {
					context->AddValue(keyId.first, valueIt->first),
					context->AddValue(valueId.first, valueIt->second); });
			}
		}
		throw std::runtime_error("dictionary changed");
	}
	return nullptr;
}

Value GeneratorGenerator::operator++() {
	Value nextValue = ++*sourceValue;
	if (std::holds_alternative<nullptr_t>(nextValue)) {
		return nextValue;
	}
	return Evaluate([this, &nextValue](std::shared_ptr<Context> context) { context->AddValue(id.first, nextValue); });
}

Value ListGenerator::operator++() {
	if (index < 0) {
		index = 0;
	} else if (index < sourceValue->size()) {
		++index;
	}
	if (index < sourceValue->size()) {
		Value nextValue = (*sourceValue)[index];
		return Evaluate([this, &nextValue](std::shared_ptr<Context> context) { context->AddValue(id.first, nextValue); });
	}
	return nullptr;
}

Value StringGenerator::operator++() {
	if (it == std::string::const_iterator{}) {
		it = sourceValue.begin();
	} else if (it != sourceValue.end()) {
		++it;
	}
	if (it != sourceValue.end()) {
		Value nextValue = Value(std::string(1, *it));
		return Evaluate([this, &nextValue](std::shared_ptr<Context> context) { context->AddValue(id.first, nextValue); });
	}
	return nullptr;
}
