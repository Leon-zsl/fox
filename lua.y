%{
#include "symbol.h"
#include "syntax.h"
#include "fox.h"

extern int yylineno;
extern char *yytext;

void yyerror(const char *error);
int yylex(void);
%}

%union {
	int vint;
	double vfloat;
	char* vstring;
}

%start program

%token REQUIRE				
%token FUNCTION
%token LOCAL
%token RETURN
%token END						
%token NIL

%token IF THEN ELSE ELSEIF WHILE REPEAT UNTIL
%token AND OR NOT GREATER GE LESS LE EQUAL NE
%token ADD SUB MULTY DIV MOD CONCAT
%token ASSIGN FIELD LPARENTHESE RPARENTHESE LBRACKET RBRACKET

%token NAME
%token STRING
%token NUMBER

%right ASSIGN						
%left AND OR 						
%left GREATER GE LESS LE EQUAL NE
%left CONCAT
%left ADD SUB
%left MULTY DIV MOD
%right NOT
%left FIELD

%%

program: statement_list { printf("program\n"); }
		;

require_list: 	  /* empty */
				 | require_list require { printf("require list\n"); }
		;

require:		REQUIRE '(' NAME ')'
				{
					
				}
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

declare_list:	
		;

declare:		
		;

block_list:		
		;

block:			
		;

variable_list:	
		;

variable:		
		;

assign:			
		;

%%

void yyerror(const char *msg) {
	log_error("%d: %s  at  %s \n", yylineno, msg, yytext);
}
