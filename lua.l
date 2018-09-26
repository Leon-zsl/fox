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

"--["=*"["			{ multiline_comment(); }
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
"["(=)*"["			    { yylval.string = multiline_string(); return STRING; }

[0-9]+("."[0-9]*)?				  | 
([0-9]+)?"."[0-9]+				  |
[0-9]+("."[0-9]*)?[eE][+-]?[0-9]+ |
([0-9]+)?"."[0-9]+[eE][+-]?[0-9]+ |
0[xX][0-9a-fA-F]+				  { yylval.string = fox_strdup(yytext); return NUMBER; }

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

char *multiline_string(void) {
	int eqcnt = 0;
	char *p = yytext;
	while(*p != '\0') {
		if(*p == '=') {
			eqcnt++;
		}
		p++;
	}
	
	static char tmpstr[4096] = { '\0' };
	memset(tmpstr, 0, 4096);
	
	int dstidx = 0;
	tmpstr[dstidx++] = '\"';
	
	int idx = 0;
	int cnt = eqcnt;
	static char tmpchar[64] = { '\0' };
	char c, c1;
loop:	
    while ((c = input()) != ']' && c != '\0') {
		if(c == '\n') {
			tmpstr[dstidx++] = '\\';
			tmpstr[dstidx++] = 'n';
			yylineno++;
		} else if(c == '\r') {
			continue;
		} else {
			tmpstr[dstidx++] = c;
		}
		log_assert(dstidx < 4094, "too long multiline string found %d:%s",
				   yylineno, tmpstr);
	}
    if(c == '\0') {
		tmpstr[dstidx++] = '\"';
		return fox_strdup(tmpstr);
	}

	idx = 0;
	cnt = eqcnt;	
	memset(tmpchar, 0, 64);
	while(cnt > 0) {
		c1 = input();
		if(c1 == '=') {
			tmpchar[idx++] = c1;
			cnt--;
			continue;
		}
		if(c1 == '\0') {
			tmpstr[dstidx++] = '\"';
			return fox_strdup(tmpstr);		
		}

		unput(c1);
		for(int i = idx - 1; i >= 0; i--) {
			unput(tmpchar[i]);
		}
		goto loop;
	}
	
    if ((c1 = input()) != ']' && c1 != '\0')
    {
        unput(c1);
		for(int i = idx - 1; i >= 0; i--) {
			unput(tmpchar[i]);
		}
        goto loop;
    }
	
	tmpstr[dstidx] = '\"';
	return fox_strdup(tmpstr);
}

void multiline_comment(void) {
	int eqcnt = 0;
	char *p = yytext;
	while(*p != '\0') {
		if(*p == '=') {
			eqcnt++;
		}
		p++;
	}

    char c, c1;
	int cnt;
loop:
    while ((c = input()) != ']' && c != '\0') {
		if(c == '\n') yylineno++;
	}
    if(c == '\0') return;

	cnt = eqcnt;
	while(cnt > 0) {
		c1 = input();
		if(c1 == '=') {
			cnt--;
			continue;
		}
		if(c1 == '\0') {
			return;
		}

		unput(c1);
		goto loop;
	}

    if ((c1 = input()) != ']' && c1 != '\0')
    {
        unput(c1);
        goto loop;
    }

	//comment finish
}
