#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include "JojobaScript.h"

class Generator {
public:
	static std::shared_ptr<Generator> Create(std::shared_ptr<Context> outerContext, std::unique_ptr<Expression> const& targetExpression, std::vector<std::pair<std::string, std::string>> const& ids, Value&& sourceValue);
	Generator(Generator&&) = default;
	virtual ~Generator() = 0;
	virtual Value operator++() = 0;

protected:
	Generator(std::shared_ptr<Context> outerContext, std::unique_ptr<Expression> const& targetExpression) : outerContext(outerContext), targetExpression(targetExpression) {}
	Value Evaluate(std::function<void(std::shared_ptr<Context>)> fn);

private:
	std::shared_ptr<Context> outerContext;
	std::unique_ptr<Expression> const& targetExpression;
};

class DictionaryGenerator : public Generator {
public:
	DictionaryGenerator(std::shared_ptr<Context> outerContext, std::unique_ptr<Expression> const& targetExpression, std::pair<std::string, std::string> const& keyId, std::pair<std::string, std::string> const& valueId, std::shared_ptr<Dictionary> sourceValue) : Generator(outerContext, targetExpression), keyId(keyId), valueId(valueId), sourceValue(sourceValue) {}
	~DictionaryGenerator() = default;
	Value operator++() override;

private:
	std::pair<std::string, std::string> const& keyId;
	std::pair<std::string, std::string> const& valueId;
	std::shared_ptr<Dictionary> sourceValue;
	std::vector<Value> keys; // Keep a collection of keys since an iterator might become invalid if the dictionary changes.
	std::vector<Value>::const_iterator keyIt;
};

class GeneratorGenerator : public Generator {
public:
	GeneratorGenerator(std::shared_ptr<Context> outerContext, std::unique_ptr<Expression> const& targetExpression, std::pair<std::string, std::string> const& id, std::shared_ptr<Generator> sourceValue) : Generator(outerContext, targetExpression), id(id), sourceValue(sourceValue) {}
	~GeneratorGenerator() = default;
	Value operator++() override;

private:
	std::pair<std::string, std::string> const& id;
	std::shared_ptr<Generator> sourceValue;
};

class ListGenerator : public Generator {
public:
	ListGenerator(std::shared_ptr<Context> outerContext, std::unique_ptr<Expression> const& targetExpression, std::pair<std::string, std::string> const& id, std::shared_ptr<List> sourceValue) : Generator(outerContext, targetExpression), id(id), sourceValue(sourceValue) {}
	~ListGenerator() = default;
	Value operator++() override;

private:
	std::pair<std::string, std::string> const& id;
	std::shared_ptr<List> sourceValue;
	int index = -1; // Use an index since an iterator might become invalid if the list changes.
};

class StringGenerator : public Generator {
public:
	StringGenerator(std::shared_ptr<Context> outerContext, std::unique_ptr<Expression> const& targetExpression, std::pair<std::string, std::string> const& id, std::string sourceValue) : Generator(outerContext, targetExpression), id(id), sourceValue(std::move(sourceValue)) {}
	~StringGenerator() = default;
	Value operator++() override;

private:
	std::pair<std::string, std::string> const& id;
	std::string sourceValue;
	std::string::const_iterator it = std::string::const_iterator{};
};
