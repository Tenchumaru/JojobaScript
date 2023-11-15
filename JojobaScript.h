#pragma once

enum class FunctionType { Unspecified, Standard, Asynchronous, Generator };

void add_symbol(char const* id, int expr = 0);
void emit(int expr = 0);
int get_value(char const* id);
