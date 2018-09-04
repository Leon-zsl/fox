%{
#include "lua_y.h"
#include "fox.h"

extern char yyfilename[];
extern int yylineno;
extern char *yytext;

#define YYLEX_DEBUG 0
#if YYLEX_DEBUG
#define yylex_debug(msg) log_info("%s:%d, %s at %s \n", yyfilename, yylineno, (msg), yytext)
#else
#define yylex_debug(msg)
#endif
%}

%option never-interactive

%%

[ \t]+					{ yylex_debug("blank"); }
\r?\n					{ yylex_debug("newline"); yylineno++; }
\r\n?					{ yylex_debug("newline"); yylineno++; }
"--".*					{ yylex_debug("comment"); }

"require"				{ yylex_debug("require"); return REQUIRE; }
"function"				{ yylex_debug("function"); return FUNCTION; }
"local"					{ yylex_debug("local"); return LOCAL; }
"return"				{ yylex_debug("return"); return RETURN; }
"end"					{ yylex_debug("end"); return END; }
"nil" 					{ yylex_debug("nil"); return NIL; }

"if" 					{ yylex_debug("if"); return IF; }
"then" 					{ yylex_debug("then"); return THEN; }
"else" 					{ yylex_debug("else"); return ELSE; }
"elseif" 				{ yylex_debug("elseif"); return ELSEIF; }
"while"					{ yylex_debug("while"); return WHILE; }
"repeat" 				{ yylex_debug("repeat"); return REPEAT; }
"until" 			    { yylex_debug("until"); return UNTIL; }

"and" 					{ yylex_debug("and"); return AND; }
"or" 					{ yylex_debug("or"); return OR; }
"not" 					{ yylex_debug("not"); return NOT; }
">" 					{ yylex_debug(">"); return GREATER; }
">=" 					{ yylex_debug(">="); return GE; }
"<" 					{ yylex_debug("<"); return LESS; }
"<=" 					{ yylex_debug("<="); return LE; }
"==" 					{ yylex_debug("=="); return EQ; }
"~=" 					{ yylex_debug("~="); return NE; }

"+"						{ yylex_debug("+"); return ADD; }
"-" 					{ yylex_debug("-"); return SUB; }
"*"						{ yylex_debug("*"); return MULTY; }
"/"						{ yylex_debug("/"); return DIV; }
"%"						{ yylex_debug("%"); return MOD; }
".."					{ yylex_debug(".."); return CONCAT; }

"=" 					{ yylex_debug("="); return ASSIGN; }
"."						{ yylex_debug("."); return FIELD; }
"("						{ yylex_debug("("); return LPARENTHESE; }
")"						{ yylex_debug(")"); return RPARENTHESE; }
"[" 					{ yylex_debug("["); return LBRACKET; }
"]"						{ yylex_debug("]"); return RBRACKET; }
","						{ yylex_debug(","); return COMMA; }
":"						{ yylex_debug(":"); return COLON; }

[a-zA-Z_][a-zA-Z0-9_]* 	{ yylex_debug("name"); yylval.string = yytext; return NAME; }

\"[^\"]*\" 				{ yylex_debug("string"); yylval.string = yytext; return STRING; }
\'[^\']*\' 				{ yylex_debug("string"); yylval.string = yytext; return STRING; }

-?[0-9]+("."[0-9]*)? | 
-?([0-9]+)?"."[0-9]+ |
-?[0-9]+("."[0-9]*)?[eE][+-]?[0-9]+ |
-?([0-9]+)?"."[0-9]+[eE][+-]?[0-9]+ { yylex_debug("number"); yylval.number = atof(yytext); return NUMBER; }

.						{ yylex_debug("unknown"); return *yytext; }

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
