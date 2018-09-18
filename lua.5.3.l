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

"function"				return FUNCTION;
"local"					return LOCAL;

"nil" 					return NIL;
"false"					return FALSE;
"true"					return TRUE;

"if" 					return IF;
"then" 					return THEN;
"else" 					return ELSE;
"elseif" 				return ELSEIF;
"while"					return WHILE;
"do"					return DO;
"repeat" 				return REPEAT;
"until" 			    return UNTIL;
"for"					return FOR;
"break"					return BREAK;
"end"					return END;
"return"				return RETURN;
"goto"					return GOTO;
"in"					return IN;
"::"					return LABEL;

"and" 					return AND;
"or" 					return OR;
"not" 					return NOT;
">=" 					return GE;
"<=" 					return LE;
"==" 					return EQ;
"~=" 					return NE;
"..."					return DOTS;
".."					return CONC;
"<<"					return LSHIFT;
">>"					return RSHIFT;
"//"					return FDIV;

[a-zA-Z_][a-zA-Z0-9_]* 	{ yylval.string = strdup(yytext); return NAME; }

\"[^\"]*\" 				{ yylval.string = strdup(yytext); return STRING; }
\'[^\']*\' 				{ yylval.string = strdup(yytext); return STRING; }

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