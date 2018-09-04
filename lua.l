%{
#include "lua_y.h"
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
"repeat" 				return REPEAT;
"until" 			    return UNTIL;

"and" 					return AND;
"or" 					return OR;
"not" 					return NOT;
">" 					return GREATER;
">=" 					return GE;
"<" 					return LESS;
"<=" 					return LE;
"==" 					return EQUAL;
"~=" 					return NE;

"+"						return ADD;
"-" 					return SUB;
"*"						return MULTY;
"/"						return DIV;
"%"						return MOD;
".."					return CONCAT;

"=" 					return ASSIGN;
"."						return FIELD;
"("						return LPARENTHESE;
")"						return RPARENTHESE;
"[" 					return LBRACKET;
"]"						return RBRACKET;

[a-zA-Z_][a-zA-Z0-9_]* 	{ return NAME; }

\"[^\"]*\" 				{ return STRING; }
\'[^\']*\' 				{ return STRING; }

-?[0-9]+("."[0-9]*)? | 
-?([0-9]+)?"."[0-9]+ |
-?[0-9]+("."[0-9]*)?[eE][+-]?[0-9]+ |
-?([0-9]+)?"."[0-9]+[eE][+-]?[0-9]+ { return NUMBER; }

. return *yytext;

%%

int yywrap(void) {
	yylineno = 1;
	return 1;
}
