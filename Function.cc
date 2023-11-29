#include "pch.h"
#include "Context.h"
#include "Function.h"
#include "Statement.h"

Value Function::Invoke(std::vector<Value>&& arguments) {
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

	// Run the statements.
	for (auto const& statement : statements) {
		auto runResult = statement->Run(context);
		if (runResult.first == Statement::RunResult::Return) {
			return std::get<Value>(runResult.second);
		} else if (runResult.first != Statement::RunResult::Next) {
			throw std::runtime_error("unexpected break or continue statement");
		}
	}
	return {};
}
