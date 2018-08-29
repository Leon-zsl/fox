#ifndef __LUA_SYM_H__
#define __LUA_SYM_H__

#include "fox.h"

extern int yylex(void);
extern int yyerror(const char *error);
extern int yywrap(void);

#endif
