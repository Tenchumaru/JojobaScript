#include "pch.h"
#include "Statement.h"
#include "Function.h"
#include "Iterator.h"
#include "Generator.h"

using RunResult = std::pair<Statement::RunResult, Statement::RunResultValue>;

namespace {
	Value Di(std::unique_ptr<Expression> const& expression, bool isIncrement, std::shared_ptr<Context> context) {
		Value& reference = expression->GetReference(context);
		if (std::holds_alternative<std::int64_t>(reference)) {
			if (isIncrement) {
				++std::get<std::int64_t>(reference);
			} else {
				--std::get<std::int64_t>(reference);
			}
			return reference;
		}
		throw std::runtime_error("cannot increment a non-integral value");
	}
}

std::unique_ptr<FunctionStatement> FunctionStatement::program;

Statement::~Statement() {}

BlockStatement::~BlockStatement() {}

RunResult BlockStatement::Run(std::shared_ptr<Context> context) const {
	for (std::unique_ptr<Statement> const& statement : statements) {
		::RunResult runResult = statement->Run(context);
		if (runResult.first != RunResult::Next) {
			return runResult;
		}
	}
	return { RunResult::Next, 0 };
}

bool BlockStatement::Run(std::shared_ptr<Context> context, std::pair<RunResult, RunResultValue>& runResult) const {
	runResult = BlockStatement::Run(context);
	switch (runResult.first) {
	case RunResult::Next:
		return false;
	case RunResult::Break:
		if (std::get<int>(runResult.second)) {
			--std::get<int>(runResult.second);
		} else {
			runResult = { RunResult::Next, 0 };
		}
		return true;
	case RunResult::Continue:
		if (std::get<int>(runResult.second)) {
			--std::get<int>(runResult.second);
			return true;
		}
		return false;
	case RunResult::Return:
	case RunResult::Yield:
		return true;
	default:
		throw std::logic_error("unexpected RunResult value");
	}
}

ForStatement::Clause::~Clause() {}

Value Statement::AssignmentClause::Run(std::shared_ptr<Context> context) const {
	Value value = sourceExpression->GetValue(context);
	Value& reference = targetExpression->GetReference(context);
	reference = value;
	return value;
}

Value Statement::DiClause::Run(std::shared_ptr<Context> context) const {
	return Di(expression, isIncrement, context);
}

Value Statement::ExpressionClause::Run(std::shared_ptr<Context> context) const {
	return expression->GetValue(context);
}

Value Statement::VarClause::Run(std::shared_ptr<Context> context) const {
	auto&& [name, _, expression] = initializer;
	Value value = expression->GetValue(context);
	context->AddValue(name, value, isConstant);
	return value;
}

RunResult AssignmentStatement::Run(std::shared_ptr<Context> context) const {
	static std::array<std::function<void(std::int64_t&, std::int64_t)>, static_cast<int>(Assignment::Size)> map = {
		/* =    */ std::function<void(std::int64_t&, std::int64_t)>{},
		/* &=   */ [](std::int64_t& a, std::int64_t b) { a &= b; },
		/* >>>= */ [](std::int64_t& a, std::int64_t b) { a = static_cast<std::int64_t>(static_cast<std::uint64_t>(a) >> b); },
		/* /=   */ [](std::int64_t& a, std::int64_t b) { if (b == 0) throw std::runtime_error("integer division by zero"); a /= b; },
		/* <<=  */ [](std::int64_t& a, std::int64_t b) { a <<= b; },
		/* >>=  */ [](std::int64_t& a, std::int64_t b) { a >>= b; },
		/* %=   */ [](std::int64_t& a, std::int64_t b) { a %= b; },
		/* |=   */ [](std::int64_t& a, std::int64_t b) { a |= b; },
		/* +=   */ [](std::int64_t& a, std::int64_t b) { a += b; },
		/* -=   */ [](std::int64_t& a, std::int64_t b) { a -= b; },
		/* *=   */ [](std::int64_t& a, std::int64_t b) { a *= b; },
		/* ^=   */ [](std::int64_t& a, std::int64_t b) { a ^= b; },
	};

	// Check identifier assignments for constancy.
	auto const* identifierExpression = dynamic_cast<IdentifierExpression const*>(targetExpression.get());
	if (identifierExpression && identifierExpression->IsConstant(context)) {
		throw std::runtime_error("cannot assign constant");
	}

	// Perform the assignment, ensuring compound assignments apply to only integer arguments except for plus-assignment, which may
	// also apply to string arguments.
	Value value = sourceExpression->GetValue(context);
	Value& reference = targetExpression->GetReference(context);
	if (assignment == Assignment()) {
		reference = value;
	} else if (std::holds_alternative<std::int64_t>(reference) && std::holds_alternative<std::int64_t>(value)) {
		map[static_cast<int>(assignment)](std::get<std::int64_t>(reference), std::get<std::int64_t>(value));
	} else if (assignment == Assignment::PA && std::holds_alternative<std::string>(reference) && std::holds_alternative<std::string>(value)) {
		std::get<std::string>(reference) += std::get<std::string>(value);
	} else {
		throw std::runtime_error("cannot perform integral operation assignment on a non-integral value");
	}
	return { RunResult::Next, 0 };
}

