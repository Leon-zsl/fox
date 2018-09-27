#ifndef __SYNTAX_H__
#define __SYNTAX_H__

enum syntax_node_type {
	STX_CHUNK,
	STX_BLOCK,
	STX_STATEMENT,
	STX_EXPRESSION,
	STX_VARIABLE,
	STX_FUNCTION,
	STX_FUNCTIONCALL,	
	STX_ARGUMENT,
	STX_TABLE,
	STX_FIELD,
};

const char *syntax_node_type_string(enum syntax_node_type ty);

struct syntax_node {
	struct syntax_node *next;
	struct syntax_node *parent;
	struct syntax_node *children;
	enum syntax_node_type type;
	int lineno;
};

typedef void (*syntax_node_handler)(struct syntax_node *n);

void syntax_node_init(struct syntax_node *n, enum syntax_node_type ty);
void syntax_node_push_child_head(struct syntax_node *p, struct syntax_node *c);
void syntax_node_push_child_tail(struct syntax_node *p, struct syntax_node *c);
void syntax_node_push_sibling_head(struct syntax_node *p, struct syntax_node *c);
void syntax_node_push_sibling_tail(struct syntax_node *p, struct syntax_node *c);
int syntax_node_depth(struct syntax_node *n);
int syntax_node_children_count(struct syntax_node *n);
int syntax_node_sibling_count(struct syntax_node *n);
struct syntax_node *syntax_node_child(struct syntax_node *n, int index);
struct syntax_node *syntax_node_sibling(struct syntax_node *n, int index);
void syntax_node_walk(struct syntax_node *n, syntax_node_handler h);
void syntax_node_release(struct syntax_node *n);

struct syntax_tree {
	struct syntax_node *root;
};

struct syntax_tree *syntax_tree_create();
void syntax_tree_release(struct syntax_tree *t);
void syntax_tree_walk(struct syntax_tree *t, syntax_node_handler h);

struct syntax_chunk {
	struct syntax_node n;
};

struct syntax_block {
	struct syntax_node n;
	struct symbol_table *sym;
};

struct symbol_table *syntax_node_symbol_table(struct syntax_node *n);
struct symbol_table *syntax_node_parent_symbol_table(struct syntax_node *n);
int chunk_scope(struct syntax_node *n);
int func_scope(struct syntax_node *n);

enum syntax_statement_tag {
	STMT_INVALID,
	STMT_EMPTY,
	STMT_LABEL,
	STMT_BREAK,
	STMT_GOTO,
	STMT_RETURN,
	
	STMT_DO,
	STMT_WHILE,
	STMT_REPEAT,
	STMT_FOR_IN,
	STMT_FOR_IT,
	
	STMT_IF,
	STMT_ELSE,
	STMT_ELSEIF,

	STMT_VAR,
	STMT_LOCAL_VAR,
	
	STMT_FUNC,
	STMT_LOCAL_FUNC,
	STMT_FCALL,
};

const char *syntax_statement_tag_string(enum syntax_statement_tag tag);

struct syntax_statement {
	struct syntax_node n;
	enum syntax_statement_tag tag;
	union {
		char *name;
	} value;
};

enum syntax_expression_tag {
	EXP_INVALID,
	EXP_NIL,
	EXP_TRUE,
	EXP_FALSE,
	EXP_NUMBER,
	EXP_STRING,
	EXP_DOTS,
	EXP_TABLE,
	EXP_VAR,
	EXP_FUNC,
	EXP_FCALL,
	EXP_PARENTHESIS,

	EXP_ADD,
	EXP_SUB,
	EXP_MUL,
	EXP_DIV,
	EXP_MOD,
	EXP_FDIV,
	EXP_EXP,

	EXP_BAND,
	EXP_BOR,
	EXP_XOR,
	EXP_LSHIFT,
	EXP_RSHIFT,

	EXP_CONC,

	EXP_LESS,
	EXP_GREATER,
	EXP_LE,
	EXP_GE,
	EXP_EQ,
	EXP_NE,
	EXP_AND,
	EXP_OR,

	EXP_NOT,
	EXP_LEN,
	EXP_NEG,
	EXP_BNOT,
};

const char *syntax_expression_tag_string(enum syntax_expression_tag tag);

struct syntax_expression {
	struct syntax_node n;

	enum syntax_expression_tag tag;
	union {
		double number;
		char *string;
	} value;
};

enum syntax_variable_tag {
	VAR_INVALID,
	VAR_NORMAL,
	VAR_INDEX,
	VAR_KEY,
};

const char *syntax_variable_tag_string(enum syntax_variable_tag tag);

struct syntax_variable {
	struct syntax_node n;

	enum syntax_variable_tag tag;
	char *name;
};

struct syntax_function {
	struct syntax_node n;
	char *name;
	char *pars;
};

struct syntax_functioncall {
	struct syntax_node n;
	char *name;
};

enum syntax_argument_tag {
	ARG_INVALID,
	ARG_EMPTY,
	ARG_NORMAL,	
	ARG_STRING,
	ARG_TABLE,
};

const char *syntax_argument_tag_string(enum syntax_argument_tag tag);

struct syntax_argument {
	struct syntax_node n;

	enum syntax_argument_tag tag;
	char *name;
};

struct syntax_table {
	struct syntax_node n;
};

enum syntax_field_tag {
	FIELD_INVALID,
	FIELD_SINGLE,
	FIELD_INDEX,
	FIELD_KEY,
};

const char *syntax_field_tag_string(enum syntax_field_tag tag);

struct syntax_field {
	struct syntax_node n;

	enum syntax_field_tag tag;
	char *name;
};

struct syntax_chunk *create_syntax_chunk();
void release_syntax_chunk(struct syntax_chunk *chunk);

struct syntax_block *create_syntax_block();
void release_syntax_block(struct syntax_block *block);

struct syntax_statement *create_syntax_statement();
void release_syntax_statement(struct syntax_statement *stmt);

struct syntax_expression *create_syntax_expression();
void release_syntax_expression(struct syntax_expression *exp);

struct syntax_variable *create_syntax_variable();
void release_syntax_variable(struct syntax_variable *var);

struct syntax_function *create_syntax_function();
void release_syntax_function(struct syntax_function *func);

struct syntax_functioncall *create_syntax_functioncall();
void release_syntax_functioncall(struct syntax_functioncall *fcall);

struct syntax_argument *create_syntax_argument();
void release_syntax_argument(struct syntax_argument *arg);

struct syntax_table *create_syntax_table();
void release_syntax_table(struct syntax_table *table);

struct syntax_field *create_syntax_field();
void release_syntax_field(struct syntax_field *field);

#endif
