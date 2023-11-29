#include "pch.h"
#include "Context.h"
#include "Function.h"
#include "Generator.h"
#include "Statement.h"

Function::~Function() {}

Value ScriptFunction::Invoke(std::vector<Value> const& arguments) {
	// Ensure the number of arguments matches the number of parameters.
	if (arguments.size() != parameters.size()) {
		throw std::runtime_error("invalid number of arguments");
	}

	// Create a context.
	auto context = std::make_shared<Context>(outerContext);

	// Add the arguments to the context using the parameters as their names.
	for (size_t i = 0; i < arguments.size(); ++i) {
		context->AddValue(parameters[i].first, arguments[i]);
	}

	if (yielding) {
		return std::make_shared<FunctionGenerator>(context, statements);
	} else {
		// Run the statements.
		for (auto const& statement : statements) {
			auto runResult = statement->Run(context);
			switch (runResult.first) {
			case Statement::RunResult::Return:
				return std::get<Value>(runResult.second);
			case Statement::RunResult::Yield:
				throw std::logic_error("not implemented");
			case Statement::RunResult::Next:
				break;
			default:
				throw std::runtime_error("unexpected break or continue statement");
			}
		}
	}
	return {};
}
