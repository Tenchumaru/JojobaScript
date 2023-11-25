#include <stdexcept>
#include "JojobaScript.h"

Expression::~Expression() {}

int& Expression::GetReference(std::shared_ptr<Context> context) {
	throw std::logic_error("cannot get a reference to an r-value expression");
}

int AwaitExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int BinaryExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int BooleanExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int DictionaryExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int DictionaryComprehensionExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int& DotExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static int n;
	return n;
}

int DotExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int& IdentifierExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static int n;
	return n;
}

int IdentifierExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int& IndexExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static int n;
	return n;
}

int IndexExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int InvocationExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int LambdaExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int ListExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int ListComprehensionExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int NumericExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int SetExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int TernaryExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int UnaryExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}
