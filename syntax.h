#ifndef __SYNTAX_H__
#define __SYNTAX_H__

struct syntax_node {
	struct syntax_node *next;
	struct syntax_node *parent;
	struct syntax_node *children;
	int type;
};

typedef void (*syntax_node_handler)(struct syntax_node *n);

void syntax_node_init(struct syntax_node *n, int ty);
void syntax_node_push_child_head(struct syntax_node *p, struct syntax_node *c);
void syntax_node_push_child_tail(struct syntax_node *p, struct syntax_node *c);
void syntax_node_push_sibling_head(struct syntax_node *p, struct syntax_node *c);
void syntax_node_push_sibling_tail(struct syntax_node *p, struct syntax_node *c);
int syntax_node_depth(struct syntax_node *n);
int syntax_node_children_count(struct syntax_node *n);
int syntax_node_sibling_count(struct syntax_node *n);
void syntax_node_walk(struct syntax_node *n, syntax_node_handler h);
void syntax_node_release(struct syntax_node *n);

struct syntax_tree {
	struct syntax_node *root;
};

struct syntax_tree *syntax_tree_create();
void syntax_tree_release(struct syntax_tree *t);
void syntax_tree_walk(struct syntax_tree *t, syntax_node_handler h);

enum syntax_node_type {
	SNT_CHUNK,
	SNT_BLOCK,
	SNT_STATEMENT,
	SNT_EXPRESSION,
	SNT_VARIABLE,
	SNT_FUNCTION,
	SNT_FUNCTIONCALL,	
	SNT_ARGUMENT,
	SNT_TABLE,
	SNT_FIELD,	
};

struct syntax_chunk {
	struct syntax_node n;
};

struct syntax_block {
	struct syntax_node n;
};

enum syntax_statement_tag {
	STMT_INVALID,
	STMT_EMPTY,
	STMT_LABEL,
	STMT_BREAK,
	STMT_GOTO,

	STMT_VAR,
	STMT_LOCAL_VAR,	
	STMT_FCALL,
	STMT_FUNC,
	STMT_LOCAL_FUNC,

	STMT_DO,
	STMT_WHILE,
	STMT_REPEAT,
	STMT_FOR_IN,
	STMT_FOR_IT,
	
	STMT_IF,
	STMT_ELSE,
	STMT_ELSEIF,

	STMT_RETURN,
};

struct syntax_statement {
	struct syntax_node n;
	int tag;
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
	EXP_FCALL,
	EXP_FUNC,
	EXP_PARENTHESIS,

	EXP_ADD,
	EXP_SUB,
	EXP_MUL,
	EXP_DIV,
	EXP_FDIV,
	EXP_EXP,
	EXP_MOD,

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

struct syntax_expression {
	struct syntax_node n;

	int tag;
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

struct syntax_variable {
	struct syntax_node n;

	int tag;
	char *name;
};

struct syntax_function {
	struct syntax_node n;
	char *name;
};

struct syntax_functioncall {
	struct syntax_node n;
	char *name;
};

enum syntax_argument_tag {
	ARG_INVALID,
	ARG_EMPTY,
	ARG_STRING,
	ARG_TABLE,
	ARG_NORMAL,
};

struct syntax_argument {
	struct syntax_node n;

	int tag;
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

struct syntax_field {
	struct syntax_node n;

	int tag;
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
