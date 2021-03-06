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
void yyset_debug(int debug);

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
#ifdef DEBUG
	yyset_debug(1);
#else
	yyset_debug(0);
#endif

	log_info("parse lua program start: %s", filename);
	
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
	bool exp_symtab;
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
	t->exp_symtab = FALSE;
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
	
	log_info("translate lua program start: %s", filename);

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
	return funcname && strstr(funcname, ":");
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
	log_debug("trans chunk %d", n->lineno);
	
	int val = trans_syntax_block(t, n->children);
	if(!val) return 0;

	if(t->exp_symtab) {
		struct syntax_block *block = (struct syntax_block *)n->children;
		fprintf(t->fp, "\n\nmodule.exports = {\n  ");
		translator = t;
		symbol_table_walk(block->symtab, exports_handler);
		fseek(t->fp, -2, SEEK_END);
		translator = NULL;
		fprintf(t->fp, "\n}\n");
	}
	return 1;
}

static void log_block_symbols(const char *name, struct symbol *s) {
	log_debug("block symbols %d:%s", ((struct syntax_node *)s->udata)->lineno, name);
}

static int trans_syntax_block(struct translator *t, struct syntax_node *n) {
	log_debug("trans block %d", n->lineno);
	struct syntax_block *block = (struct syntax_block *)n;
	symbol_table_walk(block->symtab, log_block_symbols);

	if(n->parent->type != STX_CHUNK) fprintf(t->fp, " {\n");
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
	log_debug("trans statement %d:%s",
			  n->lineno,
			  syntax_statement_tag_string(stmt->tag));
	
	switch(stmt->tag) {
	case STMT_EMPTY:
		return 1;
	case STMT_LABEL:
	{
		fprintf(t->fp, "%s:\n", stmt->value.name);
		return 1;
	}
	case STMT_GOTO:
	{
		//we support continue label not break label
		fprintf(t->fp, "continue %s", stmt->value.name);
		return 1;
	}
	case STMT_BREAK:
	{

		fprintf(t->fp, "break");
		return 1;
	}
	case STMT_RETURN:
	{
		if(!n->children) {
			if(chunk_scope(n)) {
				return 1;
			} else {
				fprintf(t->fp, "return");
				return 1;
			}
		}
		
		if(chunk_scope(n)) {
			fprintf(t->fp, "\n\nmodule.exports = ");
			t->exp_symtab = FALSE;
		} else {
			fprintf(t->fp, "return ");
		}

		if(syntax_node_children_count(n) > 1) {
			fprintf(t->fp, "[");
		}
		struct syntax_node *c = n->children;
		while(c) {
			int val = trans_syntax_expression(t, c);
			if(!val) return 0;
			if(c->next) fprintf(t->fp, ",");
			c = c->next;
		}
		if(syntax_node_children_count(n) > 1) {
			fprintf(t->fp, "]");
		}
		return 1;
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
		fprintf(t->fp, "\n{\n");

		bool needvtmp = TRUE;
		struct syntax_node *e = n->children;
		if(((struct syntax_expression *)e)->tag == EXP_FCALL && syntax_node_sibling_count(e) == 1) {
			fprintf(t->fp, "let retvals = ");
			int val = trans_syntax_expression(t, e);
			if(!val) return 0;
			fprintf(t->fp, "\n");
			
			fprintf(t->fp, "let ftmp = retvals[0]\n");
			fprintf(t->fp, "let stmp = retvals[1]\n");
			fprintf(t->fp, "let vtmp = retvals[2]\n");
		} else {
			fprintf(t->fp, "let ftmp = ");
			int val = trans_syntax_expression(t, e);
			if(!val) return 0;
			fprintf(t->fp, "\n");

			e = e->next;
			if(!e || e->type != STX_EXPRESSION) {
				log_error("missing exp for in stmt %d:%s",
						  n->lineno,
						  syntax_expression_tag_string(stmt->tag));
				return 0;
			}
			fprintf(t->fp, "let stmp = ");
			val = trans_syntax_expression(t, e);
			if(!val) return 0;
			fprintf(t->fp, "\n");

			e = e->next;
			if(e && e->type == STX_EXPRESSION) {
				fprintf(t->fp, "let vtmp = ");
				val = trans_syntax_expression(t, e);
				if(!val) return 0;
				fprintf(t->fp, "\n");
			} else {
				needvtmp = FALSE;
			}
		}

		fprintf(t->fp, "while(true) {\n");
		if(needvtmp)
			fprintf(t->fp, "let vs = ftmp(stmp, vtmp)\n");
		else
			fprintf(t->fp, "let vs = ftmp(stmp)\n");
		fprintf(t->fp, "if(vs[0] == null) break\n");
		if(needvtmp)
			fprintf(t->fp, "vtmp = vs[0]\n");

		int idx = 0;
		char *p = stmt->value.name;
		while(*p != '\0') {
			fprintf(t->fp, "let ");
			while(*p != '\0' && *p != ',') {
				fputc(*p, t->fp);
				p++;
			}
			fprintf(t->fp, " = vs[%d]\n", idx);
			if(*p != '\0') {
				p++;
				idx++;
			}
		}

		struct syntax_node *block = n->children;
		while(block && block->type != STX_BLOCK) block = block->next;
		if(block) {
			int val = trans_syntax_block(t, block);
			if(!val) return 0;
		} else {
			log_error("missing block in for stmt %d:%s",
					  n->lineno,
					  syntax_statement_tag_string(stmt->tag));
			return 0;
		}
		fprintf(t->fp, "}\n"); //while
		fprintf(t->fp, "}\n"); //for
		return 1;
	}
	case STMT_FOR_IT:
	{
		fprintf(t->fp, "\n{\n");

		struct syntax_node *vn = n->children;
		fprintf(t->fp, "let value = ");
		int val = trans_syntax_expression(t, vn);
		if(!val) return 0;
		fprintf(t->fp, "\n");

		struct syntax_node *ln = vn->next;
		fprintf(t->fp, "let limit = ");
		val = trans_syntax_expression(t, ln);
		if(!val) return 0;
		fprintf(t->fp, "\n");

		struct syntax_node *sn = ln->next;
		if(sn->type == STX_EXPRESSION) {
			fprintf(t->fp, "let step = ");
			int val = trans_syntax_expression(t, sn);
			if(!val) return 0;
			fprintf(t->fp, "\n");			
		} else {
			fprintf(t->fp, "let step = 1\n");
		}
		fprintf(t->fp, "value = value - step\n");
		fprintf(t->fp, "while(true) {\n");
		fprintf(t->fp, "value = value + step\n");
		fprintf(t->fp, "if(step >= 0 && value > limit) break\n");
		fprintf(t->fp, "if(step < 0 && value < limit) break\n");
		
		fprintf(t->fp, "let ");
		fprintf(t->fp, stmt->value.name);
		fprintf(t->fp, " = value\n");
		struct syntax_node *block = n->children;
		while(block && block->type != STX_BLOCK) block = block->next;
		if(block) {
			int val = trans_syntax_block(t, block);
			if(!val) return 0;
		} else {
			log_error("missing block in for stmt %d:%s",
					  n->lineno,
					  syntax_statement_tag_string(stmt->tag));
			return 0;
		}
		fprintf(t->fp, "}\n"); //while stmt
		fprintf(t->fp, "}\n"); //for stmt
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
		return trans_assign(t, stmt);
	}
	case STMT_LOCAL_VAR:
	{
		return trans_local_assign(t, stmt);
	}
	case STMT_FUNC:
	{
		return trans_syntax_function(t, n->children);
	}
	case STMT_LOCAL_FUNC:
	{
		return trans_syntax_function(t, n->children);
	}
	case STMT_FCALL:
	{
		struct syntax_node *c = n->children;
		if(c->type != STX_EXPRESSION) {
			log_error("stmt fcall with non-exp node %d:%s",
					  c->lineno,
					  syntax_node_type_string(c->type));
			return 0;
		}
		
		struct syntax_expression *ce = (struct syntax_expression *)c;
		if(ce->tag != EXP_FCALL) {
			log_error("stmt fcall with non-exp-fcall node %d:%s",
					  c->lineno,
					  syntax_expression_tag_string(ce->tag));
			return 0;
		}
		int val = trans_syntax_functioncall(t, c->children);
		fprintf(t->fp, "\n");
		return val;
	}
	default:
		log_assert(FALSE, "unknown statement %d:%d %s",
				   n->lineno,
				   stmt->tag,
				   syntax_statement_tag_string(stmt->tag));
		return 0;
	}

	fflush(t->fp);
}

