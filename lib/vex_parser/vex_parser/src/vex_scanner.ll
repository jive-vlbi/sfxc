%option noyywrap

%{
#include <stdlib.h>

#include "generic.h"
%}

TOKEN [^&$\*:;=\t\n\r\ ]
%%


:            return COLON;
;            return SEMICOLON;
=            return EQ;
\$           return DOLLAR;
def          return DEF;
enddef       return ENDDEF;
scan         return SCAN;
endscan      return ENDSCAN;
ref          return REF;


{TOKEN}+ {
  yylval = Vexpp_node( yytext );
  return IDENT;
}

\*[^\n\r]*(\n|\r(\n)?) /* disregard comments */

[ \t&\n\r]+            /* eat up whitespace  */

