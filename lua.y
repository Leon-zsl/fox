%{
/* lua yacc
 */
#include "lua_sym.h"
%}

%%

sentence:		word { printf("word component"); }
		;

word:			"="
		;

%%

