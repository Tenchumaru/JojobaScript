#pragma once

int yylex();
void yyerror(char const* message);
void undo_bracket(int expected_bracket_depth);
