%{
#include "fox.h"
#include "symbol.h"
#include "syntax.h"

int yylex(void);

extern char yyfilename[];
extern int yylineno;
extern char *yytext;

#ifdef DEBUG
#define YYDEBUG 1
#endif

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

void gen_block_symtable(struct syntax_block *block);
void gen_node_symtable(struct syntax_block *b, struct syntax_node *n);

void gen_stmt_symtable(struct syntax_block *b, struct syntax_statement *stmt) {
	struct syntax_node *c = stmt->n.children;
	switch(stmt->tag) {
	case STMT_LOCAL_VAR:
	{
		char *p = stmt->value.name;
		int start = 0;
		int end = 0;
		while(p[end] != '\0') {
			while(p[end] != ',' && p[end] != '\0') end++;

			char tmp[end - start + 1];
			memset(tmp, 0, end - start + 1);
			strncpy(tmp, p + start, end - start);
			char *name = fox_strcat("lv_", tmp);

			struct symbol *s = symbol_create(name, &stmt->n);
			free(name);
			symbol_table_insert(b->symtab, s);
			
			if(p[end] == '\0') break;
			end++;
			start = end;
		}

		c = stmt->n.children;
		while(c) {
			gen_node_symtable(b, c);
			c = c->next;
		}
		break;
	}
	case STMT_VAR:
	{
		if(chunk_scope(&stmt->n)) {
			c = stmt->n.children;
			while(c && c->type == STX_VARIABLE) {
				struct syntax_variable *v = (struct syntax_variable *)c;
				if(v->tag == VAR_NORMAL) {
					char *name = fox_strcat("v_", v->name);
					struct symbol *s = symbol_create(name, c);
					free(name);
					symbol_table_insert(b->symtab, s);
				}
				c = c->next;
			}
		} else {
			c = stmt->n.children;
			while(c && c->type == STX_VARIABLE) {
				struct syntax_variable *v = (struct syntax_variable *)c;
				if(v->tag == VAR_NORMAL) {
					char *name1 = fox_strcat("v_", v->name);
					char *name2 = fox_strcat("lv_", v->name);
					bool found = FALSE;
					struct syntax_block *cb = b;
					while(cb) {
						struct symbol *cs1 = symbol_table_get(cb->symtab, name1);
						struct symbol *cs2 = symbol_table_get(cb->symtab, name2);
						if(cs1 || cs2) { found = TRUE; break; }
						struct syntax_node *cp = cb->n.parent;
						while(cp && cp->type != STX_BLOCK) cp = cp->parent;
						if(!cp) break;
						cb = (struct syntax_block *)cp;
					}

					if(!found) {
						struct symbol *s = symbol_create(name1, c);
						symbol_table_insert(b->symtab, s);						
					}
					free(name1);
					free(name2);
				}
				c = c->next;
			}
		}

		c = stmt->n.children;
		while(c) {
			gen_node_symtable(b, c);
			c = c->next;
		}
		break;
	}
	case STMT_FUNC:
	{
		struct syntax_function *func = (struct syntax_function *)c;
		if(func->name && !strstr(func->name, ".") && !strstr(func->name, ":")) {
			char * name = fox_strcat("f_", func->name);
			struct symbol *s = symbol_create(name, c);
			free(name);
			symbol_table_insert(b->symtab, s);
		}
		gen_node_symtable(b, stmt->n.children);
		break;
	}
	case STMT_LOCAL_FUNC:
	{
		struct syntax_function *func = (struct syntax_function *)c;
		if(func->name && !strstr(func->name, ".") && !strstr(func->name, ":")) {
			char * name = fox_strcat("lf_", func->name);
			struct symbol *s = symbol_create(name, c);
			free(name);
			symbol_table_insert(b->symtab, s);
		}
		gen_node_symtable(b, stmt->n.children);
		break;
	}
	default:
		c = stmt->n.children;
		while(c) {
			gen_node_symtable(b, c);
			c = c->next;
		}
	}
}

void gen_node_symtable(struct syntax_block *b, struct syntax_node *n) {
	struct syntax_node *c = n->children;
	switch(n->type)
	{
	case STX_BLOCK:
		gen_block_symtable((struct syntax_block *)n);
		break;
	case STX_STATEMENT:
		gen_stmt_symtable(b, (struct syntax_statement *)n);
		break;
	default:
		while(c) {
			gen_node_symtable(b, c);
			c = c->next;
		}
	}
}

void gen_block_symtable(struct syntax_block *block) {
	struct syntax_node *c = block->n.children;
	while(c) {
		gen_node_symtable(block, c);
		c = c->next;
	}
}

void gen_chunk_symtables(struct syntax_chunk *chunk) {
	gen_block_symtable((struct syntax_block *)chunk->n.children);
}

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
	struct syntax_table *tab;
	struct syntax_field *field;
}

