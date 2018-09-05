#include "fox.h"
#include "symbol.h"
#include "parser.h"

#define YY_NULL 0

int yylex(void);
void yyset_in(FILE *in);
void yyset_out(FILE *out);
void yyset_lineno(int lineno);
void yyset_filename(const char *name);

struct syntax_tree *parse_tree = NULL;

struct syntax_tree *parse(const char *filename) {
	FILE *fp = fopen(filename, "rb");
	if(!fp) {
		log_error("open file failed %s", filename);
		return NULL;
	}

	//a new parse tree
	parse_tree = NULL;

	yyset_in(fp);
	yyset_out(stdout);
	yyset_filename(filename);
	yyset_lineno(1);

	/* yylex function test */
	while(yylex() != YY_NULL);

	/* int val = yyparse(); */
	/* if(val) { */
	/* 	fclose(fp); */
	/* 	return val; */
	/* } */

	fclose(fp);
	return parse_tree;
}

struct syntax_tree *syntax_tree_create() {
	struct syntax_tree *t = (struct syntax_tree *)malloc(sizeof(struct syntax_tree));
	t->root = NULL;
	return t;
}

void syntax_tree_release(struct syntax_tree *t) {
	if(!t) return;
	if(t->root)	syntax_node_release(t->root);
	free(t);
}

void syntax_tree_walk(struct syntax_tree *t, syntax_node_handler h) {
	//do nothing now
}

void syntax_node_init(struct syntax_node *n, int ty) {
	n->next = NULL;
	n->parent = NULL;
	n->children = NULL;

	n->type = ty;
}

static void syntax_node_release_children(struct syntax_node *n) {
	struct syntax_node *c = n->children;
	while(c != NULL) {
		struct syntax_node *cc = c;
		c = c->next;
		syntax_node_release(cc);
	}
}

static void syntax_node_add_child(struct syntax_node *p, struct syntax_node *c) {
	struct syntax_node *cc = p->children;
	struct syntax_node *pc = NULL;
	while(cc != NULL) {
		pc = cc;
		cc = cc->next;
	}
	if(pc) {
		pc->next = c;	
	} else {
		p->children = c;
	}
	c->parent = p;
}

void syntax_node_release(struct syntax_node *n) {
	switch(n->type) {
	case SNT_STATEMENT:
		release_syntax_statement((struct syntax_statement *)n);
		break;
	case SNT_FUNCTION:
		release_syntax_function((struct syntax_function *)n);
		break;
	case SNT_REQUIREMENT:
		release_syntax_requirement((struct syntax_requirement *)n);
		break;
	case SNT_DECLARATION:
		release_syntax_declaration((struct syntax_declaration *)n);
		break;
	case SNT_BLOCK:
		release_syntax_block((struct syntax_block *)n);
		break;
	case SNT_EXPRESSION:
		release_syntax_expression((struct syntax_expression *)n);
		break;
	case SNT_VARIABLE:
		release_syntax_comment((struct syntax_comment *)n);
		break;
	case SNT_COMMENT:
		release_syntax_variable((struct syntax_variable *)n);
		break;
	default:
		syntax_node_release_children(n);
		free(n);
		break;
	}
}

struct syntax_statement *create_syntax_statement() {
	struct syntax_statement *stmt = (struct syntax_statement *)malloc(sizeof(struct syntax_statement));
	syntax_node_init(&stmt->n, SNT_STATEMENT);
	return stmt;
}

void release_syntax_statement(struct syntax_statement *stmt) {
	syntax_node_release_children(&stmt->n);
	free(stmt);
}

struct syntax_function *create_syntax_function() {
	struct syntax_function *func = (struct syntax_function *)malloc(sizeof(struct syntax_function));
	syntax_node_init(&func->n, SNT_FUNCTION);
	return func;
}

void release_syntax_function(struct syntax_function *func) {
	syntax_node_release_children(&func->n);
	free(func);
}

struct syntax_requirement *create_syntax_requirement() {
	struct syntax_requirement *req = (struct syntax_requirement *)malloc(sizeof(struct syntax_requirement));
	syntax_node_init(&req->n, SNT_REQUIREMENT);
	return req;
}

void release_syntax_requirement(struct syntax_requirement *req) {
	syntax_node_release_children(&req->n);
	free(req);
}

struct syntax_declaration *create_syntax_declaration() {
	struct syntax_declaration *decl = (struct syntax_declaration *)malloc(sizeof(struct syntax_declaration));
	syntax_node_init(&decl->n, SNT_DECLARATION);
	return decl;
}

void release_syntax_declaration(struct syntax_declaration *decl) {
	syntax_node_release_children(&decl->n);
	free(decl);
}

struct syntax_block *create_syntax_block() {
	struct syntax_block *block = (struct syntax_block *)malloc(sizeof(struct syntax_block));
	syntax_node_init(&block->n, SNT_BLOCK);
	return block;
}

void release_syntax_block(struct syntax_block *block) {
	syntax_node_release_children(&block->n);
	free(block);	
}

struct syntax_expression *create_syntax_expression() {
	struct syntax_expression *expr = (struct syntax_expression *)malloc(sizeof(struct syntax_expression));
	syntax_node_init(&expr->n, SNT_EXPRESSION);
	return expr;
}

void release_syntax_expression(struct syntax_expression *expr) {
	syntax_node_release_children(&expr->n);
	free(expr);	
}

struct syntax_variable *create_syntax_variable(const char *name, struct syntax_expression *expr) {
	struct syntax_variable *var = (struct syntax_variable *)malloc(sizeof(struct syntax_variable));
	syntax_node_init(&var->n, SNT_VARIABLE);
	var->name = strcopy(name);
	syntax_node_add_child(&var->n, &expr->n);
	return var;
}

void release_syntax_variable(struct syntax_variable *var) {
	syntax_node_release_children(&var->n);
	free(var->name);
	free(var);
}

struct syntax_comment *create_syntax_comment(const char *comment) {
	struct syntax_comment *cmt = (struct syntax_comment *)malloc(sizeof(struct syntax_comment));
	syntax_node_init(&cmt->n, SNT_COMMENT);
	cmt->comment = strcopy(comment);
	return cmt;
}

void release_syntax_comment(struct syntax_comment *cmt) {
	syntax_node_release_children(&cmt->n);
	free(cmt->comment);
	free(cmt);
}
