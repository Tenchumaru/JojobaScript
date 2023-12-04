#include "pch.h"
#include "Context.h"
#include "Function.h"
#include "Generator.h"
#include "Statement.h"

namespace {
	void PrintValue(Value const& value);

	template<typename T>
	void PrintCollection(T const& collection) {
		bool isNext = false;
		for (auto const& value : collection) {
			if (isNext) {
				std::cout << ", ";
			}
			PrintValue(value);
			isNext = true;
		}
	}

	void PrintDictionary(Dictionary const& dictionary) {
		std::cout << '{';
		bool isNext = false;
		for (auto const& pair : dictionary) {
			if (isNext) {
				std::cout << ", ";
			}
			PrintValue(pair.first);
			std::cout << ": ";
			PrintValue(pair.second);
			isNext = true;
		}
		std::cout << '}';
	}

	void PrintObject(Object const& object) {
		std::cout << "#{";
		bool isNext = false;
		for (auto const& pair : object) {
			if (isNext) {
				std::cout << ", ";
			}
			std::cout << pair.first << ": ";
			PrintValue(pair.second);
			isNext = true;
		}
		std::cout << '}';
	}

#define PART _CRT_INTERNAL_PRINTF_STANDARD_ROUNDING
#include "Value.cc.inl"
}

Function::~Function() {}

Value PrintFunction::Invoke(std::vector<Value> const& arguments) {
	bool isNext = false;
	for (Value const& argument : arguments) {
		if (isNext) {
			std::cout << ' ';
		}
		PrintValue(argument);
		isNext = true;
	}
	std::cout << std::endl;
	return nullptr;
}

Value ScriptFunction::Invoke(std::vector<Value> const& arguments) {
	// Ensure the number of arguments matches the number of parameters.
	if (arguments.size() != parameters.size()) {
		throw std::runtime_error("invalid number of arguments");
	}

	// Create a context.
	auto context = std::make_shared<Context>(outerContext);

	// Add the arguments to the context using the parameters as their names.
	for (size_t i = 0; i < arguments.size(); ++i) {
		context->AddValue(parameters[i].first, arguments[i], false);
	}

	if (yielding) {
		return std::make_shared<FunctionGenerator>(context, statements);
	} else {
		// Run the statements.
		for (std::unique_ptr<Statement> const& statement : statements) {
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
