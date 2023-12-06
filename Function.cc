#include "pch.h"
#include <Windows.h>
#include "Context.h"
#include "Function.h"
#include "Generator.h"
#include "Statement.h"

std::ostream* PrintFunction::outputStream;

namespace {
	void PrintValue(Value const& value);

	template<typename T>
	void PrintCollection(T const& collection, char const framing[]) {
		*PrintFunction::outputStream << framing[0];
		bool isNext = false;
		for (auto const& value : collection) {
			if (isNext) {
				*PrintFunction::outputStream << ", ";
			}
			PrintValue(value);
			isNext = true;
		}
		*PrintFunction::outputStream << framing[1];
	}

	void PrintDictionary(Dictionary const& dictionary) {
		*PrintFunction::outputStream << '{';
		bool isNext = false;
		for (auto const& pair : dictionary) {
			if (isNext) {
				*PrintFunction::outputStream << ", ";
			}
			PrintValue(pair.first);
			*PrintFunction::outputStream << ": ";
			PrintValue(pair.second);
			isNext = true;
		}
		*PrintFunction::outputStream << '}';
	}

	void PrintObject(Object const& object) {
		*PrintFunction::outputStream << "#{";
		bool isNext = false;
		for (auto const& pair : object) {
			if (isNext) {
				*PrintFunction::outputStream << ", ";
			}
			*PrintFunction::outputStream << pair.first << ": ";
			PrintValue(pair.second);
			isNext = true;
		}
		*PrintFunction::outputStream << '}';
	}

#define PART _CRT_INTERNAL_PRINTF_STANDARD_ROUNDING
#include "Value.cc.inl"
}

Function::~Function() {}

Value PrintFunction::Invoke(std::vector<Value> const& arguments) {
	bool isNext = false;
	for (Value const& argument : arguments) {
		if (isNext) {
			*PrintFunction::outputStream << ' ';
		}
		PrintValue(argument);
		isNext = true;
	}
	*PrintFunction::outputStream << std::endl;
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

Value ThePassageOfTimeFunction::Invoke(std::vector<Value> const& arguments) {
	// Ensure there is one integral argument.
	if (arguments.size() != 1) {
		throw std::runtime_error("invalid number of arguments");
	} else if (!std::holds_alternative<double>(arguments.back()) && !std::holds_alternative<std::int64_t>(arguments.back())) {
		throw std::runtime_error("incorrect argument type");
	}

	// Set a timer for the desired time.  I invoke CloseHandle in the Awaitable::Await method.
	auto timer = CreateWaitableTimer(nullptr, FALSE, nullptr);
	if (timer) {
		LARGE_INTEGER time{};
		if (std::holds_alternative<double>(arguments.back())) {
			auto nseconds = std::get<double>(arguments.back());
			time.QuadPart = static_cast<decltype(time.QuadPart)>(-10'000'000.0 * nseconds);
		} else {
			auto nseconds = std::get<std::int64_t>(arguments.back());
			time.QuadPart = -10'000'000LL * nseconds;
		}
		if (SetWaitableTimer(timer, &time, 0, nullptr, nullptr, TRUE)) {
			return std::make_shared<Awaitable>(timer, [](void* handle) {
				CloseHandle(handle);
				return nullptr; });
		}
	}
	throw std::runtime_error("unexpected system failure");
}
