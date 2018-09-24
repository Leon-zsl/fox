%{
#include "lua_y.h"
#include "fox.h"

extern char yyfilename[];
extern int yylineno;
extern char *yytext;

#define yyinfo(msg) log_info("%s:%d, %s\n", yyfilename, yylineno, (msg))
#define yyerror(msg) log_error("%s:%d, %s\n", yyfilename, yylineno, (msg))

extern char *multiline_string(const char *str);
%}

%option never-interactive

%%

"--[["([^\]])*"]]"		;
"--".*					;

[ \t]+					;
\r?\n					yylineno++;
\r\n?					yylineno++;

"function"				return FUNCTION;
"local"					return LOCAL;

"nil" 					return NIL;
"true"					return BTRUE;
"false"					return BFALSE;

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

[a-zA-Z_][a-zA-Z0-9_]* 	{ yylval.string = fox_strdup(yytext); return NAME; }

\"[^\"]*\"				|
\'[^\']*\' 				{ yylval.string = fox_strdup(yytext); return STRING; }
"[["([^\]])*"]]"	    { yylval.string = multiline_string(yytext); return STRING; }

[0-9]+("."[0-9]*)?				  | 
([0-9]+)?"."[0-9]+				  |
[0-9]+("."[0-9]*)?[eE][+-]?[0-9]+ |
([0-9]+)?"."[0-9]+[eE][+-]?[0-9]+ |
0x[0-9a-fA-F]+					  { yylval.string = fox_strdup(yytext); return NUMBER; }

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

char *multiline_string(const char *str) {
	static char tmpstr[4096] = { '\0' };
	memset(tmpstr, 0, 4096);

	//1st 2 chars are [ and [, last 2 chars are ] and ]
	int srcidx = 2;
	int dstidx = 0;
	tmpstr[dstidx++] = '\"';
	while(srcidx < strlen(str) - 2 && dstidx < 4094) {
		char c = str[srcidx++];
		if(c == '\r') {
			if(str[srcidx] != '\n') {
				tmpstr[dstidx++] = '\\';
				tmpstr[dstidx++] = 'n';
			} else {
				continue;
			}
		} else if(c == '\n') {
			tmpstr[dstidx++] = '\\';
			tmpstr[dstidx++] = 'n';
		} else {
			tmpstr[dstidx++] = c;
		}
	}
	log_assert(dstidx < 4094, "too long multiline string found %d:%s",
			   yylineno, str);
	tmpstr[dstidx] = '\"';
	
	return fox_strdup(tmpstr);
}
