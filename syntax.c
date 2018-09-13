#include "fox.h"
#include "syntax.h"

struct syntax_tree *syntax_tree_create() {
	struct syntax_tree *t = malloc(sizeof(struct syntax_tree));
	t->root = NULL;
	return t;
}

void syntax_tree_release(struct syntax_tree *t) {
	if(!t) return;
	if(t->root)	syntax_node_release(t->root);
	free(t);
}

void syntax_tree_walk(struct syntax_tree *t, syntax_node_handler h) {
	if(!t || !t->root) return;
	syntax_node_walk(t->root, h);
}

void syntax_node_init(struct syntax_node *n, int ty) {
	n->next = NULL;
	n->parent = NULL;
	n->children = NULL;
	n->type = ty;
}

void syntax_node_push_child_head(struct syntax_node *p, struct syntax_node *c) {
	struct syntax_node *pc = c;
	struct syntax_node *cc = pc->next;
	c->parent = p;
	while(cc) {
		cc->parent = p;
		pc = cc;
		cc = pc->next;
	}
	pc->next = p->children;
	p->children = c;
}

void syntax_node_push_child_tail(struct syntax_node *p, struct syntax_node *c) {
	struct syntax_node *cc = c;
	while(cc) {
		cc->parent = p;
		cc = cc->next;
	}

	if(!p->children) {
		p->children = c;
		return;
	}

	struct syntax_node *pp = p->children;
	struct syntax_node *pn = pp->next;
	while(pn) {
		pp = pn;
		pn = pn->next;
	}
	pp->next = c;
}

void syntax_node_push_sibling_head(struct syntax_node *p, struct syntax_node *c) {
	struct syntax_node *pc = c;
	struct syntax_node *cc = pc->next;
	while(cc) {
		pc = cc;
		cc = pc->next;
	}
	pc->next = p->next;
	p->next = c;
}

void syntax_node_push_sibling_tail(struct syntax_node *p, struct syntax_node *c) {
	struct syntax_node *pc = p;
	struct syntax_node *cc = pc->next;
	while(cc) {
		pc = cc;
		cc = pc->next;
	}
	pc->next = c;
}

int syntax_node_depth(struct syntax_node *n) {
	size_t d = 0;
	struct syntax_node *p = n->parent;
	while(p) {
		d++;
		p = p->parent;
	}
	return d;
}

int syntax_node_children_count(struct syntax_node *n) {
	size_t d = 0;
	struct syntax_node *p = n->children;
	while(p) {
		d++;
		p = p->next;
	}
	return d;
}

int syntax_node_sibling_count(struct syntax_node *n) {
	size_t d = 0;
	struct syntax_node *p = n->next;
	while(p) {
		d++;
		p = p->next;
	}
	return d;
}

void syntax_node_walk(struct syntax_node *n, syntax_node_handler h) {
	h(n);
	struct syntax_node *c = n->children;
	while(c) {
		h(c);
		c = c->next;
	}
}

static void syntax_node_release_children(struct syntax_node *n) {
	struct syntax_node *c = n->children;
	while(c) {
		struct syntax_node *cc = c;
		c = c->next;
		syntax_node_release(cc);
	}
}

void syntax_node_release(struct syntax_node *n) {
	void *node = n;
	switch(n->type) {
	case SNT_PROGRAM:
		release_syntax_program(node);
		break;
	case SNT_CHUNK:
		release_syntax_chunk(node);
		break;
	case SNT_REQUIREMENT:
		release_syntax_requirement(node);
		break;
	case SNT_FUNCTION:
		release_syntax_function(node);
		break;
	case SNT_FUNCTIONCALL:
		release_syntax_functioncall(node);
		break;
	case SNT_BLOCK:
		release_syntax_block(node);
		break;
	case SNT_STATEMENT:
		release_syntax_statement(node);
		break;
	case SNT_EXPRESSION:
		release_syntax_expression(node);
		break;
	case SNT_TABLE:
		release_syntax_table(node);
		break;
	case SNT_FIELD:
		release_syntax_field(node);
		break;
	case SNT_VARIABLE:
		release_syntax_variable(node);
		break;
	case SNT_ARGUMENT:
		release_syntax_argument(node);
		break;
	case SNT_RETURN:
		release_syntax_return(node);
		break;
	default:
		log_warn("unknown syntax node type to release:%d", n->type);
		syntax_node_release_children(n);
		free(n);
		break;
	}
}

struct syntax_program *create_syntax_program() {
	struct syntax_program *program = malloc(sizeof(struct syntax_program));
	syntax_node_init(&program->n, SNT_PROGRAM);
	return program;
}

