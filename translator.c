#include "fox.h"
#include "symbol.h"
#include "parser.h"
#include "translator.h"

static int trans_syntax_node_children(FILE *file, struct syntax_node *n, struct symbol_table *t) {
	struct syntax_node *c = n->children;
	while(c) {
		if(!translate_syntax_node(file, c, t)) return 0;
		c = c->next;
	}
	return 1;
}

int trans_syntax_program(FILE *file, struct syntax_program *program, struct symbol_table *table) {
	log_info("trans program, depth:%d, children count:%d",
			 syntax_node_depth(&program->n),
			 syntax_node_children_count(&program->n));
	return trans_syntax_node_children(file, &program->n, table);
}

int trans_syntax_chunk(FILE *file, struct syntax_chunk *chunk, struct symbol_table *table) {
	log_info("trans chunk, depth:%d, children count:%d",
			 syntax_node_depth(&chunk->n),
			 syntax_node_children_count(&chunk->n));
	return trans_syntax_node_children(file, &chunk->n, table);
}

int trans_syntax_requirement(FILE *file, struct syntax_requirement *req, struct symbol_table *table) {
	log_info("trans requirement, depth:%d, children count:%d, name:%s",
			 syntax_node_depth(&req->n),
			 syntax_node_children_count(&req->n),
			 req->name);
	return trans_syntax_node_children(file, &req->n, table);
}

int trans_syntax_function(FILE *file, struct syntax_function *func, struct symbol_table *table) {
	log_info("trans function, depth:%d, children count: %d, name:%s",
			 syntax_node_depth(&func->n),
			 syntax_node_children_count(&func->n),
			 func->name);
	return trans_syntax_node_children(file, &func->n, table);
}

int trans_syntax_functioncall(FILE *file, struct syntax_functioncall *fcall, struct symbol_table *table) {
	log_info("trans functioncall, depth:%d, children count:%d",
			 syntax_node_depth(&fcall->n),
			 syntax_node_children_count(&fcall->n));
	return trans_syntax_node_children(file, &fcall->n, table);
}

int trans_syntax_block(FILE *file, struct syntax_block *block, struct symbol_table *table) {
	log_info("trans block, depth:%d, children count:%d",
			 syntax_node_depth(&block->n),
			 syntax_node_children_count(&block->n));
	return trans_syntax_node_children(file, &block->n, table);
}

int trans_syntax_statement(FILE *file, struct syntax_statement *stmt, struct symbol_table *table) {
	log_info("trans statement, depth:%d, children count:%d, tag:%d",
			 syntax_node_depth(&stmt->n),
			 syntax_node_children_count(&stmt->n),
			 stmt->tag);
	return trans_syntax_node_children(file, &stmt->n, table);
}

int trans_syntax_expression(FILE *file, struct syntax_expression *expr, struct symbol_table *table) {
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
	return trans_syntax_node_children(file, &expr->n, table);
}

int trans_syntax_variable(FILE *file, struct syntax_variable *var, struct symbol_table *table) {
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
	return trans_syntax_node_children(file, &var->n, table);
}

int trans_syntax_argument(FILE *file, struct syntax_argument *arg, struct symbol_table *table) {
	log_info("trans argument, depth:%d, children count:%d, name:%s",
			 syntax_node_depth(&arg->n),
			 syntax_node_children_count(&arg->n),
			 arg->name);
	return trans_syntax_node_children(file, &arg->n, table);
}

int trans_syntax_table(FILE *file, struct syntax_table *st, struct symbol_table *table) {
	log_info("trans table, depth:%d, children count:%d",
			 syntax_node_depth(&st->n),
			 syntax_node_children_count(&st->n));
	return trans_syntax_node_children(file, &st->n, table);
}

int trans_syntax_field(FILE *file, struct syntax_field *field, struct symbol_table *table) {
	log_info("trans field, depth:%d, children count:%d, name:%s",
			 syntax_node_depth(&field->n),
			 syntax_node_children_count(&field->n),
			 field->name);
	return trans_syntax_node_children(file, &field->n, table);
}

int trans_syntax_return(FILE *file, struct syntax_return *ret, struct symbol_table *table) {
	log_info("trans return, depth:%d, children count:%d",
			 syntax_node_depth(&ret->n),
			 syntax_node_children_count(&ret->n));
	return trans_syntax_node_children(file, &ret->n, table);
}

int translate_syntax_node(FILE *file, struct syntax_node *n, struct symbol_table *t) {
	int val = 0;
	switch(n->type) {
	case SNT_PROGRAM:
		val = trans_syntax_program(file, (struct syntax_program *)n, t);
		break;
	case SNT_CHUNK:
		val = trans_syntax_chunk(file, (struct syntax_chunk *)n, t);
		break;
	case SNT_REQUIREMENT:
		val = trans_syntax_requirement(file, (struct syntax_requirement *)n, t);
		break;
	case SNT_FUNCTION:
		val = trans_syntax_function(file, (struct syntax_function *)n, t);
		break;
	case SNT_FUNCTIONCALL:
		val = trans_syntax_functioncall(file, (struct syntax_functioncall *)n, t);
		break;
	case SNT_BLOCK:
		val = trans_syntax_block(file, (struct syntax_block *)n, t);
		break;
	case SNT_STATEMENT:
		val = trans_syntax_statement(file, (struct syntax_statement *)n, t);
		break;
	case SNT_EXPRESSION:
		val = trans_syntax_expression(file, (struct syntax_expression *)n, t);
		break;
	case SNT_TABLE:
		val = trans_syntax_table(file, (struct syntax_table *)n, t);
		break;
	case SNT_FIELD:
		val = trans_syntax_field(file, (struct syntax_field *)n, t);
		break;
	case SNT_VARIABLE:
		val = trans_syntax_variable(file, (struct syntax_variable *)n, t);
		break;
	case SNT_ARGUMENT:
		val = trans_syntax_argument(file, (struct syntax_argument *)n, t);
		break;
	case SNT_RETURN:
		val = trans_syntax_return(file, (struct syntax_return *)n, t);
		break;
	default:
		log_error("unknown syntax node type to translate:%d", n->type);
		break;
	}
	return val;
}

int translate(const char *filename, struct syntax_tree *tree, struct symbol_table *table) {
	if(!tree || !tree->root) {
		log_error("syntax tree is invalid");
		return 0;
	}

	FILE *fp = fopen(filename, "wb");
	if(!fp) {
		log_error("open file failed %s", filename);
		return 0;
	}

	if(tree->root->type != SNT_PROGRAM) {
		log_error("syntax tree root is not program");
		return 0;
	}

	int val = translate_syntax_node(fp, tree->root, table);
	
	fclose(fp);
	return val;
}
