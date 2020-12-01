#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "JojobaScript.h"

void add_symbol(char const* id, int expr /*= 0*/) {
	id, expr;
}

void emit(int expr /*= 0*/) {
	expr;
}

int get_value(char const* id) {
	id;
	return 0;
}

int yyparse();

extern FILE *yyin, *yyout;

static int usage(char const* prog) {
	fprintf(stderr, "usage: %s [input.s [output.hex]]\n", prog);
	return 2;
}

int main(int argc, char* argv[]) {
	// Set the program name.
	char const *prog = strrchr(argv[0], '\\');
	if(prog)
		++prog;
	else
		prog = argv[0];

	// Collect the options.
	while(argc > 1 && argv[1][0] == '-' && argv[1][1] != '\0') {
		switch(argv[1][1]) {
		case 'x': // TODO
			break;
		default:
			return usage(prog);
		}
		--argc, ++argv;
	}

	// Set the input and output files.
	if(argc > 1) {
		if(strcmp("-", argv[1]) && fopen_s(&yyin, argv[1], "rt")) {
			fprintf(stderr, "%s: cannot open '%s' for reading\n", prog, argv[1]);
			return 1;
		}
		if(argc > 2) {
			if(fopen_s(&yyout, argv[2], "wt")) {
				fprintf(stderr, "%s: cannot open '%s' for writing\n", prog, argv[2]);
				return 1;
			}
		}
	}

	// Parse the input.
	return yyparse();
}
