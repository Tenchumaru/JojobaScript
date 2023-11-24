#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Statement.h"
#include "Expression.h"

class ContextTemplate;

class Context {
public:
	Context(ContextTemplate& contextTemplate, std::shared_ptr<Context> outerContext) : contextTemplate(contextTemplate), outerContext(std::move(outerContext)) {}
	Context(Context&&) = default;
	~Context() = default;

private:
	ContextTemplate& contextTemplate;
	std::shared_ptr<Context> outerContext;
};

class ContextTemplate {
public:
	ContextTemplate() = default;
	ContextTemplate(ContextTemplate&&) = default;
	~ContextTemplate() = default;
	void AddCode(int operation, std::vector<std::string> const& operators);
	void AddSymbol(std::string const& symbol, bool isFunction);

private:
};

class Type {
public:
	Type(std::string&& type) : type(std::move(type)) {}
	Type(Type&&) = default;
	~Type() = default;

private:
	std::string type;
};

class Function {
public:
	Function(std::vector<std::string> const& parameters, ContextTemplate const& contextTemplate, std::shared_ptr<Context> outerContext) : parameters(parameters), contextTemplate(contextTemplate), outerContext(std::move(outerContext)) {}
	Function(Function&&) = default;
	~Function() = default;

private:
	std::vector<std::string> parameters;
	ContextTemplate const& contextTemplate;
	std::shared_ptr<Context> outerContext;
};

extern std::vector<std::shared_ptr<Context>> contextStack;
extern std::vector<ContextTemplate> contextTemplates;
