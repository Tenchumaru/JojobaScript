#include "pch.h"
#include "Context.h"
#include "FiberRunner.h"
#include "Function.h"
#include "Statement.h"
#include "parser.h"

int yyparse();

static void usage(char const* prog) {
	fprintf(stderr, "usage: %s [input.s [output.hex]]\n", prog);
}

int main(int argc, char* argv[]) {
	// Set the program name.
	char const* prog = strrchr(argv[0], '\\');
	if (prog)
		++prog;
	else
		prog = argv[0];

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

	// Parse the input.
	std::unique_ptr<FunctionStatement> program;
	switch (ParseFile(argv[1], program)) {
	case -1:
		fprintf(stderr, "%s: cannot open '%s' for reading\n", prog, argv[1]);
		__fallthrough;
	case 1:
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
