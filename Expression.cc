#include <stdexcept>
#include "JojobaScript.h"

Expression::~Expression() {}

Value& Expression::GetReference(std::shared_ptr<Context> context) {
	throw std::logic_error("cannot get a reference to an r-value expression");
}

Value AwaitExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

Value BinaryExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

Value BooleanExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

Value DictionaryExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

Value DictionaryComprehensionExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

Value& DotExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static Value n;
	return n;
}

Value DotExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

Value& IdentifierExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static Value n;
	return n;
}

Value IdentifierExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

Value& IndexExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static Value n;
	return n;
}

Value IndexExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

Value InvocationExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

Value LambdaExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

Value ListExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

Value ListComprehensionExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

Value NumericExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

Value SetExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

Value TernaryExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

Value UnaryExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}
