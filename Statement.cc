#include "pch.h"
#define NOMINMAX
#include <Windows.h>
#include "Statement.h"
#include "JojobaFiberRunner.h"
#include "Function.h"
#include "Iterator.h"
#include "Generator.h"
#include "parser.h"

using RunResult = std::pair<Statement::RunResult, Statement::RunResultValue>;

std::unordered_map<std::string, std::unique_ptr<FunctionStatement>> FunctionStatement::programs;

namespace {
	std::array<std::function<void(ValueReference&, Value const&)>, static_cast<int>(Assignment::Size)> map = {
		/* =    */ [](ValueReference& a, Value const& b) { a = b; },
		/* &=   */ [](ValueReference& a, Value const& b) { a &= b; },
		/* >>>= */ std::function<void(ValueReference&, Value const&)>{},
		/* /=   */ [](ValueReference& a, Value const& b) { a /= b; },
		/* <<=  */ [](ValueReference& a, Value const& b) { a <<= b; },
		/* >>=  */ [](ValueReference& a, Value const& b) { a >>= b; },
		/* %=   */ [](ValueReference& a, Value const& b) { a %= b; },
		/* |=   */ [](ValueReference& a, Value const& b) { a |= b; },
		/* +=   */ [](ValueReference& a, Value const& b) { a += b; },
		/* -=   */ [](ValueReference& a, Value const& b) { a -= b; },
		/* *=   */ [](ValueReference& a, Value const& b) { a *= b; },
		/* ^=   */ [](ValueReference& a, Value const& b) { a ^= b; },
	};

	void Assign(ValueReference& targetReference, Assignment assignment, Value const& sourceValue) {
		if (assignment != Assignment::LSRA) {
			map[static_cast<int>(assignment)](targetReference, sourceValue);
		} else {
			if (std::holds_alternative<std::int64_t>(targetReference) && std::holds_alternative<std::int64_t>(sourceValue)) {
				auto& targetValue = std::get<std::int64_t>(targetReference);
				targetValue = static_cast<std::int64_t>(static_cast<std::uint64_t>(targetValue) >> std::get<std::int64_t>(sourceValue));
			} else {
				throw std::runtime_error("cannot perform integral compound assignment on a non-integral value");
			}
		}
	}

	Value Assign(std::vector<std::unique_ptr<Expression>> const& targetExpressions, Assignment assignment, std::vector<std::unique_ptr<Expression>> const& sourceExpressions, std::shared_ptr<Context> context) {
		// Check if the number of sources matches the number of targets.
		size_t nsourceExpressions = sourceExpressions.size();
		if (nsourceExpressions == targetExpressions.size()) {
			// Check identifier assignments for constancy.
			for (std::unique_ptr<Expression> const& targetExpression : targetExpressions) {
				auto const* identifierExpression = dynamic_cast<IdentifierExpression const*>(targetExpression.get());
				if (identifierExpression && identifierExpression->IsConstant(context)) {
					throw std::runtime_error("cannot assign constant");
				}
			}

			// Collect the source values.  Performing this first in its own loop enables the "swap" paradigm.
			std::vector<Value> sourceValues;
			for (std::unique_ptr<Expression> const& sourceExpression : sourceExpressions) {
				sourceValues.emplace_back(sourceExpression->GetValue(context));
			}

			// Assign the sources to the targets.
			for (size_t i = 0; i < nsourceExpressions; ++i) {
				Value const& sourceValue = sourceValues[i];
				ValueReference targetReference = targetExpressions[i]->GetReference(context);
				Assign(targetReference, assignment, sourceValue);
			}
			return sourceValues.back();
		} else if (targetExpressions.size() == 1) {
			// There are multiple sources and one target.  Ensure this is a non-compound assignment.
			if (assignment == Assignment{}) {
				// Check identifier assignment for constancy.
				auto const* identifierExpression = dynamic_cast<IdentifierExpression const*>(targetExpressions.back().get());
				if (identifierExpression && identifierExpression->IsConstant(context)) {
					throw std::runtime_error("cannot assign constant");
				}

				// Create a list of source values.
				auto list = std::make_shared<List>();
				for (std::unique_ptr<Expression> const& sourceExpression : sourceExpressions) {
					list->emplace_back(sourceExpression->GetValue(context));
				}

				// Assign that list to the target.
				ValueReference targetReference = targetExpressions.back()->GetReference(context);
				targetReference = list;
				return targetReference;
			}
			throw std::runtime_error("cannot perform compound assignment on a list");
		} else if (nsourceExpressions == 1) {
			// There are one source and multiple targets.  Check if the source is a list.
			Value const& sourceValue = sourceExpressions.back()->GetValue(context);
			if (std::holds_alternative<std::shared_ptr<List>>(sourceValue)) {
				// Create a copy of the list to ensure an element in both the source and target collections does not inadvertently cause an
				// unexpected update.
				List list = *std::get<std::shared_ptr<List>>(sourceValue);

				// Check if the number of source elements matches the number of targets.
				if (list.size() == targetExpressions.size()) {
					// Assign the source elements to the targets.
					for (size_t i = 0, n = targetExpressions.size(); i < n; ++i) {
						ValueReference targetReference = targetExpressions[i]->GetReference(context);
						Assign(targetReference, assignment, list[i]);
					}
					return list.back();
				}
			}
		}
		throw std::runtime_error("the number of sources and targets does not match");
	}

