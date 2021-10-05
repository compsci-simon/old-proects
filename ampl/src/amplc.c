/**
 * @file	amplc.c
 *
 * A syntax analyser (parser) and code generation container for AMPL-2020.
 *
 * All scanning errors are handled in the scanner.  Parser errors are handbled
 * by the <code>AbortCompile</code> function.  System and environment errors,
 * like running out of memory, must be handled in the in which they occur.
 * Transient errors, such as non-existent files, are reported where they occur.
 * There are no warnings, i.e., all errors are fatal and must cause compilation
 * to terminate with abnormal error code.
 *
 * @author	W.H.K. Bester (whkbester@cs.sun.ac.za)
 * @date	2020-08-10
 */

/* TODO: Include the appropriate system and project header files */

#include <stdlib.h>
#include <string.h>
#include "scanner.h"
#include "valtypes.h"
#include "symboltable.h"
#include "hashtable.h"
#include "errmsg.h"
#include "error.h"
#include "boolean.h"
#include "token.h"
#include "codegen.h"

/* --- type definitions ----------------------------------------------------- */

/* TODO: Uncomment the following for use during type checking. */

typedef struct variable_s Variable;
struct variable_s {
	char      *id;   /**< variable identifier         */
	ValType    type; /**< variable type               */
	SourcePos  pos;  /**< variable position in source */
	Variable  *next; /**< pointer to next variable    */
};

/* DONE */

/* --- debugging ------------------------------------------------------------ */

/* TODO: Your Makefile has a variable called DFLAGS.  If it is set to contain
 * -DDEBUG_PARSER, it will cause the following prototypes to be included, and the
 * functions to which they refer (given at the end of this file) to be compiled.
 * If, on the other hand, this flag is commented out, by setting DFLAGS to
 * #-DDEBUG_PARSER, these functions will be excluded.  These definitions should
 * be used at the start end end of every parse function.  For an example, the
 * provided parse_program function.
 */

#ifdef DEBUG_PARSER
void debug_start(const char *fmt, ...);
void debug_end(const char *fmt, ...);
void debug_info(const char *fmt, ...);
#define DBG_start(...) debug_start(__VA_ARGS__)
#define DBG_end(...) debug_end(__VA_ARGS__)
#define DBG_info(...) debug_info(__VA_ARGS__)
#else
#define DBG_start(...)
#define DBG_end(...)
#define DBG_info(...)
#endif /* DEBUG_PARSER */

/* --- global variables ----------------------------------------------------- */

Token    token;       /**< the lookahead token.type                */
FILE    *src_file;    /**< the source code file                    */
char    *class_name;  /**< the name of the compiled JVM class file */
ValType  return_type; /**< the return type of the current function */
int is_assign;
int stack_depth, max_stack_depth;
int is_if_or_while = 0;

/* TODO: Uncomment the previous definition for use during type checking. */
/* DONE */

/* --- function prototypes: parser routines --------------------------------- */

void parse_program(void);
/* TODO: Add the prototypes for the rest of the parse functions. */

void parse_funcdef(void);
void parse_body(void);
void parse_varseq(Variable **vars);
void parse_type(ValType *type);
void parse_statements(void);
void parse_statement(void);
void parse_assign(void);
void parse_back(void);
void parse_do(void);
void parse_if(void);
void parse_input(void);
void parse_output(void);
void parse_while(void);
void parse_expr(ValType *type);
void parse_simple(ValType *type);
void parse_term(ValType *type);
void parse_factor(ValType *type);
void init_symbol_table(void);
void reset_offset(void);
void set_max_stack_depth(int max_depth);
int return_curr_offset(void);
void inc_stack_depth(void);

/* --- helper macros -------------------------------------------------------- */

#define STARTS_FACTOR(toktype) \
	(toktype == TOK_ID || toktype == TOK_NUM || \
	 toktype == TOK_LPAR || toktype == TOK_NOT || \
	 toktype == TOK_TRUE || toktype == TOK_FALSE)

#define STARTS_EXPR(toktype) \
	(toktype == TOK_MINUS || STARTS_FACTOR(toktype))

#define IS_ADDOP(toktype) \
	(toktype >= TOK_MINUS && toktype <= TOK_PLUS)

#define IS_MULOP(toktype) \
	(toktype == TOK_AND || toktype == TOK_DIV || \
	 toktype == TOK_MUL || toktype == TOK_MOD)

#define IS_ORDOP(toktype) \
	(IS_ADDOP(toktype) || IS_MULOP(toktype)) /* TODO have a close look at
	this... I'm not too sure this is correct */

#define IS_RELOP(toktype) \
	(toktype == TOK_EQ || toktype == TOK_GE || \
	 toktype == TOK_GT || toktype == TOK_LE || \
	 toktype == TOK_LT || toktype == TOK_NE)

#define IS_TYPE_TOKEN(toktype) \
	(toktype == TOK_BOOLEAN || toktype == TOK_INTEGER)

/* --- function prototypes: helper routines --------------------------------- */

/* TODO: Uncomment the following prototypes for use during type checking. */

void check_types(ValType type1, ValType type2, SourcePos *pos, ...);
void expect(TokenType type);
void expect_id(char **id);
IDprop *idprop(ValType type, unsigned int offset, unsigned int nparams,
               ValType *params);
Variable *variable(char *id, SourcePos pos);
/* DONE */

/* --- function prototypes: error reporting --------------------------------- */

void abort_compile(Error err, ...);
void abort_compile_pos(SourcePos *posp, Error err, ...);

/* --- main routine --------------------------------------------------------- */

