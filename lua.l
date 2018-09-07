%{
#include "lua_y.h"
#include "fox.h"

extern char yyfilename[];
extern int yylineno;
extern char *yytext;

#define yyinfo(msg) log_info("%s:%d, %s\n", yyfilename, yylineno, (msg))
#define yyerror(msg) log_error("%s:%d, %s\n", yyfilename, yylineno, (msg))
%}

%option never-interactive

%%

[ \t]+					;
\r?\n					yylineno++;
\r\n?					yylineno++;
"--".*					;

"require"				return REQUIRE;
"function"				return FUNCTION;
"local"					return LOCAL;
"return"				return RETURN;
"end"					return END;
"nil" 					return NIL;

"if" 					return IF;
"then" 					return THEN;
"else" 					return ELSE;
"elseif" 				return ELSEIF;
"while"					return WHILE;
"do"					return DO;
"repeat" 				return REPEAT;
"until" 			    return UNTIL;

"and" 					return AND;
"or" 					return OR;
"not" 					return NOT;
">=" 					return GE;
"<=" 					return LE;
"==" 					return EQ;
"~=" 					return NE;
"..."					return DOTS;
".."					return CONC;

[a-zA-Z_][a-zA-Z0-9_]* 	{ yylval.string = strcopy(yytext); return NAME; }

\"[^\"]*\" 				{ yylval.string = strcopy(yytext); return STRING; }
\'[^\']*\' 				{ yylval.string = strcopy(yytext); return STRING; }

[0-9]+("."[0-9]*)? | 
([0-9]+)?"."[0-9]+ |
[0-9]+("."[0-9]*)?[eE][+-]?[0-9]+ |
([0-9]+)?"."[0-9]+[eE][+-]?[0-9]+ { yylval.number = atof(yytext); return NUMBER; }

.						{ return *yytext; }

%%

char yyfilename[1024] = { '\0' };

void yyset_filename(const char *name) {
	strcpy(yyfilename, name);
}

const char *yyget_filename() {
	return yyfilename;
}

int yywrap(void) {
	yylineno = 1;
	return 1;
}
