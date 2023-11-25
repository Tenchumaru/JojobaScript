#include <numeric>
#include <ranges>
#include <stdexcept>
#include "JojobaScript.h"
#include "parser.h"

Expression::~Expression() {}

Value& Expression::GetReference(std::shared_ptr<Context> context) {
	throw std::logic_error("cannot get a reference to an r-value expression");
}

Value AwaitExpression::GetValue(std::shared_ptr<Context> context) {
	// TODO:  Value does not yet contain any awaitable value.
	return expression->GetValue(context);
}

Value BinaryExpression::GetValue(std::shared_ptr<Context> context) {
	if (operation == AND) {
		auto leftValue = leftExpression->GetValue(context);
		return leftValue ? rightExpression->GetValue(context) : leftValue;
	} else if (operation == OR) {
		auto leftValue = leftExpression->GetValue(context);
		return leftValue ? leftValue : rightExpression->GetValue(context);
	} else if (operation == SS) {
		auto leftValue = leftExpression->GetValue(context);
		auto rightValue = rightExpression->GetValue(context);
		auto iota = std::ranges::iota_view{ 1ll, rightValue };
		return std::accumulate(iota.begin(), iota.end(), leftValue, [](auto a, auto b) { return a * b; });
	}
	auto leftValue = leftExpression->GetValue(context);
	auto rightValue = rightExpression->GetValue(context);
	switch (operation) {
	case EQ:
		return leftValue == rightValue;
	case NE:
		return leftValue != rightValue;
	case LE:
		return leftValue <= rightValue;
	case GE:
		return leftValue >= rightValue;
	case '<':
		return leftValue < rightValue;
	case '>':
		return leftValue > rightValue;
	case '+':
		return leftValue + rightValue;
	case '-':
		return leftValue - rightValue;
	case '&':
		return leftValue & rightValue;
	case '|':
		return leftValue | rightValue;
	case '^':
		return leftValue ^ rightValue;
	case '*':
		return leftValue * rightValue;
	case '/':
		return leftValue / rightValue;
	case '%':
		return leftValue % rightValue;
	case ASR:
		return leftValue >> rightValue;
	case LSR:
		return static_cast<Value>(static_cast<std::uint64_t>(leftValue) >> rightValue);
	case SL:
		return leftValue << rightValue;
	default:
		throw std::logic_error("unexpected binary operation");
	}
	return 0;
}

Value BooleanExpression::GetValue(std::shared_ptr<Context> /*context*/) {
	return value;
}

Value DictionaryExpression::GetValue(std::shared_ptr<Context> context) {
	// TODO:  Value does not yet contain a dictionary.
	context;
	return 0;
}

Value DictionaryComprehensionExpression::GetValue(std::shared_ptr<Context> context) {
	// TODO:  Value does not yet contain a dictionary.
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
	// TODO:  Value does not yet contain an indexable type.
	context;
	static Value n;
	return n;
}

Value IndexExpression::GetValue(std::shared_ptr<Context> context) {
	// TODO:  Value does not yet contain an indexable type.
	context;
	return 0;
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
	// TODO:  Value cannot yet contain a list.
	context;
	return 0;
}

Value ListComprehensionExpression::GetValue(std::shared_ptr<Context> context) {
	// TODO:  Value cannot yet contain a list.
	context;
	return 0;
}

Value NumericExpression::GetValue(std::shared_ptr<Context> /*context*/) {
	// TODO:  Value contains only an integer.
	return std::get<0>(value);
}

Value SetExpression::GetValue(std::shared_ptr<Context> context) {
	// TODO:  Value does not yet contain a set.
	context;
	return 0;
}

Value TernaryExpression::GetValue(std::shared_ptr<Context> context) {
	return expression->GetValue(context) ? trueExpression->GetValue(context) : falseExpression->GetValue(context);
}

Value UnaryExpression::GetValue(std::shared_ptr<Context> context) {
	auto value = expression->GetValue(context);
	switch (operation) {
	case '-':
		return -value;
	case '~':
		return ~value;
	case '!':
		return value == 0;
	default:
		throw std::logic_error("unexpected unary operation");
	}
	return 0;
}
