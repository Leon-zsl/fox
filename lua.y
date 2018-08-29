%{
/* lua yacc
 */
#include "lua_sym.h"

extern int yylineno;
extern char *yytext;
void yyerror(const char *error);
int yylex(void);
%}

%%

sentence:		word { printf("word component"); }
		;

word:			"="
		;

%%

void yyerror(const char *msg) {
	printf("%d: %s  at  %s \n", yylineno, msg, yytext);
}
