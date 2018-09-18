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

int token2binexp(int token) {
	switch(token) {
	case '+':
		return EXP_ADD;
	case '-':
		return EXP_SUB;
	case '*':
		return EXP_MUL;
	case '/':
		return EXP_DIV;
	case FDIV:
		return EXP_FDIV;
	case '^':
		return EXP_EXP;
	case '%':
		return EXP_MOD;
	case '&':
		return EXP_BAND;
	case '|':
		return EXP_BOR;
	case '~':
		return EXP_XOR;
	case LSHIFT:
		return EXP_LSHIFT;
	case RSHIFT:
		return EXP_RSHIFT;
	case CONC:
		return EXP_CONC;
	case '<':
		return EXP_LESS;
	case '>':
		return EXP_GREATER;
	case LE:
		return EXP_LE;
	case GE:
		return EXP_GE;
	case EQ:
		return EXP_EQ;
	case NE:
		return EXP_NE;
	case AND:
		return EXP_AND;
	case OR:
		return EXP_OR;
	default:
		log_error("%s:%d, unknown token to binop: %d\n", yyfilename, yylineno, token)
		return 0;
	}
}

int token2unexp(int token) {
	switch(token) {
	case NOT:
		return EXP_NOT;
	case '#':
		return EXP_LEN;
	case '~':
		return EXP_BNOT;
	case '-':
		return EXP_NEG;
	default:
		log_error("%s:%d, unknown token to unop: %d\n", yyfilename, yylineno, token)		
		return 0;
	}
}

struct syntax_tree *parse_tree = NULL;
struct symbol_table *parse_table = NULL;

%}

%union {
	int integer;
	double number;
	char* string;
	struct syntax_chunk *chunk;
	struct syntax_block *block;	
	struct syntax_statement *stmt;
	struct syntax_function *func;
	struct syntax_functioncall *fcall;
	struct syntax_expression *exp;
	struct syntax_variable *var;
	struct syntax_parameter *par;
	struct syntax_argument *arg;
	struct syntax_table *table;
	struct syntax_field *field;
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
				
%type<stmt>				stat statlist retstat elsepart label
%type<exp>				exp explist prefixexp

%type<string>			funcname stdfuncname
%type<string>			namelist
						
%type<var> 				var varlist
%type<func>				funcbody
%type<fcall>			funcall
%type<arg>				args
%type<par>				pars
%type<table>			table
%type<field>			field fieldlist
%type<integer>			binop unop

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

retstat:		RETURN retend
				{
					struct syntax_statement *ret = create_syntax_statement();
					ret->tag = STMT_RETURN;
					$$ = ret;
				}
		|		RETURN explist retend
				{
					struct syntax_statement *ret = create_syntax_statement();
					ret->tag = STMT_RETURN;
					syntax_node_push_child_tail(&ret->n, &($2->n));
					$$ = ret;
				}
		;

retend:		/* empty*/
		|		';'
		;

elsepart:		/* empty */
				{
				}
		|		ELSE block
				{
				}
		|		ELSEIF exp THEN block elsepart
				{
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
		|		FUNCTION funcbody
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

funcbody:		'(' ')' block END
				{
				}
		|		'(' pars ')' block END
				{
				}
		;

pars:			namelist
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
					$$ = create_syntax_table();
					syntax_node_push_child_tail(&($$->n), &($2->n));
				}
				'{' '}'
				{
					$$ = create_syntax_table();
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
