#include "fox.h"
#include "symbol.h"
#include "parser.h"
#include "translator.h"

int trans_syntax_node_children(FILE *file, struct syntax_node *n) {
	struct syntax_node *c = n->children;
	while(c) {
		if(!translate_syntax_node(file, c)) return 0;
		c = c->next;
	}
	return 1;
}

int trans_syntax_program(FILE *file, struct syntax_program *program) {
	log_info("trans program");
	return trans_syntax_node_children(file, (struct syntax_node *)program);
}

int trans_syntax_chunk(FILE *file, struct syntax_chunk *chunk) {
	log_info("trans chunk");
	return trans_syntax_node_children(file, (struct syntax_node *)chunk);
}

int trans_syntax_requirement(FILE *file, struct syntax_requirement *req) {
	log_info("trans requirement, name:%s", req->name);
	return trans_syntax_node_children(file, (struct syntax_node *)req);
}

int trans_syntax_function(FILE *file, struct syntax_function *func) {
	log_info("trans function, name:%s", func->name);
	return trans_syntax_node_children(file, (struct syntax_node *)func);
}

int trans_syntax_functioncall(FILE *file, struct syntax_functioncall *fcall) {
	log_info("trans functioncall");
	return trans_syntax_node_children(file, (struct syntax_node *)fcall);
}

int trans_syntax_block(FILE *file, struct syntax_block *block) {
	log_info("trans block");
	return trans_syntax_node_children(file, (struct syntax_node *)block);
}

int trans_syntax_statement(FILE *file, struct syntax_statement *stmt) {
	log_info("trans statement, tag:%d", stmt->tag);
	return trans_syntax_node_children(file, (struct syntax_node *)stmt);
}

int trans_syntax_expression(FILE *file, struct syntax_expression *expr) {
	if(expr->tag == EXPR_NUMBER) {
		log_info("trans expression, tag:%d, number:%f", expr->tag, expr->value.number);
	} else if(expr->tag == EXPR_STRING) {
		log_info("trans expression, tag:%d, string:%s", expr->tag, expr->value.string);
	} else {
		log_info("trans expression, tag:%d", expr->tag);
	}
	return trans_syntax_node_children(file, (struct syntax_node *)expr);
}

int trans_syntax_variable(FILE *file, struct syntax_variable *var) {
	if(var->tag == VAR_NORMAL) {
		log_info("trans variable, tag:%d, name:%s", var->tag, var->name);
	} else {
		log_info("trans variable, tag:%d", var->tag);
	}
	return trans_syntax_node_children(file, (struct syntax_node *)var);
}

int trans_syntax_argument(FILE *file, struct syntax_argument *arg) {
	log_info("trans argument, name:%s", arg->name);
	return trans_syntax_node_children(file, (struct syntax_node *)arg);
}

int trans_syntax_table(FILE *file, struct syntax_table *table) {
	log_info("trans table");
	return trans_syntax_node_children(file, (struct syntax_node *)table);
}

int trans_syntax_field(FILE *file, struct syntax_field *field) {
	log_info("trans field, name:%s", field->name);
	return trans_syntax_node_children(file, (struct syntax_node *)field);
}

int trans_syntax_return(FILE *file, struct syntax_return *ret) {
	log_info("trans return");
	return trans_syntax_node_children(file, (struct syntax_node *)ret);
}

int translate_syntax_node(FILE *file, struct syntax_node *n) {
	int val = 0;
	switch(n->type) {
	case SNT_PROGRAM:
		val = trans_syntax_program(file, (struct syntax_program *)n);
		break;
	case SNT_CHUNK:
		val = trans_syntax_chunk(file, (struct syntax_chunk *)n);
		break;
	case SNT_REQUIREMENT:
		val = trans_syntax_requirement(file, (struct syntax_requirement *)n);
		break;
	case SNT_FUNCTION:
		val = trans_syntax_function(file, (struct syntax_function *)n);
		break;
	case SNT_FUNCTIONCALL:
		val = trans_syntax_functioncall(file, (struct syntax_functioncall *)n);
		break;
	case SNT_BLOCK:
		val = trans_syntax_block(file, (struct syntax_block *)n);
		break;
	case SNT_STATEMENT:
		val = trans_syntax_statement(file, (struct syntax_statement *)n);		
		break;
	case SNT_EXPRESSION:
		val = trans_syntax_expression(file, (struct syntax_expression *)n);		
		break;
	case SNT_TABLE:
		val = trans_syntax_table(file, (struct syntax_table *)n);		
		break;
	case SNT_FIELD:
		val = trans_syntax_field(file, (struct syntax_field *)n);		
		break;
	case SNT_VARIABLE:
		val = trans_syntax_variable(file, (struct syntax_variable *)n);
		break;
	case SNT_ARGUMENT:
		val = trans_syntax_argument(file, (struct syntax_argument *)n);		
		break;
	case SNT_RETURN:
		val = trans_syntax_return(file, (struct syntax_return *)n);
		break;
	default:
		log_error("unknown syntax node type to translate:%d", n->type);
		break;
	}
	return val;
}

int translate(const char *filename, struct syntax_tree *tree) {
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

	int val = translate_syntax_node(fp, tree->root);
	
	fclose(fp);
	return val;
}
