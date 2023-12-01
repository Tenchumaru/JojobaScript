#include "pch.h"
#include "Context.h"
#include "Generator.h"
#include "Iterator.h"
#include "Statement.h"

Iterator::Strategy::~Strategy() {}

Iterator::Iterator(std::shared_ptr<Context> outerContext, std::vector<std::pair<std::string, std::string>> const& ids, std::unique_ptr<Expression> const& sourceExpression) : outerContext(outerContext), ids(ids) {
	Value sourceValue = sourceExpression->GetValue(outerContext);
	if (std::holds_alternative<std::string>(sourceValue)) {
		strategy = std::make_unique<StringStrategy>(std::get<std::string>(sourceValue));
	} else if (std::holds_alternative<std::shared_ptr<Dictionary>>(sourceValue)) {
		strategy = std::make_unique<DictionaryStrategy>(std::get<std::shared_ptr<Dictionary>>(sourceValue));
	} else if (std::holds_alternative<std::shared_ptr<Generator>>(sourceValue)) {
		strategy = std::make_unique<GeneratorStrategy>(std::get<std::shared_ptr<Generator>>(sourceValue));
	} else if (std::holds_alternative<std::shared_ptr<List>>(sourceValue)) {
		strategy = std::make_unique<ListStrategy>(std::get<std::shared_ptr<List>>(sourceValue));
	} else if (std::holds_alternative<std::shared_ptr<Set>>(sourceValue)) {
		strategy = std::make_unique<SetStrategy>(std::get<std::shared_ptr<Set>>(sourceValue));
	} else {
		throw std::runtime_error("source value is not iterable");
	}
}

std::shared_ptr<Context> Iterator::operator++() {
	Value nextValue = ++*strategy;
	if (std::holds_alternative<nullptr_t>(nextValue)) {
		return {};
	}
	auto context = std::make_shared<Context>(outerContext);
	if (ids.size() == 1) {
		context->AddValue(ids.back().first, nextValue, true);
	} else if (std::holds_alternative<std::shared_ptr<List>>(nextValue)) {
		auto const& list = *std::get<std::shared_ptr<List>>(nextValue);
		if (ids.size() != list.size()) {
			throw std::runtime_error("incorrect number of comprehension identifiers");
		}
		for (size_t i = 0; i < ids.size(); ++i) {
			context->AddValue(ids[i].first, list[i], true);
		}
	} else {
		throw std::runtime_error("incorrect number of comprehension identifiers");
	}
	return context;
}

Value Iterator::DictionaryStrategy::operator++() {
	if (keys.empty()) {
		std::ranges::transform(*sourceValue, std::back_inserter(keys), [](auto const& pair) { return pair.first; });
		keyIt = keys.begin();
	} else if (keyIt != keys.end()) {
		++keyIt;
	}
	if (keyIt != keys.end()) {
		if (keys.size() == sourceValue->size()) {
			auto pairIt = sourceValue->find(*keyIt);
			if (pairIt != sourceValue->end()) {
				return std::make_shared<List>(std::vector<Value>{ pairIt->first, pairIt->second });
			}
		}
		throw std::runtime_error("dictionary changed");
	}
	return nullptr;
}

Value Iterator::GeneratorStrategy::operator++() {
	return ++*sourceValue;
}

Value Iterator::ListStrategy::operator++() {
	if (index < 0) {
		index = 0;
	} else if (index < sourceValue->size()) {
		++index;
	}
	if (index < sourceValue->size()) {
		return (*sourceValue)[index];
	}
	return nullptr;
}

std::shared_ptr<List> Iterator::SetStrategy::ConvertToList(std::shared_ptr<Set> sourceValue) {
	Set const& set = *sourceValue;
	std::vector<Value> values(set.begin(), set.end());
	return std::make_shared<List>(std::move(values));
}

Value Iterator::StringStrategy::operator++() {
	if (!it.has_value()) {
		it = sourceValue.begin();
	} else if (it.value() != sourceValue.end()) {
		++it.value();
	}
	if (it != sourceValue.end()) {
		return Value(std::string(1, *it.value()));
	}
	return nullptr;
}