%start					program

%token					FUNCTION LOCAL
%token					NIL BTRUE BFALSE						

%token					IF THEN ELSE ELSEIF WHILE DO REPEAT UNTIL FOR BREAK END RETURN GOTO IN LABEL
%token					AND OR GE LE EQ NE CONC DOTS LSHIFT RSHIFT FDIV

%token<string>			NAME STRING
%token<string>			NUMBER

%type<chunk>			chunk
%type<block>			block

%type<stmt>				stmtlist stmt retstmt basestmt ifstmt elsestmt loopstmt varstmt funcstmt
%type<exp>				explist exp prefixexp constexp primaryexp funcexp tableexp

%type<var> 				varlist var
%type<func>				funcbody funcdef funclambda
%type<fcall>			funcall
%type<arg>				arglist
%type<tab>				table
%type<field>			fieldlist field

%type<string>			funcname basefuncname
%type<string>			namelist
%type<string>			label
%type<string>			parlist

%left					OR
%left					AND					
%left 					EQ NE '<' '>' GE LE
%left					'|'
%left					'~'
%left					'&'
%left					LSHIFT RSHIFT
%left					CONC
%left					'+' '-'
%left					'*' '/' '%' FDIV
%left 					NOT '#' OPNEG OPBNOT
%left					'^'
												
%%

program:		chunk
				{
					parse_tree->root = &($1->n);
					gen_chunk_symtables($1);
				}
		;

chunk:			block
				{
					struct syntax_chunk *chunk = create_syntax_chunk();
					syntax_node_push_child_tail(&chunk->n, &($1->n));
					chunk->n.lineno = yylineno;
					$$ = chunk;
				}
		;

block:			/* empty */
				{
					$$ = create_syntax_block();
					$$->n.lineno = yylineno;
				}
		|		retstmt
				{
					struct syntax_block *block = create_syntax_block();
					block->n.lineno = yylineno;
					syntax_node_push_child_tail(&block->n, &($1->n));
					$$ = block;
				}
		|		stmtlist
				{
					struct syntax_block *block = create_syntax_block();
					block->n.lineno = yylineno;
					syntax_node_push_child_tail(&block->n, &($1->n));
					$$ = block;
				}
		|		stmtlist retstmt
				{
					struct syntax_block *block = create_syntax_block();
					block->n.lineno = yylineno;
					syntax_node_push_child_tail(&block->n, &($1->n));
					syntax_node_push_child_tail(&block->n, &($2->n));
					$$ = block;
				}
		;

stmtlist:		stmt
		|		stmtlist stmt
				{
					syntax_node_push_sibling_tail(&($1->n), &($2->n));
					$$ = $1;
				}
		;

stmt:			basestmt
		|		loopstmt
		|		ifstmt
		|		varstmt
		|		funcstmt				
		;

retstmt:		RETURN retend
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_RETURN;
					$$ = stmt;
				}
		|		RETURN explist retend
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_RETURN;
					syntax_node_push_child_tail(&stmt->n, &($2->n));
					$$ = stmt;
				}
		;

retend:			/* empty */
		|		';'
		;

basestmt:		';'
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_EMPTY;
					$$ = stmt;
				}
		|		label
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_LABEL;
					stmt->value.name = $1;
					$$ = stmt;
				}
		|		BREAK
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_BREAK;
					$$ = stmt;
				}
		|		GOTO NAME
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_GOTO;
					stmt->value.name = $2;
					$$ = stmt;			
				}
		|		DO block END
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_DO;
					syntax_node_push_child_tail(&stmt->n, &($2->n));
					$$ = stmt;
				}
		;

