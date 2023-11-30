#include "pch.h"
#include "Context.h"
#include "Function.h"
#include "Generator.h"
#include "Statement.h"

namespace {
	void PrintValue(Value const& value);

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

	void PrintValue(Value const& value) {
		switch (value.index()) {
		case 0: // nullptr_t
			throw std::runtime_error("cannot use empty value");
		case 1: // bool
			std::cout << std::boolalpha << std::get<1>(value);
			break;
		case 2: // std::int64_t
			std::cout << std::get<2>(value);
			break;
		case 3: // double
			std::cout << std::get<3>(value);
			break;
		case 4: // std::string
			std::cout << '"' << std::get<4>(value) << '"';
			break;
		case 5: // std::shared_ptr<Dictionary>
			PrintDictionary(*std::get<5>(value));
			break;
		case 6: // std::shared_ptr<Function>
			std::cout << "function";
			break;
		case 7: // std::shared_ptr<Generator>
			std::cout << "generator";
			break;
		case 8: // std::shared_ptr<List>
			std::cout << '[';
			PrintCollection(*std::get<8>(value));
			std::cout << ']';
			break;
		case 9: // std::shared_ptr<Set>
			std::cout << '{';
			PrintCollection(*std::get<9>(value));
			std::cout << '}';
			break;
		default:
			throw std::logic_error("unexpected value index");
		}
	}
}

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
