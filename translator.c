#include "fox.h"
#include "symbol.h"
#include "syntax.h"
#include "parser.h"
#include "translator.h"

struct translator {
	struct parser *parser;
	char *filename;
	FILE *fp;
};

static struct translator *translator_create(struct parser *p, const char *filename) {
	FILE *fp = fopen(filename, "wb");
	if(!fp) {
		log_error("open file failed %s", filename);
		return NULL;
	}

	struct translator *t = (struct translator *)malloc(sizeof(struct translator));
	t->parser = p;
	t->filename = strdup(filename);
	t->fp = fp;
	return t;
}

static void translator_release(struct translator *t) {
	if(!t) return;

	fclose(t->fp);
	free(t->filename);
	free(t);
}

static int translate_syntax_node(struct translator *t, struct syntax_node *n);
static int trans_syntax_node_children(struct translator *t, struct syntax_node *n) {
	struct syntax_node *c = n->children;
	while(c) {
		if(!translate_syntax_node(t, c)) return 0;
		c = c->next;
	}
	return 1;
}

static int trans_syntax_program(struct translator *t, struct syntax_program *program) {
	log_info("trans program, depth:%d, children count:%d",
			 syntax_node_depth(&program->n),
			 syntax_node_children_count(&program->n));
	return trans_syntax_node_children(t, &program->n);
}

static int trans_syntax_chunk(struct translator *t, struct syntax_chunk *chunk) {
	log_info("trans chunk, depth:%d, children count:%d",
			 syntax_node_depth(&chunk->n),
			 syntax_node_children_count(&chunk->n));
	return trans_syntax_node_children(t, &chunk->n);
}

static int trans_syntax_requirement(struct translator *t, struct syntax_requirement *req) {
	log_info("trans requirement, depth:%d, children count:%d, name:%s",
			 syntax_node_depth(&req->n),
			 syntax_node_children_count(&req->n),
			 req->name);
	return trans_syntax_node_children(t, &req->n);
}

static int trans_syntax_function(struct translator *t, struct syntax_function *func) {
	log_info("trans function, depth:%d, children count: %d, name:%s",
			 syntax_node_depth(&func->n),
			 syntax_node_children_count(&func->n),
			 func->name);
	return trans_syntax_node_children(t, &func->n);
}

static int trans_syntax_functioncall(struct translator *t, struct syntax_functioncall *fcall) {
	log_info("trans functioncall, depth:%d, children count:%d",
			 syntax_node_depth(&fcall->n),
			 syntax_node_children_count(&fcall->n));
	return trans_syntax_node_children(t, &fcall->n);
}

static int trans_syntax_block(struct translator *t, struct syntax_block *block) {
	log_info("trans block, depth:%d, children count:%d",
			 syntax_node_depth(&block->n),
			 syntax_node_children_count(&block->n));
	return trans_syntax_node_children(t, &block->n);
}

static int trans_syntax_statement(struct translator *t, struct syntax_statement *stmt) {
	log_info("trans statement, depth:%d, children count:%d, tag:%d",
			 syntax_node_depth(&stmt->n),
			 syntax_node_children_count(&stmt->n),
			 stmt->tag);
	return trans_syntax_node_children(t, &stmt->n);
}

static int trans_syntax_expression(struct translator *t, struct syntax_expression *expr) {
	if(expr->tag == EXPR_NUMBER) {
		log_info("trans expression, depth:%d, children count:%d, tag:%d, number:%f",
				 syntax_node_depth(&expr->n),
				 syntax_node_children_count(&expr->n),
				 expr->tag,
				 expr->value.number);
	} else if(expr->tag == EXPR_STRING) {
		log_info("trans expression, depth:%d, children count:%d, tag:%d, string:%s",
				 syntax_node_depth(&expr->n),
				 syntax_node_children_count(&expr->n),
				 expr->tag,
				 expr->value.string);
	} else {
		log_info("trans expression, depth:%d, children count:%d, tag:%d",
				 syntax_node_depth(&expr->n),
				 syntax_node_children_count(&expr->n),
				 expr->tag);
	}
	return trans_syntax_node_children(t, &expr->n);
}

