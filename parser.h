#ifndef __PARSER_H__
#define __PARSER_H__

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

extern struct syntax_tree *parse_tree;
struct syntax_tree *parse(const char *filename);
struct syntax_tree *syntax_tree_create();
void syntax_tree_release(struct syntax_tree *t);
void syntax_tree_walk(struct syntax_tree *t, syntax_node_handler h);

enum syntax_node_type {
	SNT_PROGRAM,
	SNT_CHUNK,
	SNT_REQUIREMENT,
	SNT_FUNCTION,
	SNT_FUNCTIONCALL,	
	SNT_BLOCK,
	SNT_STATEMENT,
	SNT_EXPRESSION,
	SNT_TABLE,
	SNT_FIELD,
	SNT_VARIABLE,
	SNT_ARGUMENT,
	SNT_RETURN,
};

struct syntax_program {
	struct syntax_node n;
};

struct syntax_chunk {
	struct syntax_node n;
};

struct syntax_requirement {
	struct syntax_node n;
	char* name;
};

struct syntax_function {
	struct syntax_node n;
	char *name;
};

struct syntax_functioncall {
	struct syntax_node n;
};

struct syntax_block {
	struct syntax_node n;
};

enum syntax_statement_tag {
	STMT_IF,
	STMT_ELSE_EMPTY,
	STMT_ELSE,
	STMT_ELSEIF,

	STMT_WHILE,
	STMT_REPEAT,
	STMT_FCALL,
	STMT_VARS,
	STMT_LOCAL_VARS,
};

struct syntax_statement {
	struct syntax_node n;
	int tag;
};

enum syntax_expression_tag {
	EXPR_NIL,
	EXPR_NUMBER,
	EXPR_STRING,
	EXPR_TABLE,
	EXPR_VAR,
	EXPR_FCALL,

	EXPR_PARENTHESES,

	EXPR_ADD,
	EXPR_SUB,
	EXPR_MULTY,
	EXPR_DIV,
	EXPR_MOD,
	EXPR_NEG,
	
	EXPR_EQ,
	EXPR_NE,
	EXPR_GREATER,
	EXPR_LESS,
	EXPR_GE,
	EXPR_LE,
	
	EXPR_NOT,
	EXPR_AND,
	EXPR_OR,

	EXPR_CONC,
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
	VAR_NORMAL,
	VAR_INDEX,
	VAR_KEY,
};

struct syntax_variable {
	struct syntax_node n;

	int tag;
	char *name;
};

struct syntax_argument {
	struct syntax_node n;
	char *name;
};

struct syntax_table {
	struct syntax_node n;
};

struct syntax_field {
	struct syntax_node n;
	char *name;
};

struct syntax_return {
	struct syntax_node n;
};

struct syntax_program *create_syntax_program();
void release_syntax_program(struct syntax_program *program);

struct syntax_chunk *create_syntax_chunk();
void release_syntax_chunk(struct syntax_chunk *chunk);

struct syntax_requirement *create_syntax_requirement(const char *name);
void release_syntax_requirement(struct syntax_requirement *req);

struct syntax_function *create_syntax_function(const char *name);
void release_syntax_function(struct syntax_function *func);

struct syntax_functioncall *create_syntax_functioncall();
void release_syntax_functioncall(struct syntax_functioncall *fcall);

struct syntax_block *create_syntax_block();
void release_syntax_block(struct syntax_block *block);

struct syntax_statement *create_syntax_statement();
void release_syntax_statement(struct syntax_statement *stmt);

struct syntax_expression *create_syntax_expression();
void release_syntax_expression(struct syntax_expression *expr);

struct syntax_variable *create_syntax_variable(const char *name);
void release_syntax_variable(struct syntax_variable *var);

struct syntax_argument *create_syntax_argument(const char *name);
void release_syntax_argument(struct syntax_argument *arg);

struct syntax_table *create_syntax_table();
void release_syntax_table(struct syntax_table *table);

struct syntax_field *create_syntax_field(const char *name);
void release_syntax_field(struct syntax_field *field);

struct syntax_return *create_syntax_return();
void release_syntax_return(struct syntax_return *ret);

#endif
