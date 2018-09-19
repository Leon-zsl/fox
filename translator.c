#include "fox.h"
#include "symbol.h"
#include "syntax.h"
#include "translator.h"

int yylex(void);
int yyparse(void);

void yyset_in(FILE *in);
void yyset_out(FILE *out);
void yyset_lineno(int lineno);
void yyset_filename(const char *name);

extern struct syntax_tree *parse_tree;
extern struct symbol_table *parse_table;

int parse(const char *filename, struct syntax_tree **tree, struct symbol_table **table) {
	FILE *fp = fopen(filename, "rb");
	if(!fp) {
		log_error("open file failed %s", filename);
		return 0;
	}

	yyset_in(fp);
	yyset_out(stdout);
	yyset_filename(filename);
	yyset_lineno(1);

	parse_tree = syntax_tree_create();
	parse_table = symbol_table_create();	
	if(yyparse()) {
		log_error("yyparse failed:%s", filename);
		syntax_tree_release(parse_tree);
		symbol_table_release(parse_table);
		fclose(fp);
		return 0;
	}

	fclose(fp);
	*tree = parse_tree;
	*table = parse_table;
	return 1;
}

struct translator {
	struct syntax_tree *tree;
	struct symbol_table *table;
	FILE *fp;
};

static struct translator *translator_create(struct syntax_tree *tree,
											struct symbol_table *table,
											const char *filename) {
	FILE *fp = fopen(filename, "wb");
	if(!fp) {
		log_error("open file failed %s", filename);
		return NULL;
	}

	struct translator *t = malloc(sizeof(struct translator));
	t->tree = tree;
	t->table = table;
	t->fp = fp;
	return t;
}

static void translator_release(struct translator *t) {
	if(!t) return;

	fclose(t->fp);
	free(t);
}

static int translate_syntax_node(struct translator *t, struct syntax_node *n);

int translate(struct syntax_tree *tree, struct symbol_table *table, const char *filename) {
	if(!tree || !tree->root || !table) {
		log_error("syntax tree or symbol table is invalid");
		return 0;
	}

	if(tree->root->type != SNT_CHUNK) {
		log_error("syntax tree root is not chunk");
		return 0;
	}	

	struct translator *t = translator_create(tree, table, filename);
	if(!t) {
		log_error("create translator failed");
		return 0;
	}

	fprintf(t->fp, "//CODE GENERATED BY FOX, A LUA->JS TRANSLATOR!\n\n");
	fflush(t->fp);
	int val = translate_syntax_node(t, tree->root);
	
	translator_release(t);
	return val;
}

static int trans_syntax_node_children(struct translator *t, struct syntax_node *n) {
	struct syntax_node *c = n->children;
	while(c) {
		if(!translate_syntax_node(t, c)) return 0;
		c = c->next;
	}
	return 1;
}

static int trans_syntax_chunk(struct translator *t, struct syntax_chunk *chunk) {
	log_info("trans chunk, depth:%d, children count:%d",
			 syntax_node_depth(&chunk->n),
			 syntax_node_children_count(&chunk->n));
	return trans_syntax_node_children(t, &chunk->n);
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


static int trans_syntax_expression(struct translator *t, struct syntax_expression *expr) {
	if(expr->tag == EXP_NUMBER) {
		log_info("trans expression, depth:%d, children count:%d, tag:%d, number:%f",
				 syntax_node_depth(&expr->n),
				 syntax_node_children_count(&expr->n),
				 expr->tag,
				 expr->value.number);
	} else if(expr->tag == EXP_STRING) {
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
	log_info("trans argument, depth:%d, children count:%d",
			 syntax_node_depth(&arg->n),
			 syntax_node_children_count(&arg->n));
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

static int translate_syntax_node(struct translator *t, struct syntax_node *n) {
	int val = 0;
	void *node = n;
	switch(n->type) {
	case SNT_CHUNK:
		val = trans_syntax_chunk(t, node);
		break;
	case SNT_BLOCK:
		val = trans_syntax_block(t, node);
		break;
	case SNT_STATEMENT:
		val = trans_syntax_statement(t, node);
		break;
	case SNT_EXPRESSION:
		val = trans_syntax_expression(t, node);
		break;
	case SNT_VARIABLE:
		val = trans_syntax_variable(t, node);
		break;
	case SNT_FUNCTION:
		val = trans_syntax_function(t, node);
		break;
	case SNT_FUNCTIONCALL:
		val = trans_syntax_functioncall(t, node);
		break;
	case SNT_ARGUMENT:
		val = trans_syntax_argument(t, node);
		break;
	case SNT_TABLE:
		val = trans_syntax_table(t, node);
		break;
	case SNT_FIELD:
		val = trans_syntax_field(t, node);
		break;
	default:
		log_error("unknown syntax node type to translate:%d", n->type);
		break;
	}
	return val;
}