RunResult BreakStatement::Run(std::shared_ptr<Context> /*context*/) const {
	return { RunResult::Break, nPrecedingBreaks };
}

RunResult ContinueStatement::Run(std::shared_ptr<Context> /*context*/) const {
	return { RunResult::Continue, nPrecedingBreaks };
}

RunResult DoStatement::Run(std::shared_ptr<Context> outerContext) const {
	// Create a new context.
	auto context = std::make_shared<Context>(outerContext);

	// Run the statements until the condition is met.
	do {
		std::pair<RunResult, RunResultValue> runResult;
		if (BlockStatement::Run(context, runResult)) {
			return runResult;
		}
	} while (isWhile == AsBoolean(expression->GetValue(context)));
	return { RunResult::Next, 0 };
}

RunResult ExpressionStatement::Run(std::shared_ptr<Context> context) const {
	expression->GetValue(context);
	return { RunResult::Next, 0 };
}

RunResult ForStatement::Run(std::shared_ptr<Context> outerContext) const {
	// Create a new context.
	auto context = std::make_shared<Context>(outerContext);

	// Run initializer clauses.
	for (std::unique_ptr<Statement::Clause> const& initializerClause : initializerClauses) {
		initializerClause->Run(context);
	}
	for (;;) {
		// Run expression clauses, checking the value of the last one.
		Value finalValue = -1;
		for (std::unique_ptr<Statement::Clause> const& expressionClause : expressionClauses) {
			finalValue = expressionClause->Run(context);
		}
		if (!AsBoolean(finalValue)) {
			break;
		}

		// Run the statements in the body of the "for" loop.
		std::pair<RunResult, RunResultValue> runResult;
		if (BlockStatement::Run(context, runResult)) {
			return runResult;
		}

		// Run updater clauses.
		for (std::unique_ptr<Statement::Clause> const& updaterClause : updaterClauses) {
			updaterClause->Run(context);
		}
	}
	return { RunResult::Next, 0 };
}

RunResult FunctionStatement::Run(std::shared_ptr<Context> context) const {
	// Create a named function for this function statement.
	Value function = std::make_shared<ScriptFunction>(parameters, statements, context, yielding);

	// Add it to the context.
	context->AddValue(name, function, true);
	return { RunResult::Next, 0 };
}

RunResult IfStatement::Run(std::shared_ptr<Context> outerContext) const {
	// Check "if" and "else if" fragments, exiting if any runs.
	for (std::unique_ptr<IfStatement::Fragment> const& fragment : fragments) {
		std::shared_ptr<Context> context = fragment->IsMatch(outerContext);
		if (context) {
			return fragment->Run(context);
		}
	}
	if (!statements.empty()) {
		// Create a new context.
		auto context = std::make_shared<Context>(outerContext);

		// Run the "else" statements.
		return BlockStatement::Run(context);
	}
	return { RunResult::Next, 0 };
}

std::shared_ptr<Context> IfStatement::Fragment::IsMatch(std::shared_ptr<Context> outerContext) const {
	// Create a new context.
	auto context = std::make_shared<Context>(outerContext);

	// Run initializer clauses, checking the value of the last one.
	Value finalValue = -1;
	for (std::unique_ptr<Statement::Clause> const& initializerClause : initializerClauses) {
		finalValue = initializerClause->Run(context);
	}

	// Return the context if this fragment wants to run.
	return AsBoolean(finalValue) ? context : std::shared_ptr<Context>{};
}

