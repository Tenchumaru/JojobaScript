#pragma once

#include "Context.h"

class Expression;

class Strategy {
public:
	virtual ~Strategy() = 0;
	virtual Value operator++() = 0;
};

class Generator {
public:
	Generator(std::shared_ptr<Context> outerContext, std::unique_ptr<Expression> const& targetExpression, std::vector<std::pair<std::string, std::string>> const& ids, std::unique_ptr<Expression> const& sourceExpression);
	Generator(Generator&&) = default;
	~Generator() = default;
	Value operator++();

private:
	std::shared_ptr<Context> outerContext;
	std::unique_ptr<Expression> const& targetExpression;
	std::vector<std::pair<std::string, std::string>> const& ids;
	std::unique_ptr<Expression> const& sourceExpression;
	std::unique_ptr<Strategy> strategy;
};

class DictionaryStrategy : public Strategy {
public:
	DictionaryStrategy(std::shared_ptr<Dictionary> sourceValue) : sourceValue(std::move(sourceValue)) {}
	DictionaryStrategy(DictionaryStrategy&&) = default;
	~DictionaryStrategy() = default;

private:
	std::shared_ptr<Dictionary> sourceValue;
	std::vector<Value> keys; // Keep a collection of keys since an iterator might become invalid if the dictionary changes.
	std::vector<Value>::const_iterator keyIt;

	Value operator++() override;
};

class GeneratorStrategy : public Strategy {
public:
	GeneratorStrategy(std::shared_ptr<Generator> sourceValue) : sourceValue(std::move(sourceValue)) {}
	GeneratorStrategy(GeneratorStrategy&&) = default;
	~GeneratorStrategy() = default;

private:
	std::shared_ptr<Generator> sourceValue;

	Value operator++() override;
};

class ListStrategy : public Strategy {
public:
	ListStrategy(std::shared_ptr<List> sourceValue) : sourceValue(std::move(sourceValue)) {}
	ListStrategy(ListStrategy&&) = default;
	~ListStrategy() = default;

private:
	std::shared_ptr<List> sourceValue;
	int index = -1; // Use an index since an iterator might become invalid if the list changes.

	Value operator++() override;
};

class SetStrategy : public ListStrategy {
public:
	SetStrategy(std::shared_ptr<Set> sourceValue) : ListStrategy(ConvertToList(sourceValue)) {}
	SetStrategy(SetStrategy&&) = default;
	~SetStrategy() = default;

private:
	static std::shared_ptr<List> ConvertToList(std::shared_ptr<Set> sourceValue);
};

class StringStrategy : public Strategy {
public:
	StringStrategy(std::string sourceValue) : sourceValue(std::move(sourceValue)) {}
	StringStrategy(StringStrategy&&) = default;
	~StringStrategy() = default;

private:
	std::string sourceValue;
	std::optional<std::string::const_iterator> it;

	Value operator++() override;
};
