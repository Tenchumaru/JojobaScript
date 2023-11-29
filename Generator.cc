#include "pch.h"
#include "Context.h"
#include "Expression.h"
#include "Generator.h"
#include "Statement.h"

Value Generator::operator++() {
	auto context = ++iterator;
	if (!context) {
		return nullptr;
	}
	Value targetValue = targetExpression->GetValue(context);
	return targetValue;
}
