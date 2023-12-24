#include "pch.h"
#include "Context.h"
#include "JojobaFiberRunner.h"
#include "Function.h"
#include "Statement.h"
#include "parser.h"

int yyparse();

static void usage(char const* prog) {
	std::cerr << "usage: " << prog << " [-h] [-o output.txt] [--] [input.jjs]" << std::endl;
}

int main(int argc, char* argv[]) {
	// Set the program name.
	char const* prog = strrchr(argv[0], '\\');
	if (prog) {
		++prog;
	} else {
		prog = argv[0];
	}

	// Collect the options.
	char const* outputFilePath = nullptr;
	while (argc > 1 && argv[1][0] == '-') {
		switch (argv[1][1]) {
		case 'h':
			return usage(prog), 0;
		case 'o':
			--argc, ++argv;
			outputFilePath = argv[1];
			break;
		default:
			return usage(prog), 2;
		}
		--argc, ++argv;
		if (argv[0][1] == '-') {
			break;
		}
	}
	char const* inputFilePath = argv[1];

	// Open the input file if specified or use standard input.
	FILE* inputFile;
	if (inputFilePath) {
		if (fopen_s(&inputFile, inputFilePath, "rt")) {
			std::cerr << prog << ":  cannot open '" << inputFilePath << "' for reading" << std::endl;
			return 1;
		}
	} else {
		inputFile = stdin;
	}

	// Open the output file if specified or use standard output.
	std::ofstream outputFile;
	if (outputFilePath) {
		outputFile.open(outputFilePath);
		if (!outputFile) {
			std::cerr << prog << ":  cannot open '" << outputFilePath << "' for writing" << std::endl;
			return 1;
		}
		PrintFunction::outputStream = &outputFile;
	} else {
		PrintFunction::outputStream = &std::cout;
	}

	// Parse the input.
	std::string parserError = ParseFile(inputFile, FunctionStatement::programs[""]);
	if (!parserError.empty()) {
		std::cerr << parserError << std::endl;
		return 1;
	}

	// Run the program in a fiber.
	auto fn = [] { FunctionStatement::programs[""]->RunProgram(); };
	auto fiberRunner = JojobaFiberRunner::Get();
	if (fiberRunner->Launch(std::move(fn))) {
		return 1;
	}
	fiberRunner->Run();

	return 0;
}
