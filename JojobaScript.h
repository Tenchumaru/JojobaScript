#pragma once

#include <format>
#include <memory>
#include <string>
#include <vector>
#include "Statement.h"

enum class FunctionType { Unspecified, Standard, Asynchronous, Generator };

class ContextTemplate;

class Context {
public:
	Context(ContextTemplate& contextTemplate, std::shared_ptr<Context> parentContext) : contextTemplate(contextTemplate), parentContext(std::move(parentContext)) {}
	~Context() = default;

private:
	ContextTemplate& contextTemplate;
	std::shared_ptr<Context> parentContext;
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
	~Type() = default;

private:
	std::string type;
};

class Function {
public:
	Function(std::vector<std::string> const& parameters, ContextTemplate const& contextTemplate, std::shared_ptr<Context> parentContext) : parameters(parameters), contextTemplate(contextTemplate), parentContext(std::move(parentContext)) {}
	~Function() = default;

private:
	std::vector<std::string> parameters;
	ContextTemplate const& contextTemplate;
	std::shared_ptr<Context> parentContext;
};

#include "parser.h"
#include "scanner.h"

extern std::vector<std::shared_ptr<Context>> contextStack;
extern std::vector<ContextTemplate> contextTemplates;
