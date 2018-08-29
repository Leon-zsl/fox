%{
/* lua lex
 */
#include "lua_sym.h"

int yywrap(void);
%}

%option never-interactive

%%

[0-1]+: { printf("number found"); }

[a-zA-Z]+: { printf("character found"); }

%%

int yywrap(void) {
	return 0;
}
