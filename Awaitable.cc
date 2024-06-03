#include "pch.h"
#include "ValueReference.h"
#include "JojobaFiberRunner.h"
#include "Awaitable.h"

Value Awaitable::Await() {
	JojobaFiberRunner::Get()->Await(handle);
	return fn(handle);
}
