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

std::pair<Value, std::shared_ptr<Context>> FloatFunction::Invoke(std::vector<Value> const& arguments) {
	if (arguments.empty()) {
		return { 0.0, {} };
	} else if (arguments.size() > 1) {
		throw std::runtime_error("invalid number of arguments");
	}
	Value const& value = arguments[0];
	if (std::holds_alternative<double>(value)) {
		return { value, {} };
	} else if (std::holds_alternative<std::int64_t>(value)) {
		return { static_cast<double>(std::get<std::int64_t>(value)), {} };
	} else if (std::holds_alternative<std::string>(value)) {
		auto&& s = std::get<std::string>(value);
		auto end = std::find_if_not(s.rbegin(), s.rend(), [](char ch) { return std::isspace(ch); });
		ptrdiff_t ntrailing = end - s.rbegin();
		try {
			size_t n;
			double rv = std::stod(s, &n);
			if (n + ntrailing != s.size()) {
				throw std::runtime_error("value to convert to float has invalid characters");
			}
			return { rv, {} };
		} catch (std::invalid_argument const&) {
			throw std::runtime_error("cannot convert value to float");
		} catch (std::out_of_range const&) {
			throw std::runtime_error("value to convert to float is out of range");
		}
	}
	throw std::runtime_error("cannot convert value to float");
}

std::pair<Value, std::shared_ptr<Context>> IntFunction::Invoke(std::vector<Value> const& arguments) {
	if (arguments.empty()) {
		return { 0.0, {} };
	} else if (arguments.size() > 1) {
		throw std::runtime_error("invalid number of arguments");
	}
	Value const& value = arguments[0];
	if (std::holds_alternative<double>(value)) {
		return { static_cast<std::int64_t>(std::get<double>(value)), {} };
	} else if (std::holds_alternative<std::int64_t>(value)) {
		return { value, {} };
	} else if (std::holds_alternative<std::string>(value)) {
		auto&& s = std::get<std::string>(value);
		auto end = std::find_if_not(s.rbegin(), s.rend(), [](char ch) { return std::isspace(ch); });
		auto decimal_point = std::find(end, s.rend(), '.');
		ptrdiff_t ntrailing = decimal_point == s.rend() ? end - s.rbegin() : decimal_point - s.rbegin() + 1;
		try {
			size_t n;
			std::int64_t rv = std::stoll(decimal_point == s.rend() ? s : s.substr(0, s.size() - ntrailing), &n);
			if (n + ntrailing != s.size()) {
				throw std::runtime_error("value to convert to int has invalid characters");
			}
			return { rv, {} };
		} catch (std::invalid_argument const&) {
			throw std::runtime_error("cannot convert value to int");
		} catch (std::out_of_range const&) {
			throw std::runtime_error("value to convert to int is out of range");
		}
	}
	throw std::runtime_error("cannot convert value to int");
}

std::pair<Value, std::shared_ptr<Context>> PrintFunction::Invoke(std::vector<Value> const& arguments) {
	bool isNext = false;
	for (Value const& argument : arguments) {
		if (isNext) {
			*PrintFunction::outputStream << ' ';
		}
		PrintValue(argument);
		isNext = true;
	}
	*PrintFunction::outputStream << std::endl;
	return { nullptr, {} };
}

std::pair<Value, std::shared_ptr<Context>> ScriptFunction::Invoke(std::vector<Value> const& arguments) {
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
		return { std::make_shared<FunctionGenerator>(context, statements), context };
	} else {
		// Run the statements.
		for (std::unique_ptr<Statement> const& statement : statements) {
			auto runResult = statement->Run(context);
			switch (runResult.first) {
			case Statement::RunResult::Next:
				break;
			case Statement::RunResult::Return:
				return { std::get<Value>(runResult.second), context };
			default:
				throw std::runtime_error("unexpected break, continue, fallthrough, throw, or yield statement");
			}
		}
	}
	return { {}, context };
}

std::pair<Value, std::shared_ptr<Context>> ThePassageOfTimeFunction::Invoke(std::vector<Value> const& arguments) {
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
			return { std::make_shared<Awaitable>(timer, [](void* handle) {
				CloseHandle(handle);
				return nullptr; }), {} };
		}
	}
	throw std::runtime_error("unexpected system failure");
}
