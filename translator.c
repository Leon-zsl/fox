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

int parse(const char *filename,
		  struct syntax_tree **tree,
		  struct symbol_table **table) {
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
	int val = yyparse();
	fclose(fp);

	if(val) {
		log_error("parse lua program failed: %s", filename);
		syntax_tree_release(parse_tree);
		symbol_table_release(parse_table);
		return 0;
	}
	
	log_info("parse lua program succeed: %s", filename);
	*tree = parse_tree;
	*table = parse_table;
	return 1;
}

struct translator {
	struct syntax_tree *tree;
	struct symbol_table *table;
	FILE *fp;
	bool retstmt;
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
	t->retstmt = FALSE;
	return t;
}

static void translator_release(struct translator *t) {
	if(!t) return;

	fclose(t->fp);
	free(t);
}

static int translate_syntax_node(struct translator *t, struct syntax_node *n);

int translate(const char *filename,
			  struct syntax_tree *tree,
			  struct symbol_table *table) {
	if(!tree || !tree->root || !table) {
		log_error("syntax tree or symbol table is invalid");
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
	if(!val) {
		log_error("translate lua program failed:%s", filename);
		return 0;
	}

	log_info("translate lua program succeed:%s", filename);
	return 1;
}

static int trans_syntax_chunk(struct translator *t, struct syntax_node *n);
static int trans_syntax_block(struct translator *t, struct syntax_node *);
static int trans_syntax_statement(struct translator *t, struct syntax_node *n);
static int trans_syntax_expression(struct translator *t, struct syntax_node *n);
static int trans_syntax_variable(struct translator *t, struct syntax_node *n);
static int trans_syntax_function(struct translator *t, struct syntax_node *n);
static int trans_syntax_functioncall(struct translator *t, struct syntax_node *n);
static int trans_syntax_argument(struct translator *t, struct syntax_node *n);
static int trans_syntax_table(struct translator *t, struct syntax_node *n);
static int trans_syntax_field(struct translator *t, struct syntax_node *n);

static int func_is_method(const char *funcname) {
	return strstr(funcname, ":") != NULL;
}

static struct translator *translator = NULL;
static void exports_handler(const char *name, struct symbol *s) {
	if(translator) {
		fprintf(translator->fp, name);
		fprintf(translator->fp, ":");
		fprintf(translator->fp, name);
		fprintf(translator->fp, ",\n");
	}
}

static int trans_syntax_node_children(struct translator *t, struct syntax_node *n) {
	struct syntax_node *c = n->children;
	while(c) {
		if(!translate_syntax_node(t, c)) return 0;
		c = c->next;
	}
	return 1;
}

static int trans_syntax_chunk(struct translator *t, struct syntax_node *n) {
	int val = trans_syntax_node_children(t, n);
	if(!val) return 0;

	if(!t->retstmt) {
		fprintf(t->fp, "\n\nmodule.exports = {\n");
		translator = t;
		symbol_table_walk(t->table, exports_handler);
		fseek(t->fp, -2, SEEK_END);
		//fprintf(t->fp, "  "); //eat the last ,\n
		translator = NULL;
		fprintf(t->fp, "\n}\n");
	}
	return 1;
}

static int trans_syntax_block(struct translator *t, struct syntax_node *n) {
	if(n->parent->type != STX_CHUNK) fprintf(t->fp, "\n{\n");
	int val = trans_syntax_node_children(t, n);
	if(n->parent->type != STX_CHUNK) fprintf(t->fp, "\n}\n");
	return val;
}

static int trans_local_assign(struct translator *t, struct syntax_statement *stmt) {
	if(stmt->tag != STMT_LOCAL_VAR) {
		log_error("trans local assign with illeagal stmt %d:%s",
				  stmt->n.lineno,
				  syntax_statement_tag_string(stmt->tag));
		return 0;
	}
	
	int ncnt = 1;
	char *p = stmt->value.name;
	while(*p != '\0') {
		if(*p++ == ',') ncnt++;
	}

	int ecnt = syntax_node_children_count(&stmt->n);
	if(!ecnt) {
		fprintf(t->fp, "let %s\n", stmt->value.name);
		return 1;
	}

	if(ncnt == ecnt) {
		if(ncnt == 1) {
			fprintf(t->fp, "let %s = ", stmt->value.name);
			int val = trans_syntax_expression(t, stmt->n.children);
			if(!val) return 0;

			fprintf(t->fp, "\n");
			return 1;
		} else {
			fprintf(t->fp, "let %s = ", stmt->value.name);
			char *p = stmt->value.name;
			struct syntax_node *c = stmt->n.children;
			while(c) {
				while(*p != '\0' && *p != ',') {
					fputc(*p, t->fp);
					p++;
				}
				p++;
			
				fprintf(t->fp, " = ");
				int val = trans_syntax_expression(t, c);
				if(!val) return 0;
				fprintf(t->fp, "\n");

				c = c->next;
			}
		}
	} else if(ecnt == 1) {
		fprintf(t->fp, "let %s = ", stmt->value.name);
		int val = trans_syntax_expression(t, stmt->n.children);
		if(!val) return 0;

		fprintf(t->fp, "\n");
		return 1;			
	} else {
		log_error("assign count mismatch %d:%d %d", stmt->n.lineno, ncnt, ecnt);
		return 0;			
	}
	return 1;	
}

static int trans_assign(struct translator *t, struct syntax_statement *stmt) {
	if(stmt->tag != STMT_VAR) {
		log_error("trans assign with illeagal stmt %d:%s",
				  stmt->n.lineno,
				  syntax_statement_tag_string(stmt->tag));
		return 0;
	}

	struct syntax_node *c = stmt->n.children;
	int ncnt = 0;
	struct syntax_node *nc = c;
	while(c) {
		if(c->type == STX_VARIABLE) {
			ncnt++;
		} else {
			break;
		}
		c = c->next;
	}
	int ecnt = 0;
	struct syntax_node *ec = c;
	while(c) {
		if(c->type == STX_EXPRESSION) {
			ecnt++;
		} else {
			break;
		}
		c = c->next;
	}
	if(!ecnt) {
		log_error("assign with no right value %d", stmt->n.lineno);
		return 0;
	}

	if(ncnt == ecnt) {
		while(nc && nc->type == STX_VARIABLE) {
			int val = trans_syntax_variable(t, nc);
			if(!val) return 0;
			fprintf(t->fp, " = ");
			
			val = trans_syntax_expression(t, ec);
			if(!val) return 0;
			fprintf(t->fp, "\n");
			
			nc = nc->next;
			ec = ec->next;
		}
	} else if(ecnt == 1) {
		fprintf(t->fp, "{");
		while(nc && nc->type == STX_VARIABLE) {
			int val = trans_syntax_variable(t, nc);
			if(!val) return 0;
			if(nc->next && nc->next->type == STX_VARIABLE) {
				fprintf(t->fp, ", ");				
			}
			nc = nc->next;
		}
		fprintf(t->fp, "}");
		fprintf(t->fp, " = ");
		int val = trans_syntax_expression(t, ec);
		if(!val) return 0;
		fprintf(t->fp, "\n");
		return 1;
	} else {
		log_error("assign count mismatch %d:%d %d", stmt->n.lineno, ncnt, ecnt);
		return 0;			
	}
	return 1;	
}

static int trans_syntax_statement(struct translator *t, struct syntax_node *n) {
	struct syntax_statement *stmt = (struct syntax_statement *)n;
	log_info("trans statement %d:%s", n->lineno, syntax_statement_tag_string(stmt->tag));
	
	switch(stmt->tag) {
	case STMT_EMPTY:
		return 1;
	case STMT_LABEL:
	case STMT_GOTO:
	{
		log_error("unsupport stmt %d:%s", n->lineno, syntax_statement_tag_string(stmt->tag));
		return 0;
	}
	case STMT_BREAK:
	{
		fprintf(t->fp, "break\n");
		return 1;
	}
	case STMT_RETURN:
	{
		struct syntax_node *p = n->parent->parent;
		//module return
		if(p->type == STX_CHUNK) {
			fprintf(t->fp, "\n\nmodule.exports = ");
			t->retstmt = TRUE;
		} else {
			fprintf(t->fp, "return ");
		}
		int val = trans_syntax_node_children(t, n);
		fprintf(t->fp, "\n");
		return val;
	}
	case STMT_DO:
	{
		return trans_syntax_block(t, n->children);
	}
	case STMT_WHILE:
	{
		fprintf(t->fp, "while (");
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;
		fprintf(t->fp, ")");
		return trans_syntax_block(t, n->children->next);
	}
	case STMT_REPEAT:
	{
		fprintf(t->fp, "do ");
		int val = trans_syntax_block(t, n->children);
		if(!val) return 0;
		fprintf(t->fp, "while (");
		val = trans_syntax_expression(t, n->children->next);
		if(!val) return 0;
		fprintf(t->fp, ")\n");
		return 1;
	}
	case STMT_FOR_IN:
	{
		//todo:
		return 1;
	}
	case STMT_FOR_IT:
	{
		//todo:
		return 1;
	}
	case STMT_IF:
	{
		fprintf(t->fp, "if(");
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;
		fprintf(t->fp, ")");
		val = trans_syntax_block(t, n->children->next);
		if(!val) return 0;
		val = trans_syntax_statement(t, n->children->next->next);
		if(!val) return 0;
		
		return 1;
	}
	case STMT_ELSE:
	{
		fprintf(t->fp, "else");
		return trans_syntax_block(t, n->children);
	}
	case STMT_ELSEIF:
	{
		fprintf(t->fp, "else if(");
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;
		fprintf(t->fp, ")");
		val = trans_syntax_block(t, n->children->next);
		if(!val) return 0;
		val = trans_syntax_statement(t, n->children->next->next);
		if(!val) return 0;

		return 1;
	}
	case STMT_VAR:
	{
		struct syntax_node *p = n->parent->parent;
		if(p->type == STX_CHUNK) {
			struct syntax_node *c = n->children;
			while(c && c->type == STX_VARIABLE) {
				struct syntax_variable *v = (struct syntax_variable *)c;
				if(v->tag == VAR_NORMAL) {
					struct symbol *s = symbol_create(v->name, c);
					symbol_table_insert(t->table, s);
				}
				c = c->next;
			}
		}
		return trans_assign(t, stmt);
	}
	case STMT_LOCAL_VAR:
	{
		return trans_local_assign(t, stmt);
	}
	case STMT_FUNC:
	{
		struct syntax_node *p = n->parent->parent;
		if(p->type == STX_CHUNK) {
			struct syntax_function *func = (struct syntax_function *)n->children;
			struct symbol *s = symbol_create(func->name, &func->n);
			symbol_table_insert(t->table, s);
		}
		return trans_syntax_function(t, n->children);
	}
	case STMT_LOCAL_FUNC:
	{
		return trans_syntax_function(t, n->children);
	}
	case STMT_FCALL:
	{
		int val = trans_syntax_functioncall(t, n->children);
		fprintf(t->fp, "\n");
		return val;
	}
	case STMT_INVALID:
	default:
		log_error("illeagal stmt %d:%d", n->lineno, stmt->tag);
		return 0;
	}

	fflush(t->fp);
}

static int trans_syntax_expression(struct translator *t, struct syntax_node *n) {
	struct syntax_expression * exp = (struct syntax_expression *)n;
	if(exp->tag == EXP_NUMBER) {
		log_info("trans expression %d:%s, number:%f",
				 n->lineno,
				 syntax_expression_tag_string(exp->tag),
				 exp->value.number);
	} else if(exp->tag == EXP_STRING) {
		log_info("trans expression %d:%s, string:%s",
				 n->lineno,
				 syntax_expression_tag_string(exp->tag),
				 exp->value.string);
	} else {
		log_info("trans expression %d:%s", n->lineno, syntax_expression_tag_string(exp->tag));
	}

	return trans_syntax_node_children(t, n);
}

static int trans_syntax_variable(struct translator *t, struct syntax_node *n) {
	struct syntax_variable *var = (struct syntax_variable *)n;
	log_info("trans variable %d: %d, %s, name:%s",
			 n->lineno,
			 var->tag,
			 syntax_variable_tag_string(var->tag),
			 var->name ? var->name : "");
	switch(var->tag) {
	case VAR_NORMAL:
		fprintf(t->fp, var->name);
		return 1;
	case VAR_KEY:
	{
		int val = trans_syntax_node_children(t, n);
		if(!val) return 0;
		fprintf(t->fp, ".");
		fprintf(t->fp, var->name);
		return 1;
	}
	case VAR_INDEX:
	{
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;

		fprintf(t->fp, "[");
		
		val = trans_syntax_expression(t, n->children->next);
		if(!val) return 0;

		fprintf(t->fp, "]");
		return 1;
	}
	default:
		log_error("unknown variable tag %d: %d, %s",
				  n->lineno,
				  var->tag,
				  syntax_variable_tag_string(var->tag));
		return 0;
	}
}

static int trans_syntax_function(struct translator *t, struct syntax_node *n) {
	struct syntax_function *func = (struct syntax_function *)n;
	log_info("trans function %d, name:%s", n->lineno, func->name ? func->name : "");
	
	fprintf(t->fp, "function ");
	char *p = func->name;
	while(p && *p != '\0') {
		if(*p == ':') {
			fputc('.', t->fp);
		} else {
			fputc(*p, t->fp);
		}
		p++;
	}
	
	fprintf(t->fp, "(");
	if(func_is_method(func->name)) {
		if(func->pars) {
			fprintf(t->fp, "self,");
		} else {
			fprintf(t->fp, "self");
		}
	} else {
		if(func->pars) {
			if(strstr(func->pars, "...")) {
				log_error("unsupported variable parameters %d, func:%s, pars:%s",
						  n->lineno,
						  func->name ? func->name : "",
						  func->pars);
				return 0;
			}
			fprintf(t->fp, func->pars);
		}
	}
	fprintf(t->fp, ")");

	return trans_syntax_block(t, n->children);
}

static int trans_syntax_functioncall(struct translator *t, struct syntax_node *n) {
	struct syntax_functioncall *fcall = (struct syntax_functioncall *)n;
	int val = trans_syntax_expression(t, n->children);
	if(!val) return 0;

	struct syntax_argument *arg = (struct syntax_argument *)n->children->next;
	
	fprintf(t->fp, "(");
	//means method call
	if(fcall->name) {
		if(arg->tag == ARG_EMPTY)
			fprintf(t->fp, "self");
		else
			fprintf(t->fp, "self,");
	}

	val = trans_syntax_argument(t, &arg->n);
	if(!val) return 0;
	
	fprintf(t->fp, ")");
	return val;
}

static int trans_syntax_argument(struct translator *t, struct syntax_node *n) {
	struct syntax_argument *arg = (struct syntax_argument *)n;
	switch(arg->tag) {
	case ARG_EMPTY:
		return 1;
	case ARG_NORMAL:
	{
		struct syntax_node *c = n->children;
		while(c) {
			int val = trans_syntax_expression(t, c);
			if(!val) return 0;
			if(c->next) {
				fprintf(t->fp, ",");
			}
			c = c->next;
		}
	}
	case ARG_TABLE:
		return trans_syntax_node_children(t, n);
	case ARG_STRING:
		fprintf(t->fp, arg->name);
		return 1;
	default:
		log_error("unknown argument %d:%s", n->lineno, syntax_field_tag_string(arg->tag));
		return 0;		
	}
}

static int trans_syntax_table(struct translator *t, struct syntax_node *n) {
	if(!n->children) {
		fprintf(t->fp, "{}");
		return 1;
	}

	struct syntax_field *f = (struct syntax_field *)n->children;
	int tag = FIELD_INVALID;
	while(f) {
		if(tag == FIELD_INVALID) {
			tag = f->tag;
		}
		if(tag != f->tag) {
			log_error("unsupport composite table %d", n->lineno);
			return 0;
		}
	}

	if(tag != FIELD_KEY && tag != FIELD_SINGLE) {
		log_error("unsupport composite table %d", n->lineno);
		return 0;
	}

	if(tag == FIELD_KEY) {
		fprintf(t->fp, "{");		
	} else if(tag == FIELD_SINGLE) {
		fprintf(t->fp, "[");
	}

	struct syntax_node *c = n->children;
	while(c) {
		int val = trans_syntax_field(t, c);
		if(!val) return 0;
		if(c->next) {
			fprintf(t->fp, ",");
		}

		c = c->next;
	}

	if(tag == FIELD_KEY) {
		fprintf(t->fp, "}");
	} else if(tag == FIELD_SINGLE) {
		fprintf(t->fp, "]");
	}
	return 1;
}

static int trans_syntax_field(struct translator *t, struct syntax_node *n) {
	struct syntax_field *field = (struct syntax_field *)n;
	log_info("trans field, %d:%s, name:%s",
			 n->lineno,
			 syntax_field_tag_string(field->tag),
			 field->name ? field->name : "");
	switch(field->tag) {
	case FIELD_INDEX:
	{
		log_error("unsupport field %d:%s", n->lineno, syntax_field_tag_string(field->tag));
	}
	case FIELD_KEY:
	{
		fprintf(t->fp, field->name);
		fprintf(t->fp, ": ");
		return trans_syntax_node_children(t, n);
	}
	case FIELD_SINGLE:
		return trans_syntax_node_children(t, n);
	default:
		log_error("unknown field %d:%s", n->lineno, syntax_field_tag_string(field->tag));
		return 0;
	}
}

static int translate_syntax_node(struct translator *t, struct syntax_node *n) {
	log_info("trans node line:%d, type:%s, depth:%d, children:%d",
			 n->lineno,
			 syntax_node_type_string(n->type),
			 syntax_node_depth(n),
			 syntax_node_children_count(n));

	int val = 0;
	switch(n->type) {
	case STX_CHUNK:
		val = trans_syntax_chunk(t, n);
		break;
	case STX_BLOCK:
		val = trans_syntax_block(t, n);
		break;
	case STX_STATEMENT:
		val = trans_syntax_statement(t, n);
		break;
	case STX_EXPRESSION:
		val = trans_syntax_expression(t, n);
		break;
	case STX_VARIABLE:
		val = trans_syntax_variable(t, n);
		break;
	case STX_FUNCTION:
		val = trans_syntax_function(t, n);
		break;
	case STX_FUNCTIONCALL:
		val = trans_syntax_functioncall(t, n);
		break;
	case STX_ARGUMENT:
		val = trans_syntax_argument(t, n);
		break;
	case STX_TABLE:
		val = trans_syntax_table(t, n);
		break;
	case STX_FIELD:
		val = trans_syntax_field(t, n);
		break;
	default:
		log_error("unknown syntax node type to translate %d:%d", n->lineno, n->type);
		break;
	}
	return val;
}