loopstmt:		WHILE exp DO block END
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_WHILE;
					syntax_node_push_child_tail(&stmt->n, &($2->n));
					syntax_node_push_child_tail(&stmt->n, &($4->n));
					$$ = stmt;
				}
		|		REPEAT block UNTIL exp
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_REPEAT;
					syntax_node_push_child_tail(&stmt->n, &($2->n));
					syntax_node_push_child_tail(&stmt->n, &($4->n));
					$$ = stmt;					
				}
		|		FOR namelist IN explist DO block END
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_FOR_IN;
					stmt->value.name = $2;
					syntax_node_push_child_tail(&stmt->n, &($4->n));
					syntax_node_push_child_tail(&stmt->n, &($6->n));					
					$$ = stmt;
				}
		|		FOR NAME '=' exp ',' exp ',' exp DO block END
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_FOR_IT;
					stmt->value.name = $2;
					syntax_node_push_child_tail(&stmt->n, &($4->n));
					syntax_node_push_child_tail(&stmt->n, &($6->n));
					syntax_node_push_child_tail(&stmt->n, &($8->n));
					syntax_node_push_child_tail(&stmt->n, &($10->n));					
					$$ = stmt;
				}
		|		FOR NAME '=' exp ',' exp DO block END
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_FOR_IT;
					stmt->value.name = $2;
					syntax_node_push_child_tail(&stmt->n, &($4->n));
					syntax_node_push_child_tail(&stmt->n, &($6->n));
					syntax_node_push_child_tail(&stmt->n, &($8->n));
					$$ = stmt;					
				}
		;

ifstmt:			IF exp THEN block elsestmt END
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_IF;
					syntax_node_push_child_tail(&stmt->n, &($2->n));
					syntax_node_push_child_tail(&stmt->n, &($4->n));
					syntax_node_push_child_tail(&stmt->n, &($5->n));
					$$ = stmt;
				}
		;

elsestmt:		/* empty */
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_EMPTY;
					$$ = stmt;
				}
		|		ELSE block
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_ELSE;
					syntax_node_push_child_tail(&stmt->n, &($2->n));
					$$ = stmt;		
				}
		|		ELSEIF exp THEN block elsestmt
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_ELSEIF;
					syntax_node_push_child_tail(&stmt->n, &($2->n));
					syntax_node_push_child_tail(&stmt->n, &($4->n));
					syntax_node_push_child_tail(&stmt->n, &($5->n));
					$$ = stmt;
				}
		;

varstmt:		LOCAL namelist
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_LOCAL_VAR;
					stmt->value.name = $2;
					$$ = stmt;
				}
		|		LOCAL namelist '=' explist
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_LOCAL_VAR;
					stmt->value.name = $2;
					syntax_node_push_child_tail(&stmt->n, &($4->n));
					$$ = stmt;
				}
		|		varlist '=' explist
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_VAR;
					syntax_node_push_child_tail(&stmt->n, &($1->n));
					syntax_node_push_child_tail(&stmt->n, &($3->n));
					$$ = stmt;
				}				
		;

funcstmt:		/* funcall will lead to reduce/reduce conflict */
				prefixexp
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_FCALL;
					syntax_node_push_child_tail(&stmt->n, &($1->n));
					$$ = stmt;
				}
		|		funcdef
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_FUNC;
					syntax_node_push_child_tail(&stmt->n, &($1->n));
					$$ = stmt;
				}
		|		LOCAL funcdef
				{
					struct syntax_statement *stmt = create_syntax_statement();
					stmt->n.lineno = yylineno;
					stmt->tag = STMT_LOCAL_FUNC;
					syntax_node_push_child_tail(&stmt->n, &($2->n));
					$$ = stmt;
				}
		;

label:			LABEL NAME LABEL
				{
					$$ = $2;
				}
		;

namelist:		NAME
		|		namelist ',' NAME
				{
					char *s0 = fox_strcat($1, ",");
					char *s1 = fox_strcat(s0, $3);
					free($1);
					free($3);
					free(s0);
					$$ = s1;					
				}
		;

explist:		exp
		|		explist ',' exp
				{
					syntax_node_push_sibling_tail(&($1->n), &($3->n));
					$$ = $1;					
				}
		;

exp:			prefixexp
		|		constexp
		|		primaryexp
		|		funcexp
		|		tableexp
		;

