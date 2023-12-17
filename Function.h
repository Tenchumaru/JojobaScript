#pragma once

#include "Context.h"

class Statement;

class Function {
public:
	virtual ~Function() = 0;
	virtual std::pair<Value, std::shared_ptr<Context>> Invoke(std::vector<Value> const& arguments) = 0;
};

class NativeFunction : public Function {};

class FloatFunction : public NativeFunction {
public:
	static std::ostream* outputStream;

	FloatFunction() = default;
	FloatFunction(FloatFunction&&) = default;
	~FloatFunction() = default;
	std::pair<Value, std::shared_ptr<Context>> Invoke(std::vector<Value> const& arguments) override;
};

class InputFunction : public NativeFunction {
public:
	InputFunction() = default;
	InputFunction(InputFunction&&) = default;
	~InputFunction() = default;
	std::pair<Value, std::shared_ptr<Context>> Invoke(std::vector<Value> const& arguments) override;
};

class IntFunction : public NativeFunction {
public:
	static std::ostream* outputStream;

	IntFunction() = default;
	IntFunction(IntFunction&&) = default;
	~IntFunction() = default;
	std::pair<Value, std::shared_ptr<Context>> Invoke(std::vector<Value> const& arguments) override;
};

class PrintFunction : public NativeFunction {
public:
	static std::ostream* outputStream;

	PrintFunction() = default;
	PrintFunction(PrintFunction&&) = default;
	~PrintFunction() = default;
	std::pair<Value, std::shared_ptr<Context>> Invoke(std::vector<Value> const& arguments) override;
};

class ScriptFunction : public Function {
public:
	ScriptFunction(std::vector<std::pair<std::string, std::string>> const& parameters, std::vector<std::unique_ptr<Statement>> const& statements, std::shared_ptr<Context> outerContext, bool yielding) : parameters(parameters), statements(statements), outerContext(std::move(outerContext)), yielding(yielding) {}
	ScriptFunction(ScriptFunction&&) = default;
	~ScriptFunction() = default;
	std::pair<Value, std::shared_ptr<Context>> Invoke(std::vector<Value> const& arguments) override;

private:
	std::vector<std::pair<std::string, std::string>> const& parameters;
	std::vector<std::unique_ptr<Statement>> const& statements;
	std::shared_ptr<Context> outerContext;
	bool yielding;
};

class StringFunction : public NativeFunction {
public:
	static std::ostream* outputStream;

	StringFunction() = default;
	StringFunction(StringFunction&&) = default;
	~StringFunction() = default;
	std::pair<Value, std::shared_ptr<Context>> Invoke(std::vector<Value> const& arguments) override;
};

class ThePassageOfTimeFunction : public NativeFunction {
public:
	ThePassageOfTimeFunction() = default;
	ThePassageOfTimeFunction(ThePassageOfTimeFunction&&) = default;
	~ThePassageOfTimeFunction() = default;
	std::pair<Value, std::shared_ptr<Context>> Invoke(std::vector<Value> const& arguments) override;
};