int main(int argc, char *argv[])
{
	char *jasmin_path;

	/* TODO: Uncomment the previous definition for code generation. */

	/* set up global variables */
	setprogname(argv[0]);
	stack_depth = 0;
	max_stack_depth = 0;

	/* check command-line arguments and environment */
	if (argc != 2) {
		eprintf("Usage: %s <filename>", getprogname());
	}

	/* TODO: Uncomment the following for code generation. */
	if ((jasmin_path = getenv("JASMIN_JAR")) == NULL) {
		eprintf("JASMIN_JAR environment variable not set");
	}

	setsrcname(argv[1]);

	/* open the source file, and report an error if it cannot be opened */
	if ((src_file = fopen(argv[1], "r")) == NULL) {
		eprintf("File '%s' could not be opened:", argv[1]);
	}

	/* initialise all compiler units */
	init_scanner(src_file);
	init_symbol_table();

	/* compile */
	init_code_generation();
	get_token(&token);
	parse_program();

	/* produce the object code, and assemble */
	/* TODO: For code generation. */
	make_code_file();
	assemble(jasmin_path);

	/* release allocated resources */
	/* TODO: Release resources of symbol table and code generation here */
	fclose(src_file);
	freeprogname();
	freesrcname();

#ifdef DEBUG_PARSER
	printf("SUCCESS!\n");
#endif

	return EXIT_SUCCESS;
}

/* --- parser routines ------------------------------------------------------ */

/*
 * program = "program" id ":" { funcdef } "main" ":" body.
 */
void parse_program(void)
{
	DBG_start("<program>");
	IDprop *idp_filler = idprop(TYPE_CALLABLE, 0, 0, NULL);

	expect(TOK_PROGRAM);
	expect_id(&class_name);
	set_class_name(class_name);
	expect(TOK_COLON);

	while (token.type == TOK_ID) {
		stack_depth = 0;
		max_stack_depth = 0;
		parse_funcdef();
	}
	stack_depth = 0;
	max_stack_depth = 0;

	expect(TOK_MAIN);
	insert_name("main", idp_filler);
	init_subroutine_codegen("main", idp_filler);
	expect(TOK_COLON);
	reset_offset();

	return_type = TYPE_NONE;
	parse_body();
	set_max_stack_depth(max_stack_depth);
	int variable_width = return_curr_offset();
	close_subroutine_codegen(variable_width + 1);

	DBG_end("</program>");
}

/* TODO: Turn the EBNF into a program by writing one parse function for each
 * production as instructed in the specification.  I suggest you use the
 * production as comment to the function.  Also, you may only report errors
 * through the abort_compile and abort_compile_pos functions.  You must figure
 * out whatvars arguments you should pass for each particular error.  Keep to the
 * EXACT error messages given in the spec.
 */

/* funcdef = id ":" "takes" varseq { ";" varseq } [ "returns" type ] body */
void parse_funcdef(void)
{
	DBG_start("<funcdef>");

	ValType type = 0;
	int nparams = 0;
	char *key;
	Variable *var_list = NULL;
	Variable *var_list_start;
	IDprop *idp = idprop(TYPE_NONE, 0, 0, NULL);
	ValType *params = NULL;
	IDprop *idp_reusable = idprop(TYPE_NONE, 0, 0, NULL);
	SourcePos start_pos = position;
	start_pos.col = position.col - strlen(token.lexeme) + 1;

	expect_id(&key);
	if (find_name(key, &idp)) {
		position = start_pos;
		abort_compile(ERR_MULTIPLE_DEFINITION, key);
	}
	expect(TOK_COLON);
	expect(TOK_TAKES);
	var_list = variable(key, position);
	parse_varseq(&var_list);

	var_list_start = var_list;

	while (token.type == TOK_SEMICOLON) {
		expect(TOK_SEMICOLON);
		parse_varseq(&var_list);
	}

	var_list = var_list->next;

	for (; var_list != NULL; var_list = var_list->next, nparams++) {
		// Do nothing
	}

	idp->nparams = nparams;
	params = malloc(sizeof(ValType) * nparams);

	var_list = var_list_start;
	var_list = var_list->next;
	for (int i = 0; i < nparams; i++, var_list = var_list->next) {
		params[i] = var_list->type;
	}
	idp->params = params;


	if (token.type == TOK_RETURNS) {
		expect(TOK_RETURNS);
		parse_type(&type);
		return_type = type;
	} else {
		return_type = 0;
	}

	SET_AS_CALLABLE(type);
	idp->type = type;

	if (!open_subroutine(key, idp)) {
		printf("Error here. Could not open sub-routine");
		exit(1);
	} else {
		var_list = var_list_start;
		for (var_list = var_list->next; var_list != NULL; var_list = var_list->next) {
			idp_reusable->type = var_list->type;
			insert_name(var_list->id, idp_reusable);
		}
		init_subroutine_codegen(key, idp);
		parse_body();
		close_subroutine();
	}

	close_subroutine_codegen(max_stack_depth);
	
	DBG_end("</funcdef>");
}

/* body = [ "vars" varseq { ";" varseq } ] statements */
void parse_body(void) 
{
	DBG_start("<body>");

	Variable *vars;
	vars = NULL;

	if (token.type == TOK_VARS) {
		expect(TOK_VARS);

		parse_varseq(&vars);
		IDprop *idp;
		idp = idprop(TYPE_NONE, 0, 0, NULL);

		while (token.type == TOK_SEMICOLON) {
			expect(TOK_SEMICOLON);
			parse_varseq(&vars);
		}

		for (; vars != NULL; vars = vars->next) {
			idp = idprop(vars->type, 0, 0, NULL);
			insert_name(vars->id, idp);
		}
	}

	parse_statements();

	DBG_end("</body>");
}

