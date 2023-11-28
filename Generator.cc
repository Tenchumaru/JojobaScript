#include "pch.h"
#include "Context.h"
#include "Generator.h"
#include "Statement.h"

Strategy::~Strategy() {}

Generator::Generator(std::shared_ptr<Context> outerContext, std::unique_ptr<Expression> const& targetExpression, std::vector<std::pair<std::string, std::string>> const& ids, std::unique_ptr<Expression> const& sourceExpression) : outerContext(outerContext), targetExpression(targetExpression), ids(ids), sourceExpression(sourceExpression) {
	Value sourceValue = sourceExpression->GetValue(outerContext);
	if (std::holds_alternative<std::string>(sourceValue)) {
		if (ids.size() != 1) {
			throw std::runtime_error("too many comprehension identifiers");
		}
		strategy = std::make_unique<StringStrategy>(std::get<std::string>(sourceValue));
	} else if (std::holds_alternative<std::shared_ptr<Set>>(sourceValue)) {
		strategy = std::make_unique<SetStrategy>(std::get<std::shared_ptr<Set>>(sourceValue));
	} else if (std::holds_alternative<std::shared_ptr<List>>(sourceValue)) {
		strategy = std::make_unique<ListStrategy>(std::get<std::shared_ptr<List>>(sourceValue));
	} else if (std::holds_alternative<std::shared_ptr<Dictionary>>(sourceValue)) {
		if (ids.size() != 2) {
			throw std::runtime_error("incorrect number of comprehension identifiers");
		}
		strategy = std::make_unique<DictionaryStrategy>(std::get<std::shared_ptr<Dictionary>>(sourceValue));
	} else if (std::holds_alternative<std::shared_ptr<Generator>>(sourceValue)) {
		strategy = std::make_unique<GeneratorStrategy>(std::get<std::shared_ptr<Generator>>(sourceValue));
	} else {
		throw std::runtime_error("source value is not iterable");
	}
}

Value Generator::operator++() {
	Value nextValue = ++*strategy;
	if (std::holds_alternative<nullptr_t>(nextValue)) {
		return nextValue;
	}
	auto context = std::make_shared<Context>(outerContext);
	if (ids.size() == 1) {
		context->AddValue(ids.back().first, nextValue);
	} else if (std::holds_alternative<std::shared_ptr<List>>(nextValue)) {
		auto&& list = *std::get<std::shared_ptr<List>>(nextValue);
		if (ids.size() != list.size()) {
			throw std::runtime_error("incorrect number of comprehension identifiers");
		}
		for (size_t i = 0; i < ids.size(); ++i) {
			context->AddValue(ids[i].first, list[i]);
		}
	} else {
		throw std::runtime_error("incorrect number of comprehension identifiers");
	}
	Value targetValue = targetExpression->GetValue(context);
	return targetValue;
}

Value DictionaryStrategy::operator++() {
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

Value GeneratorStrategy::operator++() {
	return ++*sourceValue;
}

Value ListStrategy::operator++() {
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

std::shared_ptr<List> SetStrategy::ConvertToList(std::shared_ptr<Set> sourceValue) {
	auto&& set = *sourceValue;
	std::vector<Value> values(set.begin(), set.end());
	return std::make_shared<List>(std::move(values));
}

Value StringStrategy::operator++() {
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