	Value Di(std::unique_ptr<Expression> const& expression, bool isIncrement, std::shared_ptr<Context> context) {
		ValueReference reference = expression->GetReference(context);
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

Statement::~Statement() {}

RunResult BlockStatement::Run(std::shared_ptr<Context> context) const {
	for (std::unique_ptr<Statement> const& statement : statements) {
		::RunResult runResult = statement->Run(context);
		if (runResult.first != RunResult::Next) {
			return runResult;
		}
	}
	return { RunResult::Next, 0 };
}

bool BlockStatement::Run(std::shared_ptr<Context> context, std::pair<RunResult, RunResultValue>& runResult, bool allowFallthrough /*= false*/) const {
	// Run the block.
	runResult = BlockStatement::Run(context);

	// Return true to indicate a block-exiting condition, i.e., break, return, or yield.  Return false otherwise.
	switch (runResult.first) {
	case RunResult::Next:
		return false;
	case RunResult::Break:
		if (std::get<size_t>(runResult.second)) {
			--std::get<size_t>(runResult.second);
		} else {
			runResult = { RunResult::Next, 0 };
		}
		return true;
	case RunResult::Continue:
		if (std::get<size_t>(runResult.second)) {
			--std::get<size_t>(runResult.second);
			return true;
		}
		return false;
	case RunResult::Fallthrough:
		if (!allowFallthrough) {
			throw std::runtime_error("illegal fallthrough");
		}
		return false;
	case RunResult::Return:
	case RunResult::Throw:
	case RunResult::Yield:
		return true;
	default:
		throw std::logic_error("unexpected RunResult value");
	}
}

ForStatement::Clause::~Clause() {}

Statement::AssignmentClause::AssignmentClause(Expression* targetExpression, Assignment assignment, Expression* sourceExpression) : assignment(assignment) {
	targetExpressions.emplace_back(std::unique_ptr<Expression>(targetExpression));
	sourceExpressions.emplace_back(std::unique_ptr<Expression>(sourceExpression));
}

Value Statement::AssignmentClause::Run(std::shared_ptr<Context> context) const {
	return Assign(targetExpressions, assignment, sourceExpressions, context);
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
	Assign(targetExpressions, assignment, sourceExpressions, context);
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

RunResult FallthroughStatement::Run(std::shared_ptr<Context> context) const {
	return { RunResult::Fallthrough, 0 };
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

std::shared_ptr<Context> FunctionStatement::RunProgram() const {
	// Create the global context.
	auto globalContext = std::make_shared<Context>(std::shared_ptr<Context>());

	// Define the globals.
	globalContext->AddValue("float", std::make_shared<FloatFunction>(), true);
	globalContext->AddValue("input", std::make_shared<InputFunction>(), true);
	globalContext->AddValue("int", std::make_shared<IntFunction>(), true);
	globalContext->AddValue("print", std::make_shared<PrintFunction>(), true);
	globalContext->AddValue("string", std::make_shared<StringFunction>(), true);
	auto thePassageOfTimeFunction = std::make_shared<ThePassageOfTimeFunction>();
	globalContext->AddValue("thePassageOfTime", thePassageOfTimeFunction, true);
	globalContext->AddValue("the_passage_of_time", thePassageOfTimeFunction, true);

	// Run the entry point.
	Run(globalContext);
	Value value = globalContext->GetValue("");
	if (std::holds_alternative<std::shared_ptr<Function>>(value)) {
		auto&& function = std::get<std::shared_ptr<Function>>(value);
		return function->Invoke({}).second;
	}
	return {};
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

	// Run initializer clauses if any.
	if(initializers) {
		initializers->Run(context);
	}

	// Check the value of the expression.
	Value finalValue = expression->GetValue(context);

	// Return the context if this fragment wants to run.
	return AsBoolean(finalValue) ? context : std::shared_ptr<Context>{};
}

RunResult ImportStatement::Run(std::shared_ptr<Context> context) const {
	// If this module is already loaded, skip loading but add new names.
	std::shared_ptr<Context> moduleContext;
	auto it = Context::modules.find(moduleName);
	if (it == Context::modules.end()) {
		// Open the input file.
		FILE* inputFile;
		std::string filePath = moduleName + ".jjs";
		if (fopen_s(&inputFile, filePath.c_str(), "rt")) {
			throw std::runtime_error("cannot open module for reading");
		}

		// Parse imported file.
		std::unique_ptr<FunctionStatement> program;
		std::string parserError = ParseFile(inputFile, program);
		if (!parserError.empty()) {
			throw std::runtime_error(parserError);
		}

		// Run the module as a program.
		moduleContext = program->RunProgram();
		Context::modules[moduleName] = moduleContext;
		FunctionStatement::programs[moduleName] = std::move(program);
	} else {
		moduleContext = it->second;
	}

	// Add its defined symbols to the context.
	if (pairs.empty()) {
		context->AddContext(alias.empty() ? moduleName : alias, moduleContext);
	} else {
		// Add each defined symbol to the context.
		for (auto& pair : pairs) {
			std::string const& key = pair.second.empty() ? pair.first : pair.second;
			context->AddValue(key, moduleContext->GetValue(pair.first), true);
		}
	}
	return { RunResult::Next, 0 };
}

std::string ImportStatement::Validate(std::string&& moduleName) {
	if (!moduleName.empty()) {
		if (std::filesystem::exists(moduleName + ".jjs")) {
			return moduleName;
		}
		throw std::runtime_error("cannot find module");
	}
	throw std::runtime_error("module name cannot be empty");
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
	if (expression) {
		Value value = expression->GetValue(context);
		return { RunResult::Return, value };
	}
	return { RunResult::Return, nullptr };
}

RunResult SwitchStatement::Run(std::shared_ptr<Context> outerContext) const {
	// All "switch" statements and their enclosed "case" clauses introduce a new context.
	auto context = std::make_shared<Context>(outerContext);

	// Run initializer clauses, keeping the value of the last one.
	Value finalValue = -1;
	for (std::unique_ptr<Statement::Clause> const& initializerClause : initializerClauses) {
		finalValue = initializerClause->Run(context);
	}

	// Run the matching "case" clause.  Allow falling through all the way through the default clause.
	auto defaultCase = statements.end();
	bool fallingThrough = false;
	for (auto it = statements.begin(); it != statements.end(); ++it) {
		auto const* p = dynamic_cast<SwitchStatement::Case const*>(it->get());
		if (p) {
			if (fallingThrough || p->IsMatch(finalValue, context)) {
				fallingThrough = false;
				std::pair<RunResult, RunResultValue> runResult;
				if (p->BlockStatement::Run(context, runResult, true)) {
					return runResult;
				} else if (runResult.first == RunResult::Fallthrough) {
					fallingThrough = true;
					continue;
				} else if (runResult.first == RunResult::Continue) {
					throw std::runtime_error("unexpected continue in switch case");
				}
				return { RunResult::Next, 0 };
			}
		} else {
			defaultCase = it;
			if (fallingThrough) {
				break;
			}
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

RunResult ThrowStatement::Run(std::shared_ptr<Context> context) const {
	if (expression) {
		Value value = expression->GetValue(context);
		return { RunResult::Throw, value };
	}
	return { RunResult::Throw, context->GetValue("exception") };
}

RunResult TryStatement::Run(std::shared_ptr<Context> outerContext) const {
	// Create a new context for the "try" block.
	auto context = std::make_shared<Context>(outerContext);

	// Run the "try" block.
	std::pair<RunResult, RunResultValue> runResult;
	try {
		runResult = BlockStatement::Run(context);
	} catch (std::runtime_error const& ex) {
		runResult.first = RunResult::Throw;
		runResult.second = Value(ex.what());
	}
	if (runResult.first == RunResult::Throw) {
		// Create a new context for the "catch" block.
		auto catchContext = std::make_shared<Context>(outerContext);

		// Add the throw value to the context with the name "exception".
		catchContext->AddValue("exception", std::get<Value>(runResult.second), true);

		// Run the "catch" block.
		std::pair<RunResult, RunResultValue> catchRunResult = catchStatements.Run(catchContext);
		if (catchRunResult.first == RunResult::Throw) {
			if (!std::holds_alternative<nullptr_t>(std::get<Value>(catchRunResult.second))) {
				// This is a new throw.  Replace the "try" block run result value.
				runResult.second = catchRunResult.second;
			}
		} else {
			// The "catch" block exited without throwing.  Use its run result.
			runResult = catchRunResult;
		}
	}

	// Create a new context for the "finally" block.
	auto finallyContext = std::make_shared<Context>(outerContext);

	// Run the "finally" block.  Use its run result if it exited early.
	std::pair<RunResult, RunResultValue> finallyRunResult = finallyStatements.Run(finallyContext);
	return finallyRunResult.first == RunResult::Next ? runResult : finallyRunResult;
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

YieldStatement::YieldStatement(Expression* expression) : expression(expression), waitHandle(CreateEvent(nullptr, TRUE, TRUE, nullptr)) {}

YieldStatement::~YieldStatement() {
	CloseHandle(waitHandle);
}

RunResult YieldStatement::Run(std::shared_ptr<Context> context) const {
	Value value = expression->GetValue(context);
	context->SetValue("$yield", value);
	JojobaFiberRunner::Get()->Await(waitHandle);
	return { RunResult::Next, 0 };
}