RunResult ImportStatement::Run(std::shared_ptr<Context> context) const {
	context; // TODO
	return { RunResult::Next, 0 };
}

RunResult IncrementStatement::Run(std::shared_ptr<Context> context) const {
	Di(expression, isIncrement, context);
	return { RunResult::Next, 0 };
}

RunResult RangeForStatement::Run(std::shared_ptr<Context> outerContext) const {
	// Create an iterator for the range.
	auto iterator = Iterator(outerContext, ids, expression);

	// Run the statements in the body of the "for" loop.
	for (std::shared_ptr<Context> context; context = ++iterator, context;) {
		std::pair<RunResult, RunResultValue> runResult;
		if (BlockStatement::Run(context, runResult)) {
			return runResult;
		}
	}
	return { RunResult::Next, 0 };
}

RunResult ReturnStatement::Run(std::shared_ptr<Context> context) const {
	Value value = expression->GetValue(context);
	return { RunResult::Return, value };
}

RunResult SwitchStatement::Run(std::shared_ptr<Context> outerContext) const {
	// All "switch" statements and their enclosed "case" clauses introduce a new context.
	auto context = std::make_shared<Context>(outerContext);

	// Run initializer clauses, keeping the value of the last one.
	Value finalValue = -1;
	for (std::unique_ptr<Statement::Clause> const& initializerClause : initializerClauses) {
		finalValue = initializerClause->Run(context);
	}

	// Run the matching "case" clause.
	auto defaultCase = statements.end();
	for (auto it = statements.begin(); it != statements.end(); ++it) {
		auto const* p = dynamic_cast<SwitchStatement::Case const*>(it->get());
		if (p) {
			if (p->IsMatch(finalValue, context)) {
				std::pair<RunResult, RunResultValue> runResult;
				if (p->BlockStatement::Run(context, runResult)) {
					return runResult;
				} else if (runResult.first == RunResult::Continue) {
					throw std::runtime_error("unexpected continue in switch case");
				}
				return { RunResult::Next, 0 };
			}
		} else {
			defaultCase = it;
		}
	}

	// Run the "default" clause if any since no case matched.
	if (defaultCase != statements.end()) {
		std::pair<RunResult, RunResultValue> runResult;
		if (static_cast<BlockStatement*>((*defaultCase).get())->BlockStatement::Run(context, runResult)) {
			return runResult;
		} else if (runResult.first == RunResult::Continue) {
			throw std::runtime_error("unexpected continue in switch case");
		}
		return { RunResult::Next, 0 };
	}
	return { RunResult::Next, 0 };
}

bool SwitchStatement::Case::IsMatch(Value value, std::shared_ptr<Context> outerContext) const {
	auto context = std::make_shared<Context>(outerContext);
	Value expressionValue = expression->GetValue(context);
	return expressionValue == value;
}

RunResult VarStatement::Run(std::shared_ptr<Context> context) const {
	for (auto const& initializer : initializers) {
		auto&& [name, _, expression] = initializer;
		context->AddValue(name, expression->GetValue(context), isConstant);
	}
	return { RunResult::Next, 0 };
}

RunResult WhileStatement::Run(std::shared_ptr<Context> outerContext) const {
	for (;;) {
		// Create a new context.
		auto context = std::make_shared<Context>(outerContext);

		// Run initializer clauses, keeping the value of the last one.
		Value finalValue = -1;
		for (std::unique_ptr<Statement::Clause> const& initializerClause : initializerClauses) {
			finalValue = initializerClause->Run(context);
		}

		// Break out of the loop if the last initializer is falsy.
		if (!AsBoolean(finalValue)) {
			break;
		}

		// Run the statements.
		std::pair<RunResult, RunResultValue> runResult;
		if (BlockStatement::Run(context, runResult)) {
			return runResult;
		}
	}
	return { RunResult::Next, 0 };
}

RunResult YieldStatement::Run(std::shared_ptr<Context> context) const {
	Value value = expression->GetValue(context);
	return { RunResult::Yield, value };
}