prefixexp:		var
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_VAR;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					$$ = exp;
				}
		|		funcall
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_FCALL;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					$$ = exp;
				}
		|		'(' exp ')'
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_PARENTHESIS;
					syntax_node_push_child_tail(&exp->n, &($2->n));
					$$ = exp;
				}

		;

constexp:		NIL
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_NIL;
					$$ = exp;
				}
		|		BTRUE
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_TRUE;
					$$ = exp;
				}
		|		BFALSE
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_FALSE;
					$$ = exp;
				}
		|		NUMBER
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_NUMBER;
					exp->value.string = $1;
					$$ = exp;					
				}
		|		STRING
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_STRING;
					exp->value.string = $1;
					$$ = exp;
				}
		|		DOTS
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_DOTS;
					$$ = exp;					
				}
		;

primaryexp:		exp '+' exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_ADD;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp '-' exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_SUB;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp '*' exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_MUL;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp '/' exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_DIV;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp FDIV exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_FDIV;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp '^' exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_EXP;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp '%' exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_MOD;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp '&' exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_BAND;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp '|' exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_BOR;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp '~' exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_XOR;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp LSHIFT exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_LSHIFT;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp RSHIFT exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_RSHIFT;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp CONC exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_CONC;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp '<' exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_LESS;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp '>' exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_GREATER;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp LE exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_LE;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp GE exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_GE;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp EQ exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_EQ;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp NE exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_NE;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp AND exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_AND;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		exp OR exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_OR;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					syntax_node_push_child_tail(&exp->n, &($3->n));
					$$ = exp;
				}
		|		'~' exp %prec OPBNOT
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_BNOT;
					syntax_node_push_child_tail(&exp->n, &($2->n));
					$$ = exp;
				}
		|		'-' exp %prec OPNEG
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_NEG;
					syntax_node_push_child_tail(&exp->n, &($2->n));
					$$ = exp;
				}
		|		NOT exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_NOT;
					syntax_node_push_child_tail(&exp->n, &($2->n));
					$$ = exp;
				}
		|		'#' exp
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_LEN;
					syntax_node_push_child_tail(&exp->n, &($2->n));
					$$ = exp;
				}
		;

funcexp:		funclambda
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_FUNC;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					$$ = exp;
				}
		;

tableexp:		table
				{
					struct syntax_expression *exp = create_syntax_expression();
					exp->n.lineno = yylineno;
					exp->tag = EXP_TABLE;
					syntax_node_push_child_tail(&exp->n, &($1->n));
					$$ = exp;
				}
		;

varlist:		var
		|		varlist ',' var
				{
					syntax_node_push_sibling_tail(&($1->n), &($3->n));
					$$ = $1;
				}
		;

var:			NAME
				{
					struct syntax_variable *var = create_syntax_variable();
					var->n.lineno = yylineno;
					var->tag = VAR_NORMAL;
					var->name = $1;
					$$ = var;
				}
		|		prefixexp '[' exp ']'
				{
					struct syntax_variable *var = create_syntax_variable();
					var->n.lineno = yylineno;
					var->tag = VAR_INDEX;
					syntax_node_push_child_tail(&var->n, &($1->n));
					syntax_node_push_child_tail(&var->n, &($3->n));
					$$ = var;
				}
		|		prefixexp '.' NAME
				{
					struct syntax_variable *var = create_syntax_variable();
					var->n.lineno = yylineno;
					var->tag = VAR_KEY;
					syntax_node_push_child_tail(&var->n, &($1->n));
					var->name = $3;
					$$ = var;	
				}
		;

funcdef:		FUNCTION funcname funcbody
				{
					$3->name = $2;
					$$ = $3;
				}
		;

funclambda:		FUNCTION funcbody
				{
					$$ = $2;
				}
		;

basefuncname:	NAME
		|		basefuncname '.' NAME
				{
					char *s0 = fox_strcat($1, ".");
					char *s1 = fox_strcat(s0, $3);
					free($1);
					free($3);
					free(s0);
					$$ = s1;
				}
		;

funcname:		basefuncname
		|		basefuncname ':' NAME
				{
					char *s0 = fox_strcat($1, ":");
					char *s1 = fox_strcat(s0, $3);
					free($1);
					free($3);
					free(s0);
					$$ = s1;
				}
		;

