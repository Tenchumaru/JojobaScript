#include "pch.h"
#include "Expression.h"
#include "Function.h"
#include "Generator.h"
#include "Iterator.h"
#include "Statement.h"
#include "parser.h"

namespace {
	template<typename T>
	std::shared_ptr<T> EvaluateComprehensionExpression(std::shared_ptr<Context> context, std::unique_ptr<Expression> const& targetExpression, std::vector<std::pair<std::string, std::string>> const& ids, std::unique_ptr<Expression> const& sourceExpression, std::function<void(T&, Value&&)> fn) {
		// Create a generator from the source expression.
		auto generator = IteratorGenerator(context, targetExpression, ids, sourceExpression);

		// Fill a collection of target values from the generator.
		T targetValues{};
		for (Value targetValue; targetValue = ++generator, !std::holds_alternative<nullptr_t>(targetValue);) {
			fn(targetValues, std::move(targetValue));
		}
		return std::make_shared<T>(std::move(targetValues));
	}

	std::int64_t EvaluateIntegralExponentiation(std::int64_t left, std::int64_t right) {
		if (right < 0) {
			return 0;
		} else if (right == 0) {
			return 1;
		} else if (right & 1) {
			return left * EvaluateIntegralExponentiation(left * left, (right - 1) >> 1);
		}
		return EvaluateIntegralExponentiation(left * left, right >> 1);
	}

	Value PerformNumericBinaryOperation(Value const& leftValue, Value const& rightValue, std::function<std::int64_t(std::int64_t, std::int64_t)> integralOperation, std::function<double(double, double)> realOperation) {
		if ((!std::holds_alternative<std::int64_t>(leftValue) && !std::holds_alternative<double>(leftValue)) || (!std::holds_alternative<std::int64_t>(rightValue) && !std::holds_alternative<double>(rightValue))) {
			throw std::runtime_error("cannot use non-numeric in a numeric operation");
		} else if (std::holds_alternative<std::int64_t>(leftValue) && std::holds_alternative<std::int64_t>(rightValue)) {
			return integralOperation(std::get<std::int64_t>(leftValue), std::get<std::int64_t>(rightValue));
		} else if (!realOperation) {
			throw std::runtime_error("cannot use non-integer in an integral operation");
		} else {
			auto leftReal = std::holds_alternative<double>(leftValue) ? std::get<double>(leftValue) : static_cast<double>(std::get<std::int64_t>(leftValue));
			auto rightReal = std::holds_alternative<double>(rightValue) ? std::get<double>(rightValue) : static_cast<double>(std::get<std::int64_t>(rightValue));
			return realOperation(leftReal, rightReal);
		}
	}

	Value PerformNumericUnaryOperation(Value const& value, std::function<std::int64_t(std::int64_t)> integralOperation, std::function<double(double)> realOperation) {
		if (std::holds_alternative<std::int64_t>(value)) {
			return integralOperation(std::get<std::int64_t>(value));
		} else if (!std::holds_alternative<double>(value)) {
			throw std::runtime_error("cannot use non-numeric in a numeric operation");
		} else if (!realOperation) {
			throw std::runtime_error("cannot use non-integer in an integral operation");
		} else {
			return realOperation(std::get<double>(value));
		}
	}
}

std::int64_t AdjustIndex(std::int64_t index, size_t size) {
	if (index < 0) {
		index += size;
	}
	if (static_cast<std::uint64_t>(index) >= size) {
		throw std::runtime_error("index out of range");
	}
	return index;
}

Expression::~Expression() {}

ValueReference Expression::GetReference(std::shared_ptr<Context> context) {
	throw std::runtime_error("cannot get a reference to an r-value expression");
}

Value AwaitExpression::GetValue(std::shared_ptr<Context> context) {
	// TODO:  Value does not yet contain any awaitable value.
	return expression->GetValue(context);
}

