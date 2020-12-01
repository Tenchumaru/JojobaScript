%{
#include <cstdio>
#include <cstring>
#include "JojobaScript.h"

int yylex();
void yyerror(char const* message);
void undo_bracket();

#pragma warning(push)
#pragma warning(disable: 4127 4244 4702)
%}

%union {
	int value;
	char* id;
}

%token AS BREAK CONTINUE DO ELSE FOR FROM FUNCTION IF IMPORT SEP STRING WHILE
%token <value> NUMBER
%token <id> ID
%nonassoc '?' ':'
%left AND OR
%left EQ NE LE GE '<' '>'
%left '@' ')' /* TODO:  perhaps nonassoc? */
%left '+' '-'
%left '&' '|' '^'
%left ASR LSR SHL
%left '*' '/' '%'
%precedence NEG /* negation:  unary minus, bit-wise complement, logical complement */
%left '.' '['
%type <value> dexpr expr expr_list fexpr id_list kv_list oid_list pexpr

%%

block:
statements { emit(); }
| statements separators { emit(); }
;

statements:
statement { emit(); }
| statements separators statement { emit(); }
;

statement:
ID ':' { add_symbol($1); }
| FUNCTION ID '(' oid_list ')' { undo_bracket(); } '{' block '}' { add_symbol($2); }
| flow_statement { emit(); }
| import_statement { emit(); }
| simple_statement { emit(); }
;

separators:
SEP
| separators SEP
;

flow_statement:
BREAK { emit(); }
| CONTINUE { emit(); }
| DO { undo_bracket(); } '{' block '}' WHILE expr { emit(); }
| FOR simple_statement SEP expr SEP simple_statement { undo_bracket(); } '{' block '}' { emit(); }
| if_statement { emit(); }
| if_statement ELSE { undo_bracket(); } '{' block '}' { emit(); }
| WHILE expr { undo_bracket(); } '{' block '}' { emit(); }
;

if_statement:
IF expr { undo_bracket(); } '{' block '}' { emit(); }
;

import_statement:
FROM STRING IMPORT imports { emit(); }
IMPORT STRING { emit(); }
IMPORT STRING AS ID { emit(); }
;

imports:
import { emit(); }
| imports ',' import { emit(); }
;

import:
ID { emit(); }
| ID AS ID { emit(); }
;

simple_statement:
dexpr '=' expr { emit($3); }
| fexpr { emit($1); }
;

expr:
NUMBER                   { $$ = $1;           }
| expr '?' expr ':' expr { $$ = $1 ? $3 : $5; }
| expr AND expr          { $$ = $1 && $3;     }
| expr OR expr           { $$ = $1 || $3;     }
| expr EQ expr           { $$ = $1 == $3;     }
| expr NE expr           { $$ = $1 != $3;     }
| expr LE expr           { $$ = $1 <= $3;     }
| expr GE expr           { $$ = $1 >= $3;     }
| expr '<' expr          { $$ = $1 < $3;      }
| expr '>' expr          { $$ = $1 > $3;      }
| expr '+' expr          { $$ = $1 + $3;      }
| expr '-' expr          { $$ = $1 - $3;      }
| expr '&' expr          { $$ = $1 & $3;      }
| expr '|' expr          { $$ = $1 | $3;      }
| expr '^' expr          { $$ = $1 ^ $3;      }
| expr SHL expr          { $$ = $1 << $3;     }
| expr LSR expr          { $$ = $1 >> $3;     }
| expr ASR expr          { $$ = $1 >> $3;     } /* TODO */
| expr '*' expr          { $$ = $1 * $3;      }
| expr '/' expr          { $$ = $1 / $3;      }
| expr '%' expr          { $$ = $1 % $3;      }
| expr '[' expr ']'      { emit();            }
| '-' expr %prec NEG     { $$ = -$2;          }
| '~' expr %prec NEG     { $$ = ~$2;          }
| '!' expr %prec NEG     { $$ = !$2;          }
| '[' ']'                { emit();            }
| '{' '}'                { emit();            } /* TODO:  set or object? */
| '[' expr_list ']'      { $$ = $2;           }
| '{' expr_list '}'      { $$ = $2;           }
| '{' kv_list '}'        { $$ = $2;           }
| dexpr                  { $$ = $1;           }
| fexpr                  { $$ = $1;           }
| pexpr                  { $$ = $1;           }
| expr '@' '(' id_list ')' expr { $$ = $1; }
;

dexpr:
expr '.' ID { emit();             }
| ID        { $$ = get_value($1); }
;

fexpr:
ID '(' expr ')' { $$ = $3; }
| fexpr '(' expr ')' { $$ = $1; }
| pexpr '(' expr ')' { $$ = $3; }
;

pexpr:
'(' expr ')' { $$ = $2; }
;

expr_list:
expr
| expr_list ',' expr { $$ = $3; }
;

kv_list:
ID ':' expr { $$ = $3; }
| kv_list ',' ID ':' expr { $$ = $5; }
;

id_list:
ID { $$ = get_value($1); }
| id_list ',' ID { $$ = $1; }
;

oid_list:
%empty { emit(); }
| id_list { $$ = $1; }
;

%%

#pragma warning(pop)

extern int yylineno;

void yyerror(char const* message) {
	fprintf(stderr, "problem in line %d: %s\n", yylineno, message);
}
