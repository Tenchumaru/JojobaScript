#include "pch.h"
#include "Context.h"
#include "FiberRunner.h"
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
	std::unique_ptr<FunctionStatement> program;
	std::string parserError = ParseFile(inputFile, program);
	if (!parserError.empty()) {
		std::cerr << parserError << std::endl;
		return 1;
	}

	// Run the program in a fiber.
	auto fn = [&program] {
		// Create the global context.
		auto globalContext = std::make_shared<Context>(std::shared_ptr<Context>());

		// Define the globals.
		globalContext->AddValue("print", std::make_shared<PrintFunction>(), true);
		auto thePassageOfTimeFunction = std::make_shared<ThePassageOfTimeFunction>();
		globalContext->AddValue("thePassageOfTime", thePassageOfTimeFunction, true);
		globalContext->AddValue("the_passage_of_time", thePassageOfTimeFunction, true);

		// Run the entry point.
		program->Run(globalContext);
		Value value = globalContext->GetValue("");
		if (std::holds_alternative<std::shared_ptr<Function>>(value)) {
			auto&& function = std::get<std::shared_ptr<Function>>(value);
			function->Invoke({});
		}
		};
	auto& fiberRunner = FiberRunner::get_Instance();
	fiberRunner.Launch(std::move(fn));
	fiberRunner.Run();

	return 0;
}
