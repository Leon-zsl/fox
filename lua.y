%{
#include "fox.h"
#include "symbol.h"
#include "parser.h"

int yylex(void);

extern char yyfilename[];
extern int yylineno;
extern char *yytext;

#define YYPARSE_DEBUG 1
#if YYPARSE_DEBUG
#define yyparse_debug(msg) log_info("%s:%d, %s at %s \n", yyfilename, yylineno, (msg), yytext)
#else
#define yyparse_debug(msg)
#endif

#define yyerror(msg) log_error("%s:%d, %s at %s \n", yyfilename, yylineno, (msg), yytext)
#define yywarn(msg) log_warn("%s:%d, %s at %s \n", yyfilename, yylineno, (msg), yytext)
#define yyinfo(msg) log_info("%s:%d, %s at %s \n", yyfilename, yylineno, (msg), yytext)

#define YYDEBUG 1

#if YYDEBUG
int yydebug = 1;
#define YYPRINT(file, type, value)   yyprint(file, type, value)
static void yyprint(FILE *file, int type, YYSTYPE value)
{
	switch(type) {
	case NUMBER:
		fprintf(file, "[YACC]%s,%s\n", "number", value.number);
		break;
	case STRING:
		fprintf(file, "[YACC]%s,%s\n", "string", value.string);
		break;
	case NAME:
		fprintf(file, "[YACC]%s,%s", "name", value.string);
		break;
	default:
		fprintf(file, "[YACC]%d,%p", type, value.chunk);
		break;
	}
}
#endif

%}

%union {
	double number;
	char* string;
	struct syntax_chunk *chunk;
	struct syntax_statement *stmt;
	struct syntax_function *func;
	struct syntax_functioncall *fcall;
	struct syntax_requirement *req;
	struct syntax_block *block;
	struct syntax_expression *expr;
	struct syntax_variable *var;
	struct syntax_argument *arg;
	struct syntax_table *table;
	struct syntax_field *field;
	struct syntax_return *ret;
}

%start					program

%token					REQUIRE FUNCTION LOCAL RETURN END NIL

%token					IF THEN ELSE ELSEIF WHILE DO REPEAT UNTIL
%token					AND OR NOT GE LE EQ NE CONC DOTS

%token<string>			NAME STRING
%token<number>			NUMBER

%type<chunk>			chunk chunk_list
%type<req>				requirement						
%type<func>				function
%type<fcall>			function_call
%type<block>			block
%type<stmt>				statement statement_list elsepart
%type<expr>				expression expression_list
%type<var> 				variable variable_list
%type<arg>				argument argument_list
%type<table>			table
%type<field>			field field_list
%type<ret>				ret

%left					AND OR
%left 					EQ NE '<' '>' GE LE
%left 					CONC
%left					 '+' '-'
%left					 '*' '/' '%'
%right 					NOT

%%

program:		/*empty*/
				{
					yyinfo("empty program!");
					struct syntax_tree *t = syntax_tree_create();
					struct syntax_program *p = create_syntax_program();
					t->root = (struct syntax_node *)p;
					parse_tree = t;					
				}
		|		chunk_list
				{
					yyinfo("reduced to program!");
					struct syntax_tree *t = syntax_tree_create();
					struct syntax_program *p = create_syntax_program();
					struct syntax_node *c = (struct syntax_node *)($1);
					syntax_node_push_child_head(&p->n, c);
					t->root = (struct syntax_node *)p;
					parse_tree = t;
				}
		;

chunk_list:		chunk
				{
					$$ = $1;
				}
		|		chunk_list chunk
				{
					syntax_node_push_sibling_tail(&($1->n), &($2->n));
					$$ = $1;
				}
		;

chunk:			requirement
				{
					struct syntax_chunk *chunk = create_syntax_chunk();
					struct syntax_node *n = (struct syntax_node *)($1);
					syntax_node_push_child_head(&chunk->n, n);
					$$ = chunk;					
				}
		|		function
				{
					struct syntax_chunk *chunk = create_syntax_chunk();
					struct syntax_node *n = (struct syntax_node *)($1);
					syntax_node_push_child_head(&chunk->n, n);
					$$ = chunk;
				}
		|		statement_list
				{
					struct syntax_chunk *chunk = create_syntax_chunk();
					struct syntax_node *n = (struct syntax_node *)($1);
					syntax_node_push_child_head(&chunk->n, n);
					$$ = chunk;
				}
		;

requirement:	REQUIRE '(' NAME ')'
				{
					$$ = create_syntax_requirement($3);
				}
		;

function:		FUNCTION NAME '(' argument_list ')' block END
				{
					struct syntax_function *func = create_syntax_function($2);
					syntax_node_push_child_tail(&func->n, &($4->n));
					syntax_node_push_child_tail(&func->n, &($6->n));
					$$ = func;
				}
		|		FUNCTION NAME '(' ')' block END
				{
					struct syntax_function *func = create_syntax_function($2);
					syntax_node_push_child_tail(&func->n, &($5->n));
					$$ = func;
				}
		;

