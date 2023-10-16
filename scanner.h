#pragma once

enum Assignment {
	AA = 1, ARA, DA, MIA, MOA, OA, PA, SLA, SRA, TA, XA,
};

int yylex();
void yyerror(char const* message);
void undo_bracket(int expected_bracket_depth);
