/* Reverse polish notation calculator. */
/* %file-prefix="parser" */

/* Location tracking.  */
/* %locations */

/* %pure-parser */
/* %parse-param {YYSTYPE &result} */

%defines

%{
#define YYERROR_VERBOSE 1

#include <math.h>
#include <stdio.h>
#include <iostream>

#include "generic.h"
%}


%token IDENT COLON SEMICOLON EQ DOLLAR DEF ENDDEF SCAN ENDSCAN REF

%{
#include <math.h>
#include <stdio.h>
#include <iostream>
%}

%% /* Grammar rules and actions follow. */

vex: 
  { linenr = 1; }
  vex_header
  blocks
  {
    $$= $3;
    $$.join($2);
    //std::cout << "FINISHED: " << $3 << std::endl; 
    parse_result = $$;
  }
;

vex_header: 
  IDENT EQ IDENT SEMICOLON 
  { 
    $$ = YYSTYPE(YYSTYPE::DICT);
    $$.set_key($1.to_string(), $3);
  }
;

blocks:
  block
  { $$ = $1; }
| blocks
  block
  {
    $$ = $1; 
    $$.join($2);
  }
;

block: 
  block_header
  block_content 
  { 
    $$ = YYSTYPE(YYSTYPE::DICT);
    $$.set_key($1.to_string(), $2); 
  }
;

block_header:
  DOLLAR IDENT SEMICOLON
  { $$ = $2; }
;

block_content:
  block_lines
  { $$ = $1; }
| block_content def_block
  {
    $$ = $1;
    $$.join($2);
  }
| block_content scan_block
  {
    $$ = $1;
    $$.join($2);
  }
;

def_block: 
  DEF  word SEMICOLON block_lines ENDDEF  SEMICOLON
  {
    $$ = YYSTYPE(YYSTYPE::DICT);
    $$.set_key($2.to_string(), $4);
  }
;

scan_block: 
  SCAN word SEMICOLON block_lines ENDSCAN SEMICOLON
  {
    $$ = YYSTYPE(YYSTYPE::DICT);
    $$.set_key($2.to_string(), $4);
  }
;

block_lines:
  { $$ = YYSTYPE(YYSTYPE::DICT); }
| block_lines 
  block_line
  { $$ = $1; $$.join($2); } 
;
           
block_line:
  REF DOLLAR word EQ value SEMICOLON 
  { 
    $$ = YYSTYPE(YYSTYPE::DICT);
    $$.set_key($3.to_string(), $5); 
  }
| word EQ value SEMICOLON
  {
    $$ = YYSTYPE(YYSTYPE::DICT);
    $$.set_key($1.to_string(), $3); 
  }
;

value:
  word { $$ = $1; }
| { $$ = YYSTYPE(""); }
| value COLON word 
  {
    if ($1.get_type() == YYSTYPE::STRING) {
      $$ = YYSTYPE(YYSTYPE::ARRAY);
      $$.append($1);
    } else {
      $$ = $1;
    } 
    $$.append($3);
  }
| value COLON
  {
    if ($1.get_type() == YYSTYPE::STRING) {
      $$ = YYSTYPE(YYSTYPE::ARRAY);
      $$.append($1);
    } else {
      $$ = $1;
    }
    $$.append(YYSTYPE(""));
  }
;

word:
  IDENT 
  { $$ = $1; }
| word 
  IDENT 
  { 
    std::string key = $1.to_string() + " " + $2.to_string(); 
    $$ = YYSTYPE(key.c_str()); 
  }
;

%% 

void yyerror (char const *s) {
  fprintf (stderr, " == Error: %s\n", s);
  fprintf (stderr, " == In line: %d\n\n", linenr);
}

// Global variables
YYSTYPE parse_result;
int     linenr;