funcbody:		'(' ')' block END
				{
					struct syntax_function *func = create_syntax_function();
					func->n.lineno = yylineno;
					syntax_node_push_child_tail(&func->n, &($3->n));
					$$ = func;
				}
		|		'(' parlist ')' block END
				{
					struct syntax_function *func = create_syntax_function();
					func->n.lineno = yylineno;
					func->pars = $2;
					syntax_node_push_child_tail(&func->n, &($4->n));
					$$ = func;
				}
		;

parlist:		namelist
		|		namelist ',' DOTS
				{
					char *s0 = fox_strcat($1, ",");
					char *s1 = fox_strcat(s0, "...");
					free($1);
					free(s0);
					$$ = s1;
				}
		|		DOTS
				{
					$$ = fox_strdup("...");
				}
		;

funcall:		prefixexp arglist
				{
					struct syntax_functioncall *fcall = create_syntax_functioncall();
					fcall->n.lineno = yylineno;
					syntax_node_push_child_tail(&fcall->n, &($1->n));
					syntax_node_push_child_tail(&fcall->n, &($2->n));
					$$ = fcall;
				}
		|		prefixexp ':' NAME arglist
				{
					struct syntax_functioncall *fcall = create_syntax_functioncall();
					fcall->n.lineno = yylineno;
					fcall->name = $3;
					syntax_node_push_child_tail(&fcall->n, &($1->n));
					syntax_node_push_child_tail(&fcall->n, &($4->n));
					$$ = fcall;
				}

arglist:		'(' ')'
				{
					struct syntax_argument *arg = create_syntax_argument();
					arg->n.lineno = yylineno;
					arg->tag = ARG_EMPTY;
					$$ = arg;
				}
		|		'(' explist ')'
				{
					struct syntax_argument *arg = create_syntax_argument();
					arg->n.lineno = yylineno;
					arg->tag = ARG_NORMAL;
					syntax_node_push_child_tail(&arg->n, &($2->n));
					$$ = arg;
				}
		|		STRING
				{
					struct syntax_argument *arg = create_syntax_argument();
					arg->n.lineno = yylineno;
					arg->tag = ARG_STRING;
					arg->name = $1;
					$$ = arg;		
				}
		|		table
				{
					struct syntax_argument *arg = create_syntax_argument();
					arg->n.lineno = yylineno;
					arg->tag = ARG_TABLE;
					syntax_node_push_child_tail(&arg->n, &($1->n));
					$$ = arg;					
				}
		;

table:			'{' fieldlist '}'
				{
					struct syntax_table *t = create_syntax_table();
					t->n.lineno = yylineno;
					syntax_node_push_child_tail(&(t->n), &($2->n));
					$$ = t;
				}
		|		'{' '}'
				{
					struct syntax_table *t = create_syntax_table();
					t->n.lineno = yylineno;
					$$ = t;
				}
		;

fieldlist:		field
		|		fieldlist fieldsep field
				{
					syntax_node_push_sibling_tail(&($1->n), &($3->n));
					$$ = $1;
				}
		|		fieldlist fieldsep
				{
					$$ = $1;
				}
		;

field:			'[' exp ']' '=' exp
				{
					struct syntax_field * f = create_syntax_field();
					f->n.lineno = yylineno;
					f->tag = FIELD_INDEX;
					syntax_node_push_child_tail(&f->n, &($2->n));	
					syntax_node_push_child_tail(&f->n, &($5->n));
					$$ = f;
				}
		|		NAME '=' exp
				{
					struct syntax_field * f = create_syntax_field();
					f->n.lineno = yylineno;
					f->tag = FIELD_KEY;
					f->name = $1;
					syntax_node_push_child_tail(&f->n, &($3->n));
					$$ = f;					
				}
		|		exp
				{
					struct syntax_field * f = create_syntax_field();
					f->n.lineno = yylineno;
					f->tag = FIELD_SINGLE;
					syntax_node_push_child_tail(&f->n, &($1->n));
					$$ = f;					
				}
		;

fieldsep:		','
		|		';'
		;

%%

#if YYDEBUG
void yyprint(FILE *file, int type, YYSTYPE value)
{
	switch(type) {
	case NUMBER:
		fprintf(file, "\n[YACC]%s, %s\n", "number", value.string);
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