static int trans_syntax_expression(struct translator *t, struct syntax_node *n) {
	struct syntax_expression * exp = (struct syntax_expression *)n;
	log_debug("trans expression %d:%s",
			 n->lineno,
			 syntax_expression_tag_string(exp->tag));

	switch(exp->tag) {
	case EXP_NIL:
	case EXP_TRUE:
	case EXP_FALSE:
		fprintf(t->fp, " %s ", syntax_expression_tag_string(exp->tag));
		return 1;
		
	case EXP_NUMBER:
	case EXP_STRING:
		fprintf(t->fp, "%s", exp->value.string);
		return 1;
		
	case EXP_PARENTHESIS:
	{
		fprintf(t->fp, "( ");
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;
		fprintf(t->fp, " )");
		return 1;
	}

	case EXP_ADD:
	case EXP_SUB:
	case EXP_MUL:
	case EXP_DIV:
	case EXP_MOD:
	case EXP_BAND:
	case EXP_BOR:
	case EXP_LSHIFT:
	case EXP_RSHIFT:
	case EXP_LESS:
	case EXP_GREATER:
	case EXP_LE:
	case EXP_GE:
	{
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;
		fprintf(t->fp, " %s ", syntax_expression_tag_string(exp->tag));
		val = trans_syntax_expression(t, n->children->next);
		if(!val) return 0;

		return 1;
	}

	case EXP_EXP:
	{
		fprintf(t->fp, "Math.pow(");
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;
		fprintf(t->fp, ", ");
		val = trans_syntax_expression(t, n->children->next);
		if(!val) return 0;
		
		fprintf(t->fp, ")");
		return 1;
	}
	case EXP_FDIV:
	{
		fprintf(t->fp, "Math.floor(");
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;
		fprintf(t->fp, " / ");
		val = trans_syntax_expression(t, n->children->next);
		if(!val) return 0;		
		fprintf(t->fp, ")");
		return 1;
	}

	case EXP_XOR:
	{
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;
		fprintf(t->fp, " ^ ");
		val = trans_syntax_expression(t, n->children->next);
		if(!val) return 0;

		return 1;		
	}
	case EXP_EQ:
	{
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;
		fprintf(t->fp, " === ");
		val = trans_syntax_expression(t, n->children->next);
		if(!val) return 0;

		return 1;
	}
	case EXP_NE:
	{
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;
		fprintf(t->fp, " !== ");
		val = trans_syntax_expression(t, n->children->next);
		if(!val) return 0;

		return 1;
	}
	case EXP_AND:
	{
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;
		fprintf(t->fp, " && ");
		val = trans_syntax_expression(t, n->children->next);
		if(!val) return 0;

		return 1;		
	}
	case EXP_OR:
	{
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;
		fprintf(t->fp, " || ");
		val = trans_syntax_expression(t, n->children->next);
		if(!val) return 0;

		return 1;		
	}
	case EXP_CONC:
	{
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;
		fprintf(t->fp, " + ");
		val = trans_syntax_expression(t, n->children->next);
		if(!val) return 0;

		return 1;		
	}
	
	case EXP_NOT:
	{	
		fprintf(t->fp, " !");
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;

		return 1;	
	}
	case EXP_NEG:
	{
		fprintf(t->fp, " -");
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;

		return 1;
	}
	case EXP_BNOT:
	{
		fprintf(t->fp, " ~");
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;

		return 1;
	}
	case EXP_LEN:
	{
		fprintf(t->fp, "(");
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;
		fprintf(t->fp, ").length ");
		return 1;
	}

	case EXP_TABLE:
	{
		return trans_syntax_table(t, n->children);
	}
	case EXP_VAR:
	{
		return trans_syntax_variable(t, n->children);
	}
	case EXP_FUNC:
	{
		return trans_syntax_function(t, n->children);
	}
	case EXP_FCALL:
	{
		return trans_syntax_functioncall(t, n->children);
	}

	case EXP_DOTS:
	{
		//can only be used in variable arguments in lua
		fprintf(t->fp, "arguments");
		return 1;
	}

	default:
		log_assert(FALSE, "unknown expression %d:%d %s",
				   n->lineno,
				   exp->tag,
				  syntax_expression_tag_string(exp->tag));
		return 0;
	}
}

