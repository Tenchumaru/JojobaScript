#include "Expression.h"
#include "Utilities.h"

Expression::~Expression() {}

AwaitExpression::~AwaitExpression() {
	Delete(expression);
}

BinaryExpression::~BinaryExpression() {
	Delete(leftExpression);
	Delete(rightExpression);
}

DictionaryExpression::~DictionaryExpression() {
	std::for_each(keyValuePairs.begin(), keyValuePairs.end(), [](auto& pair) { delete pair.first; delete pair.second; });
	keyValuePairs.clear();
}

DictionaryComprehensionExpression::~DictionaryComprehensionExpression() {
	Delete(keyExpression);
	Delete(valueExpression);
	Delete(sourceExpression);
}

InvocationExpression::~InvocationExpression() {
	Delete(expression);
	DeleteElements(arguments);
}

TernaryExpression::~TernaryExpression() {
	Delete(expression);
	Delete(trueExpression);
	Delete(falseExpression);
}

UnaryExpression::~UnaryExpression() {
	Delete(expression);
}

ListExpression::~ListExpression() {
	DeleteElements(expressions);
}

ListComprehensionExpression::~ListComprehensionExpression() {
	Delete(targetExpression);
	Delete(sourceExpression);
}

SetExpression::~SetExpression() {
	DeleteElements(expressions);
}
