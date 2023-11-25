#include "JojobaScript.h"

Expression::~Expression() {}

int& AwaitExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static int n;
	return n;
}

int AwaitExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int& BinaryExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static int n;
	return n;
}

int BinaryExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int& BooleanExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static int n;
	return n;
}

int BooleanExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int& DictionaryExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static int n;
	return n;
}

int DictionaryExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int& DictionaryComprehensionExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static int n;
	return n;
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

int& InvocationExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static int n;
	return n;
}

int InvocationExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int& LambdaExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static int n;
	return n;
}

int LambdaExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int& ListExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static int n;
	return n;
}

int ListExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int& ListComprehensionExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static int n;
	return n;
}

int ListComprehensionExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int& NumericExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static int n;
	return n;
}

int NumericExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int& SetExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static int n;
	return n;
}

int SetExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int& TernaryExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static int n;
	return n;
}

int TernaryExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}

int& UnaryExpression::GetReference(std::shared_ptr<Context> context) {
	context;
	static int n;
	return n;
}

int UnaryExpression::GetValue(std::shared_ptr<Context> context) {
	context;
	return 0;
}