Value BinaryExpression::GetValue(std::shared_ptr<Context> context) {
	if (operation == AND) {
		Value leftValue = leftExpression->GetValue(context);
		return AsBoolean(leftValue) ? rightExpression->GetValue(context) : leftValue;
	} else if (operation == OR) {
		Value leftValue = leftExpression->GetValue(context);
		return AsBoolean(leftValue) ? leftValue : rightExpression->GetValue(context);
	}
	Value leftValue = leftExpression->GetValue(context);
	Value rightValue = rightExpression->GetValue(context);
	switch (operation) {
	case EQ:
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return a == b; }, [](auto a, auto b) { return a == b; });
	case NE:
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return a != b; }, [](auto a, auto b) { return a != b; });
	case LE:
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return a <= b; }, [](auto a, auto b) { return a <= b; });
	case GE:
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return a >= b; }, [](auto a, auto b) { return a >= b; });
	case '<':
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return a < b; }, [](auto a, auto b) { return a < b; });
	case '>':
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return a > b; }, [](auto a, auto b) { return a > b; });
	case '+':
		if (std::holds_alternative<std::string>(leftValue) && std::holds_alternative<std::string>(rightValue)) {
			return std::get<std::string>(leftValue) + std::get<std::string>(rightValue);
		}
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return a + b; }, [](auto a, auto b) { return a + b; });
	case '-':
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return a - b; }, [](auto a, auto b) { return a - b; });
	case '&':
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return a & b; }, {});
	case '|':
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return a | b; }, {});
	case '^':
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return a ^ b; }, {});
	case '*':
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return a * b; }, [](auto a, auto b) { return a * b; });
	case '/':
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return a / b; }, [](auto a, auto b) { return a / b; });
	case '%':
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return a % b; }, {});
	case ASR:
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return a >> b; }, {});
	case LSR:
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return static_cast<std::int64_t>(static_cast<std::uint64_t>(a) >> b); }, {});
	case SL:
		return PerformNumericBinaryOperation(leftValue, rightValue, [](auto a, auto b) { return a << b; }, {});
	case SS:
		return PerformNumericBinaryOperation(leftValue, rightValue, EvaluateIntegralExponentiation, [](auto a, auto b) { return std::pow(a, b); });
	default:
		throw std::logic_error("unexpected binary operation");
	}
}

Value DictionaryExpression::GetValue(std::shared_ptr<Context> context) {
	Dictionary values;
	std::ranges::transform(keyValuePairs, std::inserter(values, values.end()), [&context](auto&& p) {
		return std::make_pair(p.first->GetValue(context), p.second->GetValue(context)); });
	return std::make_shared<Dictionary>(std::move(values));
}

Value DictionaryComprehensionExpression::GetValue(std::shared_ptr<Context> outerContext) {
	// Create an iterator to produce contexts that contain values for the targets.
	auto iterator = Iterator(outerContext, ids, sourceExpression);

	// Construct and return a dictionary using the target values to generate the key-value pairs.
	Dictionary rv;
	for (std::shared_ptr<Context> context; context = ++iterator, context;) {
		Value keyValue = keyExpression->GetValue(context);
		Value valueValue = valueExpression->GetValue(context);
		rv.insert(std::make_pair(std::move(keyValue), std::move(valueValue)));
	}
	return std::make_shared<Dictionary>(std::move(rv));
}

ValueReference DotExpression::GetReference(std::shared_ptr<Context> context) {
	// TODO:  Value does not yet contain a type with fields.
	context;
	static Value n = 1;
	return n;
}

Value DotExpression::GetValue(std::shared_ptr<Context> context) {
	// TODO:  Value does not yet contain a type with fields.
	context;
	return 1;
}

Value GeneratorExpression::GetValue(std::shared_ptr<Context> context) {
	return std::make_shared<IteratorGenerator>(context, targetExpression, ids, sourceExpression);
}

ValueReference IdentifierExpression::GetReference(std::shared_ptr<Context> context) {
	return context->GetReference(id);
}

Value IdentifierExpression::GetValue(std::shared_ptr<Context> context) {
	return context->GetValue(id);
}

bool IdentifierExpression::IsConstant(std::shared_ptr<Context> context) const {
	return context->IsConstant(id);
}