/* varseq = id { "," id } "as" type */
void parse_varseq(Variable **vars) 
{
	DBG_start("<varseq>");

	ValType type = 0;
	Variable *start, *begin;
	start = NULL;
	begin = NULL;
	SourcePos start_pos;
	IDprop *idp = idprop(TYPE_NONE, 0, 0, NULL);
	Variable *temp_var;
	temp_var = NULL;

	if (*vars != NULL) {
		begin = *vars;
		for (; (*vars)->next != NULL; *vars = (*vars)->next) {
			// Do nothing
		}
		if ((*vars)->next == NULL) {
			(*vars)->next = emalloc(sizeof(Variable));
			*vars = (*vars)->next;
		} else {
			printf("Error here. (*vars)->next should always be NULL.\n");
			exit(1);
		}
	} else {
		*vars = emalloc(sizeof(Variable));
		begin = *vars;
	}
	start = *vars;

	char *key;
	unsigned int lex_len = strlen(token.lexeme);
	key = malloc(sizeof(char));
	start_pos = position;
	start_pos.col -= strlen(token.lexeme) - 1;
	position.col -= lex_len - 1;
	expect_id(&key);
	position.col += lex_len - 1;

	for (temp_var = begin; temp_var != NULL; temp_var = temp_var->next) {
		if (temp_var->id == NULL) {
			break;
		}
		if (!strcmp(temp_var->id, key) || find_name(key, &idp)) {
			position = start_pos;
			abort_compile(ERR_MULTIPLE_DEFINITION, key);
		}
	}


	(*vars)->id = key;
	(*vars)->pos = position;

	while (token.type == TOK_COMMA) {
		unsigned int lex_len;
		expect(TOK_COMMA);
		lex_len = strlen(token.lexeme);
		start_pos = position;
		start_pos.col -= strlen(token.lexeme) - 1;
		position.col -= lex_len;
		expect_id(&key);
		position.col += lex_len;
		
		for (temp_var = begin; temp_var != NULL; temp_var = temp_var->next) {
			if (temp_var->id == NULL) {
				break;
			}
			if (!strcmp(temp_var->id, key) || find_name(key, &idp)) {
				position = start_pos;
				abort_compile(ERR_MULTIPLE_DEFINITION, key);
			}
		}

		(*vars)->next = variable(key, position);
		*vars = (*vars)->next;
	}

	expect(TOK_AS);
	parse_type(&type);
	*vars = start;

	for (; *vars != NULL; *vars = (*vars)->next) {
		(*vars)->type = type;
	}

	*vars = begin;
	
	DBG_end("</varseq>");
}

/* ("boolean"|"integer")["array"] */
void parse_type(ValType *type)
{
	DBG_start("<type>");

	if (!IS_TYPE_TOKEN(token.type)) {
		abort_compile(ERR_MISSING_TYPE, token.type);
	}
	if (token.type == TOK_BOOLEAN) {
		*type = TYPE_BOOLEAN;
	} else if (token.type == TOK_INTEGER) {
		*type = TYPE_INTEGER;
	} else {
	printf("token.type = %d\n", token.type);
		abort_compile(ERR_UNREACHABLE);
	}
	get_token(&token);

	if (token.type == TOK_ARRAY) {
		expect(TOK_ARRAY);
		*type = (*type | TYPE_ARRAY);
	}

	DBG_end("</type>");
}

/* statements = "chillax" | statement { ";" statement } "end" */
void parse_statements(void)
{
	DBG_start("<statements>");

	if (token.type == TOK_CHILLAX) {
		expect(TOK_CHILLAX);
	} else {
		parse_statement();

		while (token.type == TOK_SEMICOLON) {
			expect(TOK_SEMICOLON);
			parse_statement();
		}

		expect(TOK_END);
	}

	DBG_end("</statements>");
}

/* statement = assign | back | do | if | input | output | while */
void parse_statement(void)
{
	DBG_start("<statement>");

	switch (token.type) {
		case TOK_LET:
			parse_assign();
			break;
		case TOK_BACK:
			parse_back();
			break;
		case TOK_DO:
			parse_do();
			break;
		case TOK_IF:
			parse_if();
			break;
		case TOK_INPUT:
			parse_input();
			break;
		case TOK_OUTPUT:
			parse_output();
			break;
		case TOK_WHILE:
			parse_while();
			break;
		default:
			abort_compile(ERR_MISSING_STATEMENT, token.type);
			break;
	}

	DBG_end("</statement>");
}

/* assign =  "let" id [ "[" simple "]" ] "=" ( expr | "array" simple ) */
void parse_assign(void)
{
	DBG_start("<assign>");

	ValType type1 = 0;
	ValType type2 = 0;
	is_assign = 1;

	expect(TOK_LET);
	SourcePos start_pos = position;
	start_pos.col = position.col - strlen(token.lexeme) + 1;
	
	char *key;
	IDprop *idp = idprop(TYPE_NONE, 0, 0, NULL);
	expect_id(&key);
	if (!find_name(key, &idp)) {
		abort_compile(ERR_UNKNOWN_IDENTIFIER, key);
	}

	if (IS_CALLABLE_TYPE(idp->type)) {
		position = start_pos;
		abort_compile(ERR_NOT_A_VARIABLE, key);
	}

	if (token.type == TOK_LBRACK) {
		if (!IS_ARRAY(idp->type)) {
			abort_compile(ERR_NOT_AN_ARRAY, key);
		}
		expect(TOK_LBRACK);
		start_pos = position;
		start_pos.col = position.col - strlen(token.lexeme) + 1;
		gen_2(JVM_ALOAD, idp->offset);
		parse_simple(&type1);
		if (type1 != TYPE_INTEGER) {
			position = start_pos;
			char string[30] = "for array index of '";
			strcat(string, key);
			strcat(string, "'");
			check_types(type1, TYPE_INTEGER, &position, string);
		}
		expect(TOK_RBRACK);
	}

	expect(TOK_EQ);

	if (STARTS_EXPR(token.type)) {
		if (type1) {
			type2 = -1;
			start_pos = position;
			start_pos.col = position.col - strlen(token.lexeme) + 1;
			parse_expr(&type2);
			ValType x = (idp->type ^ TYPE_ARRAY);
			ValType base_type = TYPE_CALLABLE ^ (TYPE_CALLABLE | type2);
			if (IS_ARRAY(type2)) {
				abort_compile(ERR_ILLEGAL_INDEXED_ARRAY_ALLOCATION, key);
			}
			if (base_type != x) {
				position = start_pos;
				char string[30] = "for array index of '";
				strcat(string, key);
				strcat(string, "'");
				check_types(base_type, x, &position, string);
			}
			gen_1(JVM_IASTORE);
		} else {
			start_pos = position;
			start_pos.col = position.col - strlen(token.lexeme) + 1;

			if (IS_ARRAY(idp->type) && !type1) {
				SET_AS_ARRAY(type2);
			}
			
			type2 = -1;
			parse_expr(&type2);
			
			char string[40] = "for assignment to '";
			strcat(string, key);
			strcat(string, "'");
			if (IS_CALLABLE_TYPE(type2)) {
				ValType func_type = (TYPE_CALLABLE ^ (TYPE_CALLABLE | type2));
				if (func_type != idp->type) {
					position = start_pos;
					check_types(func_type, idp->type, &position, string);
				}
				gen_2(JVM_ISTORE, idp->offset);
				stack_depth--;
			} else {
				if (type2 != idp->type) {
					position = start_pos;
					check_types(type2, idp->type, &position, string);
				}
				gen_2(JVM_ISTORE, idp->offset);
				stack_depth--;
			}
		}
	} else if (token.type == TOK_ARRAY) {

		if (!IS_ARRAY(idp->type)) {
			printf("Error here. Cannot initialize non-array as an array.\n");
		} else if (IS_INTEGER_TYPE(type1)) {
			position = start_pos;
			abort_compile(ERR_ILLEGAL_INDEXED_ARRAY_ALLOCATION, key);
		}
		expect(TOK_ARRAY);
		start_pos = position;
		start_pos.col = position.col - strlen(token.lexeme) + 1;
		
		parse_simple(&type2);
		if (type2 != TYPE_INTEGER) {
			char string[30] = "for array index of '";
			strcat(string, key);
			strcat(string, "'");
			position = start_pos;
			check_types(type2, TYPE_INTEGER, &position, string);
		}
		gen_newarray(T_INT);
		gen_2(JVM_ASTORE, idp->offset);
	
	} else {
		abort_compile(ERR_MISSING_ARRAY_ALLOCATION_OR_EXPRESSION, token.type);
	}

	is_assign = 0;

	DBG_end("</assign>");
}

