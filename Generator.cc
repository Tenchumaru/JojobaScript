#include "pch.h"
#include "Context.h"
#include "Expression.h"
#include "FiberRunner.h"
#include "Generator.h"
#include "Statement.h"

Generator::~Generator() {}

Value FunctionGenerator::operator++() {
	if (it != statements.end()) {
		if (fiber) {
			// Continue the generator fiber.
			FiberRunner::SwitchToFiber(fiber);
		} else {
			// Create hidden values to track progress.
			context->AddValue("$error", nullptr, false);
			context->AddValue("$generator", reinterpret_cast<std::int64_t>(FiberRunner::GetCurrentFiber()), false);
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
				} };
			fiber = FiberRunner::get_Instance().Launch(std::move(fn));
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