block:			/* empty */
				{
					$$ = create_syntax_block();
				}
		|		statement_list
				{
					$$ = create_syntax_block();
					syntax_node_push_child_tail(&($$->n), &($1->n));
				}
		|		statement_list ret
				{
					$$ = create_syntax_block();
					syntax_node_push_child_tail(&($$->n), &($1->n));
					syntax_node_push_child_tail(&($$->n), &($2->n));
				}
		;

ret:			RETURN expression
				{
					$$ = create_syntax_return();
					syntax_node_push_child_tail(&($$->n), &($2->n));
				}
		;

statement_list:	statement
				{
					$$ = $1;
				}
		|		statement_list statement
				{
					syntax_node_push_sibling_tail(&($1->n), &($2->n));
					$$ = $1;
				}
		;

statement:		IF expression THEN block elsepart END
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->tag = STMT_IF;
					syntax_node_push_child_tail(&stmt->n, &($2->n));
					syntax_node_push_child_tail(&stmt->n, &($4->n));
					syntax_node_push_child_tail(&stmt->n, &($5->n));
					$$ = stmt;
				}
		|		REPEAT block UNTIL expression
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->tag = STMT_REPEAT;					
					syntax_node_push_child_tail(&stmt->n, &($2->n));
					syntax_node_push_child_tail(&stmt->n, &($4->n));
					$$ = stmt;
				}
		|		WHILE expression DO block END
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->tag = STMT_WHILE;					
					syntax_node_push_child_tail(&stmt->n, &($2->n));
					syntax_node_push_child_tail(&stmt->n, &($4->n));
					$$ = stmt;
				}
		|		function_call
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->tag = STMT_FCALL;					
					syntax_node_push_child_tail(&stmt->n, &($1->n));
					$$ = stmt;					
				}				
		|		variable_list '=' expression_list
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->tag = STMT_VARS;					
					syntax_node_push_child_tail(&stmt->n, &($1->n));
					syntax_node_push_child_tail(&stmt->n, &($3->n));
					$$ = stmt;
				}
		|		LOCAL variable_list '=' expression_list
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->tag = STMT_LOCAL_VARS;
					syntax_node_push_child_tail(&stmt->n, &($2->n));
					syntax_node_push_child_tail(&stmt->n, &($4->n));
					$$ = stmt;
				}
		;

elsepart:		/* empty */
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->tag = STMT_ELSE_EMPTY;
					$$ = stmt;
				}
		|		ELSE block
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->tag = STMT_ELSE;
					syntax_node_push_child_tail(&stmt->n, &($2->n));
					$$ = stmt;
				}
		|		ELSEIF expression THEN block elsepart
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->tag = STMT_ELSEIF;
					syntax_node_push_child_tail(&stmt->n, &($2->n));
					syntax_node_push_child_tail(&stmt->n, &($4->n));
					syntax_node_push_child_tail(&stmt->n, &($5->n));
					$$ = stmt;
				}
		;

expression_list: expression
				{
					$$ = $1;
				}
		|		expression_list	',' expression
				{
					syntax_node_push_sibling_tail(&($1->n), &($3->n));
					$$ = $1;
				}
		;

