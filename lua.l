%{
#include "lua_y.h"
#include "fox.h"

extern char yyfilename[];
extern int yylineno;
extern char *yytext;

#define yyinfo(msg) log_info("%s:%d, %s\n", yyfilename, yylineno, (msg))
#define yyerror(msg) log_error("%s:%d, %s\n", yyfilename, yylineno, (msg))

extern void multiline_comment(void);
extern char *multiline_string(void);
%}

%option never-interactive

%%

"--[["					multiline_comment();
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

\"[^\"]*\" 				{ yylval.string = fox_strdup(yytext); return STRING; }
\'[^\']*\' 				{ yylval.string = fox_strdup(yytext); return STRING; }
"[["		 			{ yylval.string = fox_strdup(multiline_string()); return STRING; }

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

static char tmp_mlstr[4096] = {'\0'};
char *multiline_string(void) {
	tmp_mlstr[4095] = '\0';
	tmp_mlstr[0] = '\"';
	int idx = 1;
	char c,c1;
loop:
    while ((c = input()) != ']' && c != '\0' && idx < 4094) {
		if(c == '\r') {
			idx++;
			continue;
		}
		if(c == '\n') {
			tmp_mlstr[idx++] = '\\';
			tmp_mlstr[idx++] = 'n';
			continue;
		}
		tmp_mlstr[idx++] = c;
	}
	if(idx >= 4094) {
		yyerror("too long multiline string");
		return "";
	}
	if(c == '\0') return tmp_mlstr;

    if ((c1 = input()) != ']' && c1 != '\0')
    {
        unput(c1);
        goto loop;
    }
	tmp_mlstr[idx++] = '\"';
	tmp_mlstr[idx++] = '\0';
	log_info("multilie str:%s", tmp_mlstr);
	return tmp_mlstr;
}

void multiline_comment(void) {
	char c,c1;
loop:
    while ((c = input()) != ']' && c != '\0');
	if(c == '\0') return;

    if ((c1 = input()) != ']' && c1 != '\0')
    {
        unput(c1);
        goto loop;
    }
}