static int trans_syntax_variable(struct translator *t, struct syntax_variable *var) {
	if(var->tag == VAR_NORMAL) {
		log_info("trans variable, depth:%d, children count:%d, tag:%d, name:%s",
				 syntax_node_depth(&var->n),
				 syntax_node_children_count(&var->n),
				 var->tag,
				 var->name);
	} else {
		log_info("trans variable, depth:%d, children count:%d, tag:%d",
				 syntax_node_depth(&var->n),
				 syntax_node_children_count(&var->n),
				 var->tag);
	}
	return trans_syntax_node_children(t, &var->n);
}

static int trans_syntax_argument(struct translator *t, struct syntax_argument *arg) {
	log_info("trans argument, depth:%d, children count:%d, name:%s",
			 syntax_node_depth(&arg->n),
			 syntax_node_children_count(&arg->n),
			 arg->name);
	return trans_syntax_node_children(t, &arg->n);
}

static int trans_syntax_table(struct translator *t, struct syntax_table *st) {
	log_info("trans table, depth:%d, children count:%d",
			 syntax_node_depth(&st->n),
			 syntax_node_children_count(&st->n));
	return trans_syntax_node_children(t, &st->n);
}

static int trans_syntax_field(struct translator *t, struct syntax_field *field) {
	log_info("trans field, depth:%d, children count:%d, name:%s",
			 syntax_node_depth(&field->n),
			 syntax_node_children_count(&field->n),
			 field->name);
	return trans_syntax_node_children(t, &field->n);
}

static int trans_syntax_return(struct translator *t, struct syntax_return *ret) {
	log_info("trans return, depth:%d, children count:%d",
			 syntax_node_depth(&ret->n),
			 syntax_node_children_count(&ret->n));
	return trans_syntax_node_children(t, &ret->n);
}

static int translate_syntax_node(struct translator *t, struct syntax_node *n) {
	int val = 0;
	switch(n->type) {
	case SNT_PROGRAM:
		val = trans_syntax_program(t, (struct syntax_program *)n);
		break;
	case SNT_CHUNK:
		val = trans_syntax_chunk(t, (struct syntax_chunk *)n);
		break;
	case SNT_REQUIREMENT:
		val = trans_syntax_requirement(t, (struct syntax_requirement *)n);
		break;
	case SNT_FUNCTION:
		val = trans_syntax_function(t, (struct syntax_function *)n);
		break;
	case SNT_FUNCTIONCALL:
		val = trans_syntax_functioncall(t, (struct syntax_functioncall *)n);
		break;
	case SNT_BLOCK:
		val = trans_syntax_block(t, (struct syntax_block *)n);
		break;
	case SNT_STATEMENT:
		val = trans_syntax_statement(t, (struct syntax_statement *)n);
		break;
	case SNT_EXPRESSION:
		val = trans_syntax_expression(t, (struct syntax_expression *)n);
		break;
	case SNT_TABLE:
		val = trans_syntax_table(t, (struct syntax_table *)n);
		break;
	case SNT_FIELD:
		val = trans_syntax_field(t, (struct syntax_field *)n);
		break;
	case SNT_VARIABLE:
		val = trans_syntax_variable(t, (struct syntax_variable *)n);
		break;
	case SNT_ARGUMENT:
		val = trans_syntax_argument(t, (struct syntax_argument *)n);
		break;
	case SNT_RETURN:
		val = trans_syntax_return(t, (struct syntax_return *)n);
		break;
	default:
		log_error("unknown syntax node type to translate:%d", n->type);
		break;
	}
	return val;
}

int translate(struct parser *p, const char *filename) {
	if(!p || !p->tree || !p->tree->root || !p->table) {
		log_error("syntax tree is invalid");
		return 0;
	}

	if(p->tree->root->type != SNT_PROGRAM) {
		log_error("syntax tree root is not program");
		return 0;
	}	

	struct translator *t = translator_create(p, filename);
	if(!t) {
		log_error("create translator failed");
		return 0;
	}

	int val = translate_syntax_node(t, p->tree->root);
	
	translator_release(t);
	return val;
}
