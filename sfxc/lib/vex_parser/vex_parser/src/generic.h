#ifndef GENERIC_H
#define GENERIC_H

#include <string>

#define MSG(a) { std::cout << a << std::endl; }

#include <stdio.h>
extern FILE *yyin, *yyout;

#include "vex/Vex++.h"

#define YYSTYPE Vex::Node
#include "vex_parser.h"

extern YYSTYPE parse_result;

// Compute the line number
extern int linenr;

int yylex();
int yyparse();
void yyerror (char const *msg);

#endif // GENERIC_H
