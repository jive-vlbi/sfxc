#ifndef GENERIC_H
#define GENERIC_H

#include <string>

#define MSG(a) { std::cout << a << std::endl; }

#include <stdio.h>
extern FILE *yyin, *yyout;

#include "vex/Vex++.h"

#define YYSTYPE Vex::Node
#include "vex_parser.h"

// NGHK: TODO: Check the default way to return the AST
extern YYSTYPE parse_result;

int yylex();
int yyparse();
void yyerror (char const *msg);

#endif // GENERIC_H
