%option noyywrap

%{
#include <stdlib.h>

#include "generic.h"
%}

TOKEN [^&$\*:;=\t\n\r\ ]
ENDL  [(\r\n)\n]
%x literal
%%

start_literal\([ \t]*\)[ \t]*; {
  BEGIN(literal);
}

<literal>end_literal\([ \t]*\)[ \t]*; {
  BEGIN(INITIAL);
}

<literal>{ENDL} {
  linenr = linenr + 1;
}

<literal>[^\r\n]*	/* eat up literal body */

:            yylval = Vexpp_node(); return COLON;
;            yylval = Vexpp_node(); return SEMICOLON;
=            yylval = Vexpp_node(); return EQ;
\$           yylval = Vexpp_node(); return DOLLAR;
def          yylval = Vexpp_node(); return DEF;
enddef       yylval = Vexpp_node(); return ENDDEF;
scan         yylval = Vexpp_node(); return SCAN;
endscan      yylval = Vexpp_node(); return ENDSCAN;
ref          yylval = Vexpp_node(); return REF;
{ENDL}       yylval = Vexpp_node(); linenr = linenr + 1;


{TOKEN}+ {
  yylval = Vexpp_node( yytext );
  return IDENT;
}

\"(\\.|[^\\"])*\" {
  yylval = Vexpp_node( yytext );
  return IDENT;
}

\*[^\r\n]*{ENDL} {/* disregard comments */
  linenr = linenr + 1;
}

[ \t&]+            /* eat up whitespace  */