expression:		NIL
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_NIL;
					$$ = expr;					
				}
		|		NUMBER
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_NUMBER;
					expr->value.number = $1;
					$$ = expr;
				}
		|		STRING
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_STRING;
					expr->value.string = strcopy($1);
					$$ = expr;
				}
		|		table
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_TABLE;
					syntax_node_push_child_tail(&expr->n, &($1->n));					
					$$ = expr;					
				}				
		|		variable
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_VAR;
					syntax_node_push_child_tail(&expr->n, &($1->n));
					$$ = expr;
				}
		|		function_call
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_FCALL;
					syntax_node_push_child_tail(&expr->n, &($1->n));
					$$ = expr;
				}
		|		'(' expression ')'
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_PARENTHESES;
					syntax_node_push_child_tail(&expr->n, &($2->n));
					$$ = expr;
				}
		|		expression '+' expression
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_ADD;
					syntax_node_push_child_tail(&expr->n, &($1->n));
					syntax_node_push_child_tail(&expr->n, &($3->n));
					$$ = expr;
				}
		|		expression '-' expression
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_SUB;
					syntax_node_push_child_tail(&expr->n, &($1->n));
					syntax_node_push_child_tail(&expr->n, &($3->n));
					$$ = expr;
				}
		|		expression '*' expression
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_MULTY;
					syntax_node_push_child_tail(&expr->n, &($1->n));
					syntax_node_push_child_tail(&expr->n, &($3->n));
					$$ = expr;
				}
		|		expression '/' expression
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_DIV;
					syntax_node_push_child_tail(&expr->n, &($1->n));
					syntax_node_push_child_tail(&expr->n, &($3->n));
					$$ = expr;
				}
		|		expression '%' expression
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_MOD;
					syntax_node_push_child_tail(&expr->n, &($1->n));
					syntax_node_push_child_tail(&expr->n, &($3->n));
					$$ = expr;
				}
		|		'-' expression
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_NEG;
					syntax_node_push_child_tail(&expr->n, &($2->n));
					$$ = expr;					
				}
		|		expression EQ expression
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_EQ;
					syntax_node_push_child_tail(&expr->n, &($1->n));
					syntax_node_push_child_tail(&expr->n, &($3->n));
					$$ = expr;
				}
		|		expression NE expression
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_NE;
					syntax_node_push_child_tail(&expr->n, &($1->n));
					syntax_node_push_child_tail(&expr->n, &($3->n));
					$$ = expr;
				}
		|		expression '>' expression
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_GREATER;
					syntax_node_push_child_tail(&expr->n, &($1->n));
					syntax_node_push_child_tail(&expr->n, &($3->n));
					$$ = expr;
				}
		|		expression '<' expression
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_LESS;
					syntax_node_push_child_tail(&expr->n, &($1->n));
					syntax_node_push_child_tail(&expr->n, &($3->n));
					$$ = expr;
				}
		|		expression GE expression
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_GE;
					syntax_node_push_child_tail(&expr->n, &($1->n));
					syntax_node_push_child_tail(&expr->n, &($3->n));
					$$ = expr;
				}
		|		expression LE expression
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_LE;
					syntax_node_push_child_tail(&expr->n, &($1->n));
					syntax_node_push_child_tail(&expr->n, &($3->n));
					$$ = expr;
				}
		|		NOT expression
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_NOT;
					syntax_node_push_child_tail(&expr->n, &($2->n));
					$$ = expr;
				}
		|		expression AND expression
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_AND;
					syntax_node_push_child_tail(&expr->n, &($1->n));
					syntax_node_push_child_tail(&expr->n, &($3->n));
					$$ = expr;					
				}
		|		expression OR expression
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_OR;
					syntax_node_push_child_tail(&expr->n, &($1->n));
					syntax_node_push_child_tail(&expr->n, &($3->n));
					$$ = expr;					
				}
		|		expression CONC expression
				{
					struct syntax_expression *expr = create_syntax_expression();
					expr->tag = EXPR_CONC;
					syntax_node_push_child_tail(&expr->n, &($1->n));
					syntax_node_push_child_tail(&expr->n, &($3->n));
					$$ = expr;					
				}
		;

function_call:	variable '(' expression_list ')'
				{
					$$ = create_syntax_functioncall();
					syntax_node_push_child_tail(&($$->n), &($1->n));
					syntax_node_push_child_tail(&($$->n), &($3->n));
				}
		|		variable '(' ')'
				{
					$$ = create_syntax_functioncall();
					syntax_node_push_child_tail(&($$->n), &($1->n));
				}
		;

table:			'{' '}'
				{
					$$ = create_syntax_table();
				}
		|		'{' field_list '}'
				{
					$$ = create_syntax_table();
					syntax_node_push_child_tail(&($$->n), &($2->n));
				}
		;

field_list:		field
				{
					$$ = $1;
				}
		|		field_list ',' field
				{
					syntax_node_push_sibling_tail(&($1->n), &($3->n));
					$$ = $1;
				}
		;

field:			NAME '=' expression
				{
					$$ = create_syntax_field($1);
					syntax_node_push_child_tail(&($$->n), &($3->n));
				}
		|		expression
				{
					$$ = create_syntax_field(NULL);
					syntax_node_push_child_tail(&($$->n), &($1->n));
				}
		;

variable_list:	variable
				{
					$$ = $1;
				}
		|		variable_list ',' variable
				{
					syntax_node_push_sibling_tail(&($1->n), &($3->n));
					$$ = $1;
				}
		;

variable:		NAME
				{
					struct syntax_variable *var = create_syntax_variable($1);
					var->tag = VAR_NORMAL;
					$$ = var;
				}
		|		variable '[' expression ']'
				{
					struct syntax_variable *var = create_syntax_variable(NULL);
					var->tag = VAR_INDEX;
					syntax_node_push_child_tail(&var->n, &($1->n));
					syntax_node_push_child_tail(&var->n, &($3->n));
					$$ = var;
				}
		|		variable '.' NAME
				{
					struct syntax_variable *var = create_syntax_variable($3);
					var->tag = VAR_KEY;
					syntax_node_push_child_tail(&var->n, &($1->n));
					$$ = var;
				}
		;

argument_list:	argument
				{
					$$ = $1;
				}
		|		argument_list ',' argument
				{
					syntax_node_push_sibling_tail(&($1->n), &($3->n));
					$$ = $1;
				}
		;

argument:		NAME
				{
					$$ = create_syntax_argument($1);
				}
		|		DOTS
				{
					$$ = create_syntax_argument("...");
				}
		;

%%

/* 
void yyerror(const char *msg) {
	log_error("%s:%d, %s at %s \n", yyfilename, yylineno, msg, yytext);
}
*/
