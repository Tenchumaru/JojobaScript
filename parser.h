#pragma once

class FunctionStatement;

std::string ParseFile(FILE* inputFile, std::unique_ptr<FunctionStatement>& program_);