/* back = "back" [ expr ] */
void parse_back(void)
{
	DBG_start("<back>");

	ValType type = 0;
	SourcePos start_pos = position;
	start_pos.col = position.col - strlen(token.lexeme) + 1;

	expect(TOK_BACK);

	if (STARTS_EXPR(token.type)) {
		start_pos = position;
		parse_expr(&type);
		if (type != return_type) {
			position = start_pos;
			position.col = start_pos.col;
			check_types(type, return_type, &position, "for 'back' statement");
		}
		gen_1(JVM_IRETURN);
	} else {
		position = start_pos;
		abort_compile(ERR_MISSING_BACK_EXPRESSION);
	}

	DBG_end("</back>");
}

/* do = "do" id "(" expr { "," expr } ")" */
void parse_do(void)
{
	DBG_start("<do>");

	ValType type = 0;
	char *key = NULL;
	IDprop *idp = malloc(sizeof(IDprop));
	unsigned int current_param = 0;
	SourcePos tmp_pos1;
	
	expect(TOK_DO);
	expect_id(&key);

	if (!find_name(key, &idp)) {
		abort_compile(ERR_UNKNOWN_IDENTIFIER, key);
	}

	if (!IS_PROCEDURE(idp->type)) {
		position.col += 1;
		abort_compile(ERR_NOT_A_PROCEDURE, key);
	}
	
//	char string[100] = "for parameter ";
//	char curr_param[2] = {0};
//	itoa(curr_param, (current_param + 1), 10);
//	strcat(string, curr_param);
//	strcat(string, " of call to ");
//	strcat(string, key);

	expect(TOK_LPAR);
	tmp_pos1 = position;
	tmp_pos1.col = position.col - strlen(token.lexeme);
	parse_expr(&type);
	if (type != idp->params[current_param]) {
		position = tmp_pos1;
		check_types(type, idp->params[current_param], &position, ""); 
	}

	while (token.type == TOK_COMMA) {
		current_param++;
		if (current_param == idp->nparams) {
			abort_compile(ERR_TOO_MANY_ARGUMENTS, key);
		}
		expect(TOK_COMMA);
		tmp_pos1 = position;
		tmp_pos1.col = position.col - strlen(token.lexeme) + 1;
		parse_expr(&type);
		if (type != idp->params[current_param]) {
			position = tmp_pos1;
			check_types(type, idp->params[current_param], &position, "");
		}
	}

	if (current_param < idp->nparams - 1) {
		position.col += strlen(token.lexeme);
		abort_compile(ERR_TOO_FEW_ARGUMENTS, key);
	}

	expect(TOK_RPAR);

	DBG_end("</do>");
}

/* if = "if" expr ":" statements { "elif" expr ":" statements } [ "else" ":" statements ] */
void parse_if(void)
{
	DBG_start("<if>");

	ValType type1 = 0;
	SourcePos start_pos;
	Label l1 = get_label();
	Label l2 = get_label();
	is_if_or_while = 1;

	expect(TOK_IF);
	parse_expr(&type1);
	gen_2(JVM_LDC, 1);
	gen_2_label(JVM_IF_ICMPNE, l2);
	position.col --;
	check_types(type1, TYPE_BOOLEAN, &position, "for 'if' guard");
	position.col ++;
	expect(TOK_COLON);

	parse_statements();
	gen_2_label(JVM_GOTO, l1);

	while (token.type == TOK_ELIF) {
		expect(TOK_ELIF);
		gen_label(l2);
		start_pos = position;
		start_pos.col = position.col - strlen(token.lexeme) + 1;
		parse_expr(&type1);
		if (type1 != TYPE_BOOLEAN) {
			check_types(type1, TYPE_BOOLEAN, &start_pos, "for 'elif' guard");
		}
		
		l2 = get_label();
		gen_2(JVM_LDC, 1);
		gen_2_label(JVM_IF_ICMPNE, l2);

		expect(TOK_COLON);
		parse_statements();
		gen_2_label(JVM_GOTO, l1);
	}

	gen_label(l2);
	if (token.type == TOK_ELSE) {
		expect(TOK_ELSE);
		expect(TOK_COLON);
		parse_statements();
	}
	gen_label(l1);
	is_if_or_while = 0;
	

	DBG_end("</if>");
}

