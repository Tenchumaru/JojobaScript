#pragma once

class FunctionStatement;

bool ParseFile(FILE* inputFile, std::unique_ptr<FunctionStatement>& program_);
