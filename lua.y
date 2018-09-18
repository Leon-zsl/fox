%{
#include "fox.h"
#include "symbol.h"
#include "syntax.h"

int yylex(void);

extern char yyfilename[];
extern int yylineno;
extern char *yytext;

#define YYDEBUG 1
#if YYDEBUG
int yydebug = 1;
typedef union YYSTYPE YYSTYPE;
void yyprint(FILE *file, int type, YYSTYPE value);
#define YYPRINT(file, type, value)   yyprint(file, type, value)
#endif

#define yyinfo(msg) log_info("%s:%d, %s\n", yyfilename, yylineno, (msg))
#define yyerror(msg) log_error("%s:%d, %s\n", yyfilename, yylineno, (msg))

struct syntax_tree *parse_tree = NULL;
struct symbol_table *parse_table = NULL;

%}

%union {
	double number;
	char* string;
	struct syntax_chunk *chunk;
	struct syntax_block *block;	
	struct syntax_statement *stmt;
	struct syntax_function *func;
	struct syntax_functioncall *fcall;
	struct syntax_expression *expr;
	struct syntax_variable *var;
	struct syntax_parameter *par;
	struct syntax_table *table;
	struct syntax_field *field;
	struct syntax_operator *op;
}

%start					program

%token					FUNCTION LOCAL
%token					NIL TRUE FALSE						

%token					IF THEN ELSE ELSEIF WHILE DO REPEAT UNTIL FOR BREAK END RETURN GOTO IN LABEL
%token					AND OR NOT GE LE EQ NE CONC DOTS SHIFT_LEFT LSHIFT RSHIFT FDIV

%token<string>			NAME STRING
%token<number>			NUMBER

%type<chunk>			chunk
%type<block>			block
				
%type<stmt>				stat statlist elsepart retstat label 
%type<expr>				exp explist prefixexp
						
%type<var> 				var varlist
%type<func>				function
%type<fcall>			function_call						
%type<arg>				argument argument_list

%type<table>			table
%type<field>			field fieldlist
%type<ret>				ret

%left					OR
%left					AND						
%left 					EQ NE '<' '>' GE LE
%left					'|'
%left					'~'
%left					'&'
%left					LSHIFT RSHIFT
%left					CONC																														
%left					 '+' '-'
%left					 '*' '/' '%' FDIV
%right 					NOT '#' '-' '~'
%left					'^'

%%

program:		chunk
				{
					parse_tree->root = &($1->n);
				}
		;

chunk:			block
				{
					struct syntax_chunk *chunk = create_syntax_chunk();
					syntax_node_push_child_tail(&chunk->n, &($1->n));
					$$ = chunk;
				}
		;

block:			/* empty */
				{
					$$ = create_syntax_block();
				}
		|		retstat
				{
					struct syntax_block *block = create_syntax_block();
					syntax_node_push_child_tail(&block->n, &($1->n));
					$$ = block;
				}
		|		statlist
				{
					struct syntax_block *block = create_syntax_block();
					syntax_node_push_child_tail(&block->n, &($1->n));
					$$ = block;
				}
		|		statlist retstat
				{
					struct syntax_block *block = create_syntax_block();
					syntax_node_push_child_tail(&block->n, &($1->n));
					syntax_node_push_child_tail(&block->n, &($2->n));
					$$ = block;
				}
		;

statlist:		stat
				{
					$$ = $1;
				}
		|		statlist stat
				{
					syntax_node_push_child_tail(&($1->n), &($2->));
					$$ = $1;
				}
		;

stat:			';'
				{
					struct syntax_statement *stl = create_syntax_statement();
					stl->tag = STMT_RETURN;
					$$ = stl;
				}
		|		varlist '=' explist
				{
				}
		|		funcall
				{
				}
		|		label
				{
				}
		|		BREAK
				{
				}
		|		GOTO NAME
				{
				}
		|		DO block END
				{
				}
		|		WHILE exp DO block END
				{
				}
		|		REPEAT block UNTIL exp
				{
				}
		|		IF exp THEN block elsepart END
				{
				}
		|		FOR namelist IN explist DO block END
				{
				}
		|		FOR NAME '=' exp ',' exp ',' exp DO block END
				{
				}
		|		FOR NAME '=' exp ',' exp DO block END
				{
				}
		|		FUNCTION funcname funcbody
				{
				}
		|		LOCAL FUNCTION funcname funcbody
				{
				}
		|		LOCAL namelist
				{
				}
		|		LOCAL namelist '=' explist
				{
				}
		;