/* input = "input" id [ "[" simple "]" ] */
void parse_input(void)
{
	DBG_start("<input>");

	ValType type1 = 0;
	char *key = NULL;
	IDprop *idp = emalloc(sizeof(IDprop));
	SourcePos start_pos;

	expect(TOK_INPUT);
	start_pos = position;
	start_pos.col = position.col - strlen(token.lexeme) + 1;
	expect_id(&key);
	
	if (!find_name(key, &idp)) {
		abort_compile(ERR_UNKNOWN_IDENTIFIER, key);
	}
	if (IS_FUNCTION(idp->type) || IS_PROCEDURE(idp->type)) {
		position = start_pos;
		abort_compile(ERR_NOT_A_VARIABLE, key);
	}

	if (token.type == TOK_LBRACK) {
		expect(TOK_LBRACK);
		parse_simple(&type1);
		check_types(type1, TYPE_INTEGER, &position, "");
		ValType x = (idp->type | TYPE_ARRAY) ^ TYPE_ARRAY;
		check_types(x, TYPE_INTEGER, &position, "in input");
		expect(TOK_RBRACK);
	} else {
		if (!IS_INTEGER_TYPE(type1)) {
			position = start_pos;
			abort_compile(ERR_SCALAR_VARIABLE_EXPECTED, key);
		}
	}

	DBG_end("</input>");
}

/* output = "output" ( string | expr ) { "&" ( string | expr ) } */
void parse_output(void)
{
	DBG_start("<output>");

	ValType type = 0;

	expect(TOK_OUTPUT);

	if (token.type == TOK_STR) {
		char *string = token.string;
		expect(TOK_STR);
		gen_print_string(string);
	} else if (STARTS_EXPR(token.type)) {
		parse_expr(&type);
		if (type & TYPE_CALLABLE) {
			gen_print(type ^ TYPE_CALLABLE);
		} else {
			gen_print(type);
		}
	} else {
		abort_compile(ERR_MISSING_STRING_OR_EXPRESSION, token.type);
	}

	while (token.type == TOK_CAT) {
		expect(TOK_CAT);

		if (token.type == TOK_STR) {
			expect(TOK_STR);
		} else {
			parse_expr(&type);
		}

	}

	DBG_end("</output>");
}

/* while = "while" expr ":" statements */
void parse_while(void)
{
	DBG_start("<while>");

	ValType type = 0;
	is_if_or_while = 1;
	Label l1 = get_label();
	Label l2 = get_label();

	expect(TOK_WHILE);
	gen_label(l2);
	parse_expr(&type);
	gen_2(JVM_LDC, 1);
	gen_2_label(JVM_IF_ICMPNE, l1);
	position.col --;
	check_types(type, TYPE_BOOLEAN, &position, "for 'while' guard");
	position.col ++;
	expect(TOK_COLON);
	parse_statements();
	gen_2_label(JVM_GOTO, l2);
	gen_label(l1);
	is_if_or_while = 0;

	DBG_end("</while>");
}

/* expr = simple [ relop simple ] */
void parse_expr(ValType *type)
{
	DBG_start("<expr>");

	ValType type1 = 0;
	ValType type2 = 0;
	SourcePos start_pos;

	if (IS_ARRAY(*type)) {
		type1 = *type;
	}

	parse_simple(&type1);
	if (IS_ARRAY(type1)) {
		SET_AS_ARRAY(*type);
	}

	if (!IS_RELOP(token.type)) {
		*type = type1;
	}

	if (IS_RELOP(token.type)) {
		if (token.type == TOK_EQ || token.type == TOK_NE) {
		
			start_pos = position;
			start_pos.col = position.col - strlen(token.lexeme);
			get_token(&token);
			parse_simple(&type2);
			if (type2 != type1) {
				check_types(type2, type1, &start_pos, "");
			}
			if (is_if_or_while == 1) {
				if (token.type == TOK_EQ) {
					gen_cmp(JVM_IF_ICMPEQ);
				} else {
					gen_cmp(JVM_IF_ICMPNE);
				}
			}
			*type = TYPE_BOOLEAN;
		
		} else if (token.type == TOK_GE || token.type == TOK_GT ||
				   token.type == TOK_LE || token.type == TOK_LT) {

			TokenType relop = token.type;
			if (type1 != TYPE_INTEGER) {
				if ((type1 ^ TYPE_CALLABLE) != TYPE_INTEGER) {
					position.col -= 1;
					check_types(type1, TYPE_INTEGER, &position, "");
				}
			}
			get_token(&token);
			start_pos = position;
			start_pos.col = position.col - strlen(token.lexeme) - 1;
			parse_simple(&type2);
			check_types(type2, TYPE_INTEGER, &start_pos, "");
			*type = TYPE_BOOLEAN;
			
			if (is_if_or_while == 1) {
				switch (relop) {
					case TOK_GE:
						gen_cmp(JVM_IF_ICMPGE);
						break;
					case TOK_GT:
						gen_cmp(JVM_IF_ICMPGT);
						break;
					case TOK_LE:
						gen_cmp(JVM_IF_ICMPLE);
						break;
					case TOK_LT:
						gen_cmp(JVM_IF_ICMPLT);
						break;
					default:
						abort_compile(ERR_UNREACHABLE);
						break;
				}
			}
		
		} else {
		printf("%s\n", get_token_string(token.type));
			abort_compile(ERR_UNREACHABLE);
		}
	}
	
	DBG_end("</expr>");
}

