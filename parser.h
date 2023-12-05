#pragma once

class FunctionStatement;

int ParseFile(char const* filePath, std::unique_ptr<FunctionStatement>& program_);