retstat:		RETURN
				{
					struct syntax_statement *ret = create_syntax_statement();
					ret->tag = STMT_RETURN;
					$$ = ret;
				}
		|		RETURN ';'
				{
					struct syntax_statement *ret = create_syntax_statement();
					ret->tag = STMT_RETURN;
					$$ = ret;
				}
		|		RETURN explist
				{
					struct syntax_statement *ret = create_syntax_statement();
					ret->tag = STMT_RETURN;
					syntax_node_push_child_tail(&ret->n, &($2->n));
					$$ = ret;
				}
		|		RETURN explist ';'
				{
					struct syntax_statement *ret = create_syntax_statement();
					ret->tag = STMT_RETURN;
					syntax_node_push_child_tail(&ret->n, &($2->n));
					$$ = ret;					
				}
		;

label:			LABEL NAME LABEL
				{
				}
		;

stdfuncname:	NAME
				{
				}
		|		stdfuncname '.' NAME
				{
				}
		;

funcname:		stdfuncname
				{
				}
		|		stdfuncname ':' NAME
				{
				}
		;

varlist:		var
				{
				}
		|		varlist ',' var
				{
				}
		;

var:			NAME
				{
				}
		|		prefixexp '[' exp ']'
				{
				}
		|		prefixexp '.' NAME
				{
				}
		;

namelist:		NAME
				{
				}
		|		namelist ',' NAME
				{
				}
		;

explist:		exp
				{
				}
		|		explist ',' exp
				{
				}
		;

exp:			NIL
				{
				}
		|		TRUE
				{
				}
		|		FALSE
				{
				}
		|		NUMBER
				{
				}
		|		STRING
				{
				}
		|		DOTS
				{
				}
		|		funcdef
				{
				}
		|		prefixexp
				{
				}
		|		table
				{
				}
		|		exp binop exp
				{
				}
		|		unop exp
				{
				}
		;

prefixexp:		var
				{
				}
		|		funcall
				{
				}
		|		'(' exp ')'
				{
				}
		;

funcall:		prefixexp args
				{
				}
		|		prefixexp ':' Name args
				{
				}

args:			'(' ')'
				{
				}
		|		'(' explist ')'
				{
				}
		|		table
				{
				}
		|		STRING
				{
				}
		;

funcdef:		FUNCTION funcbody
				{
				}
		;

funcbody:		'(' ')' block END
				{
				}
		|		'(' parlist ')' block END
				{
				}
		;

parlist:		namelist
				{
				}
		|		namelist ',' DOTS
				{
				}
		|		DOTS
				{
				}
		;

table:			'{' fieldlist '}'
				{
				}
		;

fieldlist:		field
				{
				}
		|		fieldlist fieldsep field
				{
				}
		|		fieldlist fieldsep
				{
				}
		;

field:			'[' exp ']' '=' exp
				{
				}
		|		NAME '=' exp
				{
				}
		|		exp
				{
				}
		;

fieldsep:		','
		|		';'
		;

binop:			'+'
				{
					struct syntax_operator *op = 
				}
		|		'-'
		|		'*'
		|		'/'
		|		FDIV
		|		'^'
		|		'%'
		|		'&'
		|		'~'
		|		'|'
		|		LSHIFT
		|		RSHIFT
		|		CONC
		|		'<'
		|		'>'
		|		LE
		|		GE
		|		EQ
		|		NE
		|		AND
		|		OR
		;

unop:			NOT
		|		'#'
		|		'~'
		|		'-'
		;
%%

/* 
void yyerror(const char *msg) {
	log_error("%s:%d, %s at %s \n", yyfilename, yylineno, msg, yytext);
}
*/

#if YYDEBUG
void yyprint(FILE *file, int type, YYSTYPE value)
{
	switch(type) {
	case NUMBER:
		fprintf(file, "\n[YACC]%s, %f\n", "number", value.number);
		break;
	case STRING:
		fprintf(file, "\n[YACC]%s, %s\n", "string", value.string);
		break;
	case NAME:
		fprintf(file, "\n[YACC]%s, %s\n", "name", value.string);
		break;
	default:
		fprintf(file, "\n[YACC]%s, %d\n", "token", type);
		break;
	}
}
#endif