static int trans_syntax_variable(struct translator *t, struct syntax_node *n) {
	struct syntax_variable *var = (struct syntax_variable *)n;
	log_debug("trans variable %d: %d, %s, name:%s",
			 n->lineno,
			 var->tag,
			 syntax_variable_tag_string(var->tag),
			 var->name ? var->name : "");
	switch(var->tag) {
	case VAR_NORMAL:
		fprintf(t->fp, "%s", var->name);
		return 1;
	case VAR_KEY:
	{
		int val = trans_syntax_node_children(t, n);
		if(!val) return 0;
		fprintf(t->fp, ".");
		fprintf(t->fp, "%s", var->name);
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
		log_assert(FALSE, "unknown variable tag %d: %d, %s",
				  n->lineno,
				  var->tag,
				  syntax_variable_tag_string(var->tag));
		return 0;
	}
}

static int trans_syntax_function(struct translator *t, struct syntax_node *n) {
	struct syntax_function *func = (struct syntax_function *)n;
	log_debug("trans function %d, name:%s", n->lineno, func->name ? func->name : "");

	if(func->name) {
		if((strstr(func->name, ".") || strstr(func->name, ":"))) {
			char *p = func->name;
			while(p && *p != '\0') {
				if(*p == ':') {
					fputc('.', t->fp);
				} else {
					fputc(*p, t->fp);
				}
				p++;
			}
			fprintf(t->fp, " = function ");
		} else {
			fprintf(t->fp, "function %s ", func->name);
		}
	} else {
		fprintf(t->fp, " function ");
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
			char *p = func->pars;
			while(p && *p != '\0') {
				//skip variable parameters ...
				if(*p != '.') {
					fputc(*p, t->fp);
				}
				p++;
			}
		}
	}
	fprintf(t->fp, ")");

	return trans_syntax_block(t, n->children);
}