/* simple = [ "-" ] term { addop term } */
void parse_simple(ValType *type)
{
	DBG_start("<simple>");

	ValType type1 = 0;
	ValType type2 = 0;
	SourcePos start_pos;
	
	if (token.type == TOK_MINUS) {
		expect(TOK_MINUS);
		*type = TYPE_INTEGER;
	}

	start_pos = position;
	start_pos.col = position.col - strlen(token.lexeme) + 1;
	parse_term(&type1);
	if (*type == TYPE_INTEGER) {
		if (IS_ARRAY(type1)) {
			position = start_pos;
			TokenType p = TOK_MINUS;
			abort_compile(ERR_ILLEGAL_ARRAY_OPERATION, p);
		}
		check_types(type1, TYPE_INTEGER, &start_pos, "");
	}

	start_pos = position;
	start_pos.col = position.col - strlen(token.lexeme) + 1;

	if (!IS_ADDOP(token.type)) {
		*type = type1;
	}

	while (IS_ADDOP(token.type)) {
		if (IS_ARRAY(*type)) {
			abort_compile(ERR_ILLEGAL_ARRAY_OPERATION, token.type);
		} else if (token.type == TOK_PLUS || token.type == TOK_MINUS) {
			if (IS_ARRAY(type1)) {
				abort_compile(ERR_ILLEGAL_ARRAY_OPERATION, token.type);
			}
			if (type1 != TYPE_INTEGER) {
				check_types(type1, TYPE_INTEGER, &position, "");
			}
			TokenType temp_tok = token.type;
			start_pos = position;
			start_pos.col = position.col - strlen(token.lexeme) + 1;
			get_token(&token);
			parse_term(&type2);
			if (IS_ARRAY(type2)) {
				position = start_pos;
				abort_compile(ERR_ILLEGAL_ARRAY_OPERATION, temp_tok);
			}
			if (type2 != TYPE_INTEGER) {
				check_types(type2, TYPE_INTEGER, &start_pos, "");
			}
			if (temp_tok == TOK_PLUS) {
				stack_depth--;
				gen_1(JVM_IADD);
			} else if (temp_tok == TOK_MINUS) {
				stack_depth--;
				gen_1(JVM_INEG);
				gen_1(JVM_IADD);
			} else {
				abort_compile(ERR_UNREACHABLE);
			}
			*type = type1;
		} else if (token.type == TOK_OR) {
			if (type1 != TYPE_BOOLEAN) {
				check_types(type1, TYPE_BOOLEAN, &start_pos, "");
			}
			start_pos = position;
			start_pos.col = position.col - strlen(token.lexeme) + 1;
			get_token(&token);
			parse_term(&type2);
			if (type2 != TYPE_BOOLEAN) {
				check_types(type2, TYPE_BOOLEAN, &start_pos, "");
			}
			gen_1(JVM_IOR);
			*type = type1;
		} else {
			abort_compile(ERR_UNREACHABLE);
		}
	}

	DBG_end("</simple>");
}

/* term = factor { mulop factor } */
void parse_term(ValType *type)
{
	DBG_start("<term>");

	ValType type1 = 0;
	ValType type2 = 0;
	SourcePos start_pos;

	if (!STARTS_FACTOR(token.type)) {
		abort_compile(ERR_MISSING_FACTOR);
	}

	parse_factor(&type1);

	if (!IS_MULOP(token.type)) {
		*type = type1;
	}

	while (IS_MULOP(token.type)) {
		
		if (token.type == TOK_MUL || token.type == TOK_DIV ||
			token.type == TOK_MOD) {
			
			if (IS_ARRAY(type1)) {
				abort_compile(ERR_ILLEGAL_ARRAY_OPERATION, token.type);
			}
			
			TokenType temp_tok = token.type;
			start_pos = position;
			start_pos.col = position.col - strlen(token.lexeme) + 1;
			get_token(&token);
			if (type1 != TYPE_INTEGER) {
				check_types(type1, TYPE_INTEGER, &start_pos, "");
			}
			parse_factor(&type2);
			if (IS_ARRAY(type2)) {
				position = start_pos;
				abort_compile(ERR_ILLEGAL_ARRAY_OPERATION, temp_tok);
			}
			if (type2 != TYPE_INTEGER) {
				check_types(type2, TYPE_INTEGER, &start_pos, "");
			}
			if (temp_tok == TOK_MUL) {
				stack_depth--;
				gen_1(JVM_IMUL);
			} else if (temp_tok == TOK_DIV) {
				stack_depth--;
				gen_1(JVM_IDIV);
			} else if (temp_tok == TOK_MOD) {
				stack_depth--;
				gen_1(JVM_IREM);
			} else {
				abort_compile(ERR_UNREACHABLE);
			}
			*type = type1;
		} else if (token.type == TOK_AND) {
			if (type1 != TYPE_BOOLEAN) {
				position.col -= strlen(token.lexeme) - 1;
				check_types(type1, TYPE_BOOLEAN, &position, "");
			}
			start_pos = position;
			start_pos.col = position.col - strlen(token.lexeme) + 1;
			get_token(&token);
			parse_factor(&type2);
			if (type2 != TYPE_BOOLEAN) {
				check_types(type2, TYPE_BOOLEAN, &start_pos, "");
			}
			*type = type1;
			gen_1(JVM_IAND);
		} else {
			abort_compile(ERR_UNREACHABLE);
		}

	}

	DBG_end("</term>");
}

