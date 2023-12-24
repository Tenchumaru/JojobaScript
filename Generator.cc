#include "pch.h"
#define NOMINMAX
#include <Windows.h>
#include "Context.h"
#include "Expression.h"
#include "JojobaFiberRunner.h"
#include "Generator.h"
#include "Statement.h"

Generator::~Generator() {}

FunctionGenerator::~FunctionGenerator() {
	CloseHandle(waitHandle);
}

Value FunctionGenerator::operator++() {
	if (it != statements.end()) {
		if (waitHandle) {
			// Continue the generator fiber.
			JojobaFiberRunner::Get()->Await(waitHandle);
		} else {
			// Create the wait handle.  It is reset manually and starts signaled so it is always ready.
			waitHandle = CreateEvent(nullptr, TRUE, TRUE, nullptr);

			// Create the hidden values to track progress.
			context->AddValue("$error", nullptr, false);
			context->AddValue("$yield", nullptr, false);

			// Create a fiber in which to run this generator.
			auto fn = [this] {
				try {
					do {
						auto runResult = (*it)->Run(context);
						if (runResult.first != Statement::RunResult::Next) {
							context->SetValue("$error", "unexpected non-yield exiting statement in generator function");
							break;
						}
					} while (++it != statements.end());
				} catch (std::exception const& ex) {
					context->SetValue("$error", ex.what());
				}
				context->SetValue("$yield", nullptr); };
			JojobaFiberRunner::Get()->Launch(fn);
		}
		Value error = context->GetValue("$error");
		return std::holds_alternative<nullptr_t>(error) ? context->GetValue("$yield") : throw std::runtime_error(std::get<std::string>(error));
	}
	return nullptr;
}

Value IteratorGenerator::operator++() {
	std::shared_ptr<Context> context = ++iterator;
	if (!context) {
		return nullptr;
	}
	Value targetValue = targetExpression->GetValue(context);
	return targetValue;
}