static int trans_syntax_functioncall(struct translator *t, struct syntax_node *n) {
	struct syntax_functioncall *fcall = (struct syntax_functioncall *)n;
	log_debug("trans function call %d", n->lineno);
	
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
	log_debug("trans argument %d:%s",
			 n->lineno,
			 syntax_argument_tag_string(arg->tag));
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
		return 1;
	}
	case ARG_TABLE:
		return trans_syntax_table(t, n->children);
	case ARG_STRING:
		fprintf(t->fp, "%s", arg->name);
		return 1;
	default:
		log_assert(FALSE, "unknown argument %d:%d %s",
				   n->lineno,
				   arg->tag,
				   syntax_field_tag_string(arg->tag));
		return 0;		
	}
}

static int trans_syntax_table(struct translator *t, struct syntax_node *n) {
	log_debug("trans table %d", n->lineno);
	if(!n->children) {
		fprintf(t->fp, "{}");
		return 1;
	}

	//struct syntax_table *table = (struct syntax_table *)n;
	struct syntax_field * field = (struct syntax_field *)n->children;
	if(field->tag == FIELD_KEY) {
		fprintf(t->fp, "{");
	} else if(field->tag == FIELD_SINGLE) {
		fprintf(t->fp, "[");
	} else {
		fprintf(t->fp, "{");
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

	if(field->tag == FIELD_KEY) {
		fprintf(t->fp, "}");
	} else if(field->tag == FIELD_SINGLE) {
		fprintf(t->fp, "]");
	} else {
		fprintf(t->fp, "}");
	}
	return 1;
}

static int trans_syntax_field(struct translator *t, struct syntax_node *n) {
	struct syntax_field *field = (struct syntax_field *)n;
	log_debug("trans field, %d:%s, name:%s",
			 n->lineno,
			 syntax_field_tag_string(field->tag),
			 field->name ? field->name : "");
	switch(field->tag) {
	case FIELD_INDEX:
	{
		int val = trans_syntax_expression(t, n->children);
		if(!val) return 0;
		fprintf(t->fp, ": ");
		return trans_syntax_expression(t, n->children->next);
	}
	case FIELD_KEY:
	{
		fprintf(t->fp, "%s", field->name);
		fprintf(t->fp, ": ");
		return trans_syntax_expression(t, n->children);
	}
	case FIELD_SINGLE:
		return trans_syntax_expression(t, n->children);
	default:
		log_assert(FALSE, "unknown field %d:%d %s",
				   n->lineno,
				   field->tag,
				   syntax_field_tag_string(field->tag));
		return 0;
	}
}

static int translate_syntax_node(struct translator *t, struct syntax_node *n) {
	log_debug("trans node line:%d, type:%s, depth:%d, children:%d",
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
		log_assert(FALSE,"unknown syntax node %d:%d %s",
				   n->lineno, n->type, syntax_node_type_string(n->type));
		break;
	}
	return val;
}
