#ifndef __PARSER_H__
#define __PARSER_H__

struct syntax_node {
	struct syntax_node *next;
	struct syntax_node *parent;
	struct syntax_node *children;
	int type;
};

typedef void (*syntax_node_handler)(struct syntax_node *n);

struct syntax_tree {
	struct syntax_node *root;
};

extern struct syntax_tree *parse_tree;
struct syntax_tree *parse(const char *filename);
struct syntax_tree *syntax_tree_create();
void syntax_tree_release(struct syntax_tree *t);
void syntax_tree_walk(struct syntax_tree *t, syntax_node_handler h);

enum syntax_node_type {
	SNT_STATEMENT,
	SNT_FUNCTION,
	SNT_REQUIREMENT,
	SNT_DECLARATION,
	SNT_BLOCK,
	SNT_EXPRESSION,
	SNT_VARIABLE,
	SNT_COMMENT,
};

struct syntax_statement {
	struct syntax_node n;
};

struct syntax_function {
	struct syntax_node n;
};

struct syntax_requirement {
	struct syntax_node n;
};

struct syntax_declaration {
	struct syntax_node n;
};

struct syntax_block {
	struct syntax_node n;
};

struct syntax_expression {
	struct syntax_node n;
};

struct syntax_variable {
	struct syntax_node n;
	char *name;
};

struct syntax_comment {
	struct syntax_node n;
	char *comment;
};

struct syntax_statement *create_syntax_statement();
void release_syntax_statement(struct syntax_statement *stmt);

struct syntax_function *create_syntax_function();
void release_syntax_function(struct syntax_function *func);

struct syntax_requirement *create_syntax_requirement();
void release_syntax_requirement(struct syntax_requirement *req);

struct syntax_declaration *create_syntax_declaration();
void release_syntax_declaration(struct syntax_declaration *decl);

struct syntax_block *create_syntax_block();
void release_syntax_block(struct syntax_block *block);

struct syntax_expression *create_syntax_expression();
void release_syntax_expression(struct syntax_expression *expr);

struct syntax_variable *create_syntax_variable(const char *name, struct syntax_expression *expr);
void release_syntax_variable(struct syntax_variable *var);

struct syntax_comment *create_syntax_comment(const char *comment);
void release_syntax_comment(struct syntax_comment *cmt);

#endif
