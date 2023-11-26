#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <ranges>
#include <stdexcept>
#include "JojobaScript.h"
#include "parser.h"

namespace {
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
Expression::~Expression() {}

Value& Expression::GetReference(std::shared_ptr<Context> context) {
	throw std::runtime_error("cannot get a reference to an r-value expression");
}

Value AwaitExpression::GetValue(std::shared_ptr<Context> context) {
	// TODO:  Value does not yet contain any awaitable value.
	return expression->GetValue(context);
}

Value BinaryExpression::GetValue(std::shared_ptr<Context> context) {
	if (operation == AND) {
		auto leftValue = leftExpression->GetValue(context);
		return AsBoolean(leftValue) ? rightExpression->GetValue(context) : leftValue;
	} else if (operation == OR) {
		auto leftValue = leftExpression->GetValue(context);
		return AsBoolean(leftValue) ? leftValue : rightExpression->GetValue(context);
	}
	auto leftValue = leftExpression->GetValue(context);
	auto rightValue = rightExpression->GetValue(context);
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

Value DictionaryComprehensionExpression::GetValue(std::shared_ptr<Context> context) {
	// TODO
	context;
	return 0;
}

Value& DotExpression::GetReference(std::shared_ptr<Context> context) {
	// TODO:  Value does not yet contain a type with fields.
	context;
	static Value n;
	return n;
}

Value DotExpression::GetValue(std::shared_ptr<Context> context) {
	// TODO:  Value does not yet contain a type with fields.
	context;
	return 0;
}

Value& IdentifierExpression::GetReference(std::shared_ptr<Context> context) {
	return context->GetReference(id);
}

Value IdentifierExpression::GetValue(std::shared_ptr<Context> context) {
	return context->GetValue(id);
}

Value& IndexExpression::GetReference(std::shared_ptr<Context> context) {
	// The only indexable types are dictionaries and lists.
	auto indexedValue = indexedExpression->GetValue(context);
	if (std::holds_alternative<std::shared_ptr<List>>(indexedValue)) {
		auto indexingValue = indexingExpression->GetValue(context);
		if (std::holds_alternative<std::int64_t>(indexingValue)) {
			auto&& list = *std::get<std::shared_ptr<List>>(indexedValue);
			std::int64_t index = std::get<std::int64_t>(indexingValue);
			return list[index];
		}
		throw std::runtime_error("cannot index list with non-integral value");
	} else if (std::holds_alternative<std::shared_ptr<Dictionary>>(indexedValue)) {
		auto indexingValue = indexingExpression->GetValue(context);
		auto&& dictionary = *std::get<std::shared_ptr<Dictionary>>(indexedValue);
		return dictionary[indexingValue];
	}
	throw std::runtime_error("cannot index non-indexable");
}

Value IndexExpression::GetValue(std::shared_ptr<Context> context) {
	return GetReference(context);
}

Value InvocationExpression::GetValue(std::shared_ptr<Context> context) {
	// TODO:  Value does not yet contain an invocable type.
	context;
	return 0;
}

Value LambdaExpression::GetValue(std::shared_ptr<Context> context) {
	// TODO:  Value does not yet contain an invocable type.
	context;
	return 0;
}

Value ListExpression::GetValue(std::shared_ptr<Context> context) {
	List values;
	std::ranges::transform(expressions, std::back_inserter(values), [&context](auto&& p) { return p->GetValue(context); });
	return std::make_shared<List>(std::move(values));
}

Value ListComprehensionExpression::GetValue(std::shared_ptr<Context> context) {
	// TODO
	context;
	return 0;
}

Value LiteralExpression::GetValue(std::shared_ptr<Context> /*context*/) {
	// Value contains a number or string.
	return value;
}

Value SetExpression::GetValue(std::shared_ptr<Context> context) {
	// TODO:  Value does not yet contain a set.
	context;
	return 0;
}

Value TernaryExpression::GetValue(std::shared_ptr<Context> context) {
	return AsBoolean(expression->GetValue(context)) ? trueExpression->GetValue(context) : falseExpression->GetValue(context);
}

Value UnaryExpression::GetValue(std::shared_ptr<Context> context) {
	auto value = expression->GetValue(context);
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
