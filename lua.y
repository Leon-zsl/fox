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

%}

%union {
	double number;
	char* string;
	struct syntax_tree *tree;
	struct syntax_statement *stmt;
	struct syntax_function *func;
	struct syntax_requirement *req;
	struct syntax_declaration *decl;
	struct syntax_block *block;
	struct syntax_expression *expr;
	struct syntax_variable *var;
	struct syntax_comment *cmt;
}

%start program

%token REQUIRE FUNCTION LOCAL RETURN END NIL

%token IF THEN ELSE ELSEIF WHILE REPEAT UNTIL
%token AND OR NOT GREATER GE LESS LE EQ NE
%token ADD SUB MULTY DIV MOD CONCAT
%token ASSIGN FIELD LPARENTHESE RPARENTHESE LBRACKET RBRACKET COMMA COLON

%token<string> COMMENT NAME STRING
%token<number> NUMBER

%type<tree> program
%type<stmt> statement statement_list
%type<func>	function function_list
%type<req> requirement requirement_list
%type<decl> declaration declaration_list
%type<block> block block_list
%type<expr> expression expression_list
%type<var>	variable variable_list
%type<cmt> comment

%right ASSIGN				
%left AND OR 						
%left GREATER GE LESS LE EQUAL NE
%left CONCAT
%left ADD SUB
%left MULTY DIV MOD
%right NOT
%left FIELD

%%

program: statement_list
				{
					printf("program\n");
					struct syntax_tree *t = syntax_tree_create();
					t->root = $1;
					$$ = t;
				}
		;

requirement_list: 	  /* empty */
				 | requirement_list requirement { printf("require list\n"); }
		;

requirement:		REQUIRE '(' NAME ')' { }
		;

statement_list:	
		;

statement:
		;

function_list:	
		;

function:
		;

function_call:	
		;

expression_list:
		;

expression:		
		;

declaration_list:	
		;

declaration:		
		;

block_list:		
		;

block:			
		;

variable_list:	
		;

variable:		
		;

comment:		COMMENT { $$ = create_syntax_comment(yylval.string); }
		;

%%

/* 
void yyerror(const char *msg) {
	log_error("%s:%d, %s at %s \n", yyfilename, yylineno, msg, yytext);
}
*/