ValueReference IndexExpression::GetReference(std::shared_ptr<Context> context) {
	// The only indexable types are dictionaries, lists, and strings.
	ValueReference indexedValue = indexedExpression->GetReference(context);
	if (std::holds_alternative<std::shared_ptr<List>>(indexedValue)) {
		Value indexingValue = indexingExpression->GetValue(context);
		if (std::holds_alternative<std::int64_t>(indexingValue)) {
			std::int64_t index = std::get<std::int64_t>(indexingValue);
			List& list = *std::get<std::shared_ptr<List>>(indexedValue);
			index = AdjustIndex(index, list.size());
			return list[index];
		}
		throw std::runtime_error("cannot index list with non-integral value");
	} else if (std::holds_alternative<std::shared_ptr<Dictionary>>(indexedValue)) {
		Value indexingValue = indexingExpression->GetValue(context);
		Dictionary& dictionary = *std::get<std::shared_ptr<Dictionary>>(indexedValue);
		if (dictionary.find(indexingValue) == dictionary.end()) {
			throw std::runtime_error("indexing value not found in dictionary");
		}
		return dictionary[indexingValue];
	} else if (std::holds_alternative<std::string>(indexedValue)) {
		Value indexingValue = indexingExpression->GetValue(context);
		if (std::holds_alternative<std::int64_t>(indexingValue)) {
			std::int64_t index = std::get<std::int64_t>(indexingValue);
			auto& string = std::get<std::string>(indexedValue);
			index = AdjustIndex(index, string.size());
			return ValueReference(string, index);
		}
		throw std::runtime_error("cannot index string with non-integral value");
	}
	throw std::runtime_error("cannot index non-indexable");
}

Value IndexExpression::GetValue(std::shared_ptr<Context> context) {
	return GetReference(context);
}

Value InvocationExpression::GetValue(std::shared_ptr<Context> context) {
	Value value = expression->GetValue(context);
	if (std::holds_alternative<std::shared_ptr<Function>>(value)) {
		std::vector<Value> values;
		std::ranges::transform(arguments, std::back_inserter(values), [&context](auto const& argument) {
			return argument->GetValue(context); });
		return std::get<std::shared_ptr<Function>>(value)->Invoke(values);
	}
	throw std::runtime_error("cannot invoke non-invocable");
}

Value LambdaExpression::GetValue(std::shared_ptr<Context> context) {
	return std::make_shared<ScriptFunction>(parameters, statements, context, yielding);
}

std::vector<std::unique_ptr<Statement>> LambdaExpression::Convert(Expression* expression) {
	std::vector<std::unique_ptr<Statement>> statements;
	auto p = std::make_unique<ReturnStatement>(expression);
	statements.emplace_back(std::move(p));
	return statements;
}

Value ListExpression::GetValue(std::shared_ptr<Context> context) {
	List values;
	std::ranges::transform(expressions, std::back_inserter(values), [&context](auto&& p) { return p->GetValue(context); });
	return std::make_shared<List>(std::move(values));
}

Value ListComprehensionExpression::GetValue(std::shared_ptr<Context> context) {
	auto fn = [](List& list, Value&& value) { list.emplace_back(std::move(value)); };
	return EvaluateComprehensionExpression<List>(context, targetExpression, ids, sourceExpression, fn);
}

Value LiteralExpression::GetValue(std::shared_ptr<Context> /*context*/) {
	// Value contains a number or string.
	return value;
}

Value SetExpression::GetValue(std::shared_ptr<Context> context) {
	Set values;
	std::ranges::transform(expressions, std::inserter(values, values.end()), [&context](auto&& p) { return p->GetValue(context); });
	return std::make_shared<Set>(std::move(values));
}

Value SetComprehensionExpression::GetValue(std::shared_ptr<Context> context) {
	auto fn = [](Set& set, Value&& value) { set.emplace(std::move(value)); };
	return EvaluateComprehensionExpression<Set>(context, targetExpression, ids, sourceExpression, fn);
}

Value TernaryExpression::GetValue(std::shared_ptr<Context> context) {
	return AsBoolean(expression->GetValue(context)) ? trueExpression->GetValue(context) : falseExpression->GetValue(context);
}

Value UnaryExpression::GetValue(std::shared_ptr<Context> context) {
	Value value = expression->GetValue(context);
	switch (operation) {
	case '-':
		return PerformNumericUnaryOperation(value, [](auto v) { return -v; }, [](auto v) { return -v; });
	case '~':
		return PerformNumericUnaryOperation(value, [](auto v) { return ~v; }, {});
	case '!':
		return !AsBoolean(value);
	default:
		throw std::logic_error("unexpected unary operation");
	}
}