void release_syntax_program(struct syntax_program *program) {
	syntax_node_release_children(&program->n);
	free(program);	
}

struct syntax_chunk *create_syntax_chunk() {
	struct syntax_chunk *chunk = malloc(sizeof(struct syntax_chunk));
	syntax_node_init(&chunk->n, SNT_CHUNK);
	return chunk;
}

void release_syntax_chunk(struct syntax_chunk *chunk) {
	syntax_node_release_children(&chunk->n);
	free(chunk);
}

struct syntax_requirement *create_syntax_requirement(const char *name) {
	struct syntax_requirement *req = malloc(sizeof(struct syntax_requirement));
	syntax_node_init(&req->n, SNT_REQUIREMENT);
	req->name = strdup(name);
	return req;
}

void release_syntax_requirement(struct syntax_requirement *req) {
	syntax_node_release_children(&req->n);
	free(req->name);
	free(req);
}

struct syntax_function *create_syntax_function(const char *name) {
	struct syntax_function *func = malloc(sizeof(struct syntax_function));
	syntax_node_init(&func->n, SNT_FUNCTION);
	func->name = strdup(name);
	return func;
}

void release_syntax_function(struct syntax_function *func) {
	syntax_node_release_children(&func->n);
	free(func->name);
	free(func);
}

struct syntax_functioncall *create_syntax_functioncall() {
	struct syntax_functioncall *fcall = malloc(sizeof(struct syntax_functioncall));
	syntax_node_init(&fcall->n, SNT_FUNCTIONCALL);
	return fcall;
}

void release_syntax_functioncall(struct syntax_functioncall *fcall) {
	syntax_node_release_children(&fcall->n);
	free(fcall);
}

struct syntax_block *create_syntax_block() {
	struct syntax_block *block = malloc(sizeof(struct syntax_block));
	syntax_node_init(&block->n, SNT_BLOCK);
	return block;
}

void release_syntax_block(struct syntax_block *block) {
	syntax_node_release_children(&block->n);
	free(block);	
}

struct syntax_statement *create_syntax_statement() {
	struct syntax_statement *stmt = malloc(sizeof(struct syntax_statement));
	syntax_node_init(&stmt->n, SNT_STATEMENT);
	return stmt;
}

void release_syntax_statement(struct syntax_statement *stmt) {
	syntax_node_release_children(&stmt->n);
	free(stmt);
}

struct syntax_expression *create_syntax_expression() {
	struct syntax_expression *expr = malloc(sizeof(struct syntax_expression));
	syntax_node_init(&expr->n, SNT_EXPRESSION);
	return expr;
}

void release_syntax_expression(struct syntax_expression *expr) {
	syntax_node_release_children(&expr->n);
	free(expr);	
}

struct syntax_variable *create_syntax_variable(const char *name) {
	struct syntax_variable *var = malloc(sizeof(struct syntax_variable));
	syntax_node_init(&var->n, SNT_VARIABLE);
	var->name = strdup(name);
	return var;
}

void release_syntax_variable(struct syntax_variable *var) {
	syntax_node_release_children(&var->n);
	free(var->name);
	free(var);
}

struct syntax_argument *create_syntax_argument(const char *name) {
	struct syntax_argument *arg = malloc(sizeof(struct syntax_argument));
	syntax_node_init(&arg->n, SNT_ARGUMENT);
	arg->name = strdup(name);
	return arg;
}

void release_syntax_argument(struct syntax_argument *arg) {
	syntax_node_release_children(&arg->n);
	free(arg->name);
	free(arg);
}

struct syntax_table *create_syntax_table() {
	struct syntax_table *table = malloc(sizeof(struct syntax_table));
	syntax_node_init(&table->n, SNT_TABLE);
	return table;
}

void release_syntax_table(struct syntax_table *table) {
	syntax_node_release_children(&table->n);
	free(table);
}

struct syntax_field *create_syntax_field(const char *name) {
	struct syntax_field *field = malloc(sizeof(struct syntax_field));
	syntax_node_init(&field->n, SNT_FIELD);
	field->name = strdup(name);
	return field;
}

void release_syntax_field(struct syntax_field *field) {
	syntax_node_release_children(&field->n);
	free(field->name);
	free(field);
}

struct syntax_return *create_syntax_return() {
	struct syntax_return *ret = malloc(sizeof(struct syntax_return));
	syntax_node_init(&ret->n, SNT_RETURN);
	return ret;	
}

void release_syntax_return(struct syntax_return *ret) {
	syntax_node_release_children(&ret->n);
	free(ret);	
}
