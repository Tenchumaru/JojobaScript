#include "pch.h"
#include "Context.h"
#include "Expression.h"
#include "Generator.h"
#include "Statement.h"

Generator::~Generator() {}

Value FunctionGenerator::operator++() {
	// TODO:  The "yield" keyword applies only as a top-level statement in a generator function.  It will skip all lower-level
	// statements.  Consider running this in a fiber and switching fibers upon reaching a "yield" keyword.
	while (it != statements.end()) {
		auto runResult = (*it)->Run(context);
		++it;
		if (runResult.first == Statement::RunResult::Yield) {
			return std::get<Value>(runResult.second);
		} else if (runResult.first != Statement::RunResult::Next) {
			throw std::runtime_error("unexpected non-yield exiting statement in generator function");
		}
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