/* factor = id [ "[" simple "]" | "(" expr { "," expr } ")" ] | num | "(" expr ")" | "not" factor | "true" | "false" */
void parse_factor(ValType *type)
{
	DBG_start("<factor>");

	ValType type_local = 0;

	switch (token.type) {
		case TOK_ID:
		{
			char *key;
			SourcePos id_start_pos = position;
			id_start_pos.col = position.col - strlen(token.lexeme) + 1;
			expect_id(&key);
			IDprop *idp = malloc(sizeof(IDprop));
			SourcePos start_pos;
			
			if (!find_name(key, &idp)) {
				abort_compile(ERR_UNKNOWN_IDENTIFIER, key);
			}


			if (token.type != TOK_LBRACK && token.type != TOK_LPAR) {
				
				if (IS_CALLABLE_TYPE(idp->type) || IS_ARRAY_TYPE(idp->type)) {
					// Check this again for error handling
				}
				if (!IS_VARIABLE(idp->type)) {
					position = id_start_pos;
					abort_compile(ERR_NOT_A_VARIABLE, key);
				}
				*type = idp->type;
				gen_2(JVM_ILOAD, idp->offset);
			
			} else if (token.type == TOK_LBRACK) {
				
				start_pos = position;
				start_pos.col = position.col + 1;
				expect(TOK_LBRACK);
				parse_simple(&type_local);
				if (type_local != TYPE_INTEGER) {
					position = start_pos;
					char string[30] = "for array index of '";
					strcat(string, key);
					strcat(string, "'");
					check_types(type_local, TYPE_INTEGER, &position, string);
				}
				
				if (!IS_ARRAY_TYPE(idp->type)) {
					position = start_pos;
					abort_compile(ERR_NOT_AN_ARRAY, key);
				}

				expect(TOK_RBRACK);
				*type = idp->type ^ TYPE_ARRAY;
				gen_2(JVM_ALOAD, idp->offset);
				gen_1(JVM_SWAP);
				gen_1(JVM_IALOAD);
			
			} else if (token.type == TOK_LPAR) {
				
				int nparams = 0;
				int current_param = 0;
				expect(TOK_LPAR);

				if (!IS_FUNCTION(idp->type) && !IS_PROCEDURE(idp->type)) {
					abort_compile(ERR_NOT_A_FUNCTION, key);
				}


				if (is_assign && IS_PROCEDURE(idp->type)) {
					position = id_start_pos;
					abort_compile(ERR_NOT_A_FUNCTION, key);
				}

				nparams = idp->nparams;

				if (STARTS_EXPR(token.type)) {
					start_pos = position;
					start_pos.col = position.col - strlen(token.lexeme) + 1;
					parse_expr(&type_local);
					if (type_local != idp->params[current_param]) {
						check_types(type_local, idp->params[current_param],
						&start_pos, "");
					}
					

					while (token.type == TOK_COMMA && current_param < nparams) {
						current_param++;
						if (current_param == nparams) {
							abort_compile(ERR_TOO_MANY_ARGUMENTS, key);
						}
						get_token(&token);
						start_pos = position;
						start_pos.col = position.col - strlen(token.lexeme);
						parse_expr(&type_local);
						if (type_local != idp->params[current_param]) {
							check_types(type_local, idp->params[current_param],
							&start_pos, "");
						}
						
					}

					if (current_param != nparams - 1) {
						if (current_param < nparams - 1) {
							abort_compile(ERR_TOO_FEW_ARGUMENTS, key);
						} else {
							abort_compile(ERR_TOO_MANY_ARGUMENTS, key);
						}
					}

				}

				*type = idp->type;
				expect(TOK_RPAR);
				
				gen_call(key, idp);
			} else {
				abort_compile(ERR_UNREACHABLE);
			}
			break;
		}
		case TOK_LPAR:
		{
			expect(TOK_LPAR);
			
			parse_expr(&type_local);

			if (token.type != TOK_COMMA) {
				*type = type_local;
			}

			while (token.type == TOK_COMMA) {
				expect(TOK_COMMA);
				parse_expr(&type_local);
			}

			expect(TOK_RPAR);
			break;
		}
		case TOK_NUM:
		{
			inc_stack_depth();
			*type = TYPE_INTEGER;
			gen_2(JVM_LDC, token.value);
			expect(TOK_NUM);
			break;
		}
		case TOK_NOT:
		{
			SourcePos start_pos;
			expect(TOK_NOT);
			start_pos = position;
			start_pos.col = position.col - strlen(token.lexeme) + 1;
			start_pos = position;
			start_pos.col = position.col;
			parse_factor(&type_local);
			if (type_local != TYPE_BOOLEAN) {
				check_types(type_local, TYPE_BOOLEAN, &start_pos, "");
			}
			*type = type_local;
			break;
		}
		case TOK_TRUE:
		{
			expect(TOK_TRUE);
			*type = TYPE_BOOLEAN;
			inc_stack_depth();
			gen_2(JVM_LDC, 1);
			break;
		}
		case TOK_FALSE:
		{
			expect(TOK_FALSE);
			*type = TYPE_BOOLEAN;
			inc_stack_depth();
			gen_2(JVM_LDC, 0);
			break;
		}
		default:
			abort_compile(ERR_MISSING_FACTOR, token.type);
	}

	DBG_end("</factor>");
}

/* --- helper routines ------------------------------------------------------ */

#define MAX_MESSAGE_LENGTH 256
char msgbuf[MAX_MESSAGE_LENGTH];

/* TODO: Uncomment the following function for use during type checking. */

void check_types(ValType found, ValType expected, SourcePos *pos, ...)
{
	char *s;
	va_list ap;

	if (found != expected) {
		msgbuf[0] = '\0';
		va_start(ap, pos);
		s = va_arg(ap, char *);
		vsnprintf(msgbuf, MAX_MESSAGE_LENGTH, s, ap);
		va_end(ap);
		if (pos != NULL) {
			position = *pos;
		}
		leprintf("incompatible types (expected %s, found %s) %s",
			get_valtype_string(expected), get_valtype_string(found), msgbuf);
	}
}

void expect(TokenType type)
{
	if (token.type == type) {
		get_token(&token);
	} else {
		abort_compile(ERR_EXPECT, type);
	}
}

void expect_id(char **id)
{
	if (token.type == TOK_ID) {
		*id = strdup(token.lexeme);
		get_token(&token);
	} else {
		abort_compile(ERR_EXPECT, TOK_ID);
	}
}

void inc_stack_depth(void) {
	stack_depth++;
	if (stack_depth > max_stack_depth) {
		max_stack_depth = stack_depth;
	}
}

/* TODO: Uncomment the following functions for use during type checking. */
/* DONE */

