#include "pch.h"
#include "Context.h"
#include "FiberRunner.h"
#include "Function.h"
#include "Statement.h"

int yyparse();

extern FILE* yyin;
extern FILE* yyout;

static int usage(char const* prog) {
	fprintf(stderr, "usage: %s [input.s [output.hex]]\n", prog);
	return 2;
}

int main(int argc, char* argv[]) {
	// Set the program name.
	char const* prog = strrchr(argv[0], '\\');
	if (prog)
		++prog;
	else
		prog = argv[0];

	// Collect the options.
	while (argc > 1 && argv[1][0] == '-' && argv[1][1] != '\0') {
		switch (argv[1][1]) {
		case 'x': // TODO
			break;
		default:
			return usage(prog);
		}
		--argc, ++argv;
	}

	// Set the input and output files.
	if (argc > 1) {
		if (strcmp("-", argv[1]) && fopen_s(&yyin, argv[1], "rt")) {
			fprintf(stderr, "%s: cannot open '%s' for reading\n", prog, argv[1]);
			return 1;
		}
		if (argc > 2) {
			if (fopen_s(&yyout, argv[2], "wt")) {
				fprintf(stderr, "%s: cannot open '%s' for writing\n", prog, argv[2]);
				return 1;
			}
		}
	}

	// Parse the input.
	int parseError = yyparse();
	if (parseError) {
		return parseError;
	}

	// Run the program in a fiber.
	auto fn = [] {
		// Create the global context.
		auto globalContext = std::make_shared<Context>(std::shared_ptr<Context>());

		// Define the globals.
		globalContext->AddValue("print", std::make_shared<PrintFunction>(), true);
		auto thePassageOfTimeFunction = std::make_shared<ThePassageOfTimeFunction>();
		globalContext->AddValue("thePassageOfTime", thePassageOfTimeFunction, true);
		globalContext->AddValue("the_passage_of_time", thePassageOfTimeFunction, true);

		// Run the entry point.
		FunctionStatement::program->Run(globalContext);
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
