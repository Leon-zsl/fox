%{
/* lua lex
 */
#include "lua_sym.h"
%}

%option never-interactive

%%

[0-1]+: { printf("number found"); }

[a-zA-Z]+: { printf("character found"); }

%%