IDprop *idprop(ValType type, unsigned int offset, unsigned int nparams,
               ValType *params)
{
	IDprop *ip;

	ip = emalloc(sizeof(IDprop));
	ip->type = type;
	ip->offset = offset;
	ip->nparams = nparams;
	ip->params = params;

	return ip;
}

Variable *variable(char *id, SourcePos pos)
{
	Variable *v;

	v = emalloc(sizeof(Variable));
	v->id = id;
	v->type = TYPE_NONE;
	v->pos = pos;
	v->next = NULL;

	return v;
}

/* --- error handling routine ----------------------------------------------- */

void _abort_compile(SourcePos *posp, Error err, va_list args);

void abort_compile(Error err, ...)
{
	va_list args;

	va_start(args, err);
	_abort_compile(NULL, err, args);
	va_end(args);
}

void abort_compile_pos(SourcePos *posp, Error err, ...)
{
	va_list args;

	va_start(args, err);
	_abort_compile(posp, err, args);
	va_end(args);
}

void _abort_compile(SourcePos *posp, Error err, va_list args)
{
	char expstr[MAX_MESSAGE_LENGTH], *s;
	int t;

	if (posp)
		position = *posp;

	snprintf(expstr, MAX_MESSAGE_LENGTH, "expected %%s, but found %s",
		get_token_string(token.type));
	
	switch (err) {

		/* TODO: Add additional cases here as is necessary, referring to
		 * errmsg.h for all possible errors.  Some errors only become possible
		 * to recognise once we add type checking.  Until you get to type
		 * checking, you can handle such errors by adding the default case.
		 * However, you final submission MUST handle all cases explicitly.
		 */

		case ERR_EXPECT:
			t = va_arg(args, int);
			leprintf(expstr, get_token_string(t));
			break;

		case ERR_UNREACHABLE:
			leprintf("unreachable code");
			break;

		case ERR_MISSING_TYPE:
			t = va_arg(args, int);
			leprintf("expected type, but found %s", get_token_string(t));
			break;

		case ERR_MISSING_STATEMENT:
			t = va_arg(args, int);
			leprintf("expected statement, but found %s", get_token_string(t));
			break;

		case ERR_MISSING_FACTOR:
			t = va_arg(args, int);
			leprintf("expected factor, but found %s", get_token_string(t));
			break;

		case ERR_MISSING_ARRAY_ALLOCATION_OR_EXPRESSION:
			t = va_arg(args, int);
			leprintf("expected array allocation or expression, but found %s", get_token_string(t));
			break;

		case ERR_ILLEGAL_ARRAY_OPERATION:
			t = va_arg(args, int);
			leprintf("%s is an illegal array operation", get_token_string(t));
			break;
		
		case ERR_MISSING_STRING_OR_EXPRESSION:
			t = va_arg(args, int);
			leprintf("expected string or expression, but found %s", get_token_string(t));
			break;

		case ERR_UNKNOWN_IDENTIFIER:
			s = va_arg(args, char *);
			position.col -= strlen(s);
			leprintf("unknown identifier '%s'", s);
			break;
		
		case ERR_NOT_A_FUNCTION:
			s = va_arg(args, char *);
			leprintf("'%s' is not a function", s);
			break;
		
		case ERR_NOT_A_PROCEDURE:
			s = va_arg(args, char *);
			position.col -= strlen(s) + 1;
			leprintf("'%s' is not a procedure", s);
			break;
		
		case ERR_NOT_A_VARIABLE:
			s = va_arg(args, char *);
			leprintf("'%s' is not a variable", s);
			break;
		
		case ERR_NOT_AN_ARRAY:
			s = va_arg(args, char *);
			position.col -= strlen(s) + 1;
			leprintf("'%s' is not an array", s);
			break;
		
		case ERR_ILLEGAL_INDEXED_ARRAY_ALLOCATION:
			s = va_arg(args, char *);
			position.col -= strlen(s) - 1;
			leprintf("illegal allocation to indexed array '%s'", s);
			break;
		
		case ERR_TOO_FEW_ARGUMENTS:
			s = va_arg(args, char *);
			leprintf("too few arguments for call to '%s'", s);
			break;
		
		case ERR_TOO_MANY_ARGUMENTS:
			s = va_arg(args, char *);
			leprintf("too many arguments for call to '%s'", s);
			break;
		
		case ERR_ILLEGAL_BACK_EXPRESSION:
			leprintf("'back' expression not allowed in procedure");
			break;
		
		case ERR_MISSING_BACK_EXPRESSION:
			leprintf("missing 'back' expression in function");
			break;
			
		case ERR_MULTIPLE_DEFINITION:
			s = va_arg(args, char *);
			leprintf("multiple definition of '%s'", s);
			break;
		
		case ERR_SCALAR_VARIABLE_EXPECTED:
			s = va_arg(args, char *);
			leprintf("expected scalar variable instead of '%s'", s);
			break;
		
		default:
			break;

	}
}

/* --- debugging output routines -------------------------------------------- */

#ifdef DEBUG_PARSER

static int indent = 0;

void debug_start(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	debug_info(fmt, ap);
	va_end(ap);
	indent += 2;
}

void debug_end(const char *fmt, ...)
{
	va_list ap;

	indent -= 2;
	va_start(ap, fmt);
	debug_info(fmt, ap);
	va_end(ap);
}

void debug_info(const char *fmt, ...)
{
	int i;
	char *buf_ptr;
	va_list ap;

	buf_ptr = msgbuf;

	va_start(ap, fmt);

	for (i = 0; i < indent; i++) {
		*buf_ptr++ = ' ';
	}
	vsprintf(buf_ptr, fmt, ap);

	buf_ptr += strlen(buf_ptr);
	snprintf(buf_ptr, MAX_MESSAGE_LENGTH, " in line %d.\n", position.line);
	fflush(stdout);
	fputs(msgbuf, stdout);
	fflush(NULL);

	va_end(ap);
}

#endif /* DEBUG_PARSER */
