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
#include "valtypes.c"
#include "symboltable.h"
#include "hashtable.h"
#include "errmsg.h"
#include "error.h"
#include "boolean.h"
#include "token.h"

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

/* TODO: Uncomment the previous definition for use during type checking. */
/* DONE */

/* --- function prototypes: parser routines --------------------------------- */

void parse_program(void);
/* TODO: Add the prototypes for the rest of the parse functions. */

void parse_funcdef(void);
void parse_body(void);
void parse_varseq(void);
void parse_type(void);
void parse_statements(void);
void parse_statement(void);
void parse_assign(void);
void parse_back(void);
void parse_do(void);
void parse_if(void);
void parse_input(void);
void parse_output(void);
void parse_while(void);
void parse_expr(void);
void parse_simple(void);
void parse_term(void);
void parse_factor(void);


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
	#if 0
	char *jasmin_path;
	#endif
	/* TODO: Uncomment the previous definition for code generation. */

	/* set up global variables */
	setprogname(argv[0]);

	/* check command-line arguments and environment */
	if (argc != 2) {
		eprintf("Usage: %s <filename>", getprogname());
	}

	/* TODO: Uncomment the following for code generation. */
	#if 0
	if ((jasmin_path = getenv("JASMIN_JAR")) == NULL) {
		eprintf("JASMIN_JAR environment variable not set");
	}
	#endif

	setsrcname(argv[1]);

	/* open the source file, and report an error if it cannot be opened */
	if ((src_file = fopen(argv[1], "r")) == NULL) {
		eprintf("File '%s' could not be opened:", argv[1]);
	}

	/* initialise all compiler units */
	init_scanner(src_file);

	/* compile */
	get_token(&token);
	parse_program();

	/* produce the object code, and assemble */
	/* TODO: For code generation. */

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

	expect(TOK_PROGRAM);
	expect_id(&class_name);
	expect(TOK_COLON);

	while (token.type == TOK_ID) {
		parse_funcdef();
	}

	expect(TOK_MAIN);
	expect(TOK_COLON);

	return_type = TYPE_NONE;
	parse_body();

	DBG_end("</program>");
}

/* TODO: Turn the EBNF into a program by writing one parse function for each
 * production as instructed in the specification.  I suggest you use the
 * production as comment to the function.  Also, you may only report errors
 * through the abort_compile and abort_compile_pos functions.  You must figure
 * out what arguments you should pass for each particular error.  Keep to the
 * EXACT error messages given in the spec.
 */

/* funcdef = id ":" "takes" varseq { ";" varseq } [ "returns" type ] body */
void parse_funcdef(void)
{
	DBG_start("<funcdef>");

	expect(TOK_ID);
	expect(TOK_COLON);
	expect(TOK_TAKES);

	parse_varseq();

	while (token.type == TOK_SEMICOLON) {
		expect(TOK_SEMICOLON);
		parse_varseq();
	}

	if (token.type == TOK_RETURNS) {
		expect(TOK_RETURNS);
		parse_type();
	}

	parse_body();
	
	DBG_end("</funcdef>");
}

/* body = [ "vars" varseq { ";" varseq } ] statements */
void parse_body(void) 
{
	DBG_start("<body>");

	if (token.type == TOK_VARS) {
		expect(TOK_VARS);

		parse_varseq();

		while (token.type == TOK_SEMICOLON) {
			expect(TOK_SEMICOLON);
			parse_varseq();
		}

	}

	parse_statements();

	DBG_end("</body>");
}

/* varseq = id { "," id } "as" type */
void parse_varseq(void) 
{
	DBG_start("<varseq>");

	expect(TOK_ID);

	while (token.type == TOK_COMMA) {
		expect(TOK_COMMA);
		expect(TOK_ID);
	}

	expect(TOK_AS);
	parse_type();
	
	DBG_end("</varseq>");
}

/* ("boolean"|"integer")["array"] */
void parse_type(void)
{
	DBG_start("<type>");

	if (!IS_TYPE_TOKEN(token.type)) {
		abort_compile(ERR_MISSING_TYPE, token.type);
	}
	get_token(&token);

	if (token.type == TOK_ARRAY) {
		expect(TOK_ARRAY);
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

	expect(TOK_LET);
	expect(TOK_ID);

	if (token.type == TOK_LBRACK) {
		expect(TOK_LBRACK);
		parse_simple();
		expect(TOK_RBRACK);
	}

	expect(TOK_EQ);

	if (STARTS_EXPR(token.type)) {
		parse_expr();
	} else if (token.type == TOK_ARRAY) {
		expect(TOK_ARRAY);
		parse_simple();
	} else {
		abort_compile(ERR_MISSING_ARRAY_ALLOCATION_OR_EXPRESSION, token.type);
	}

	DBG_end("</assign>");
}

/* back = "back" [ expr ] */
void parse_back(void)
{
	DBG_start("<back>");

	expect(TOK_BACK);

	if (STARTS_EXPR(token.type)) {
		parse_expr();
	}

	DBG_end("</back>");
}

/* do = "do" id "(" expr { "," expr } ")" */
void parse_do(void)
{
	DBG_start("<do>");

	expect(TOK_DO);
	expect(TOK_ID);
	expect(TOK_LPAR);
	parse_expr();
	
	while (token.type == TOK_COMMA) {
		expect(TOK_COMMA);
		parse_expr();
	}

	expect(TOK_RPAR);

	DBG_end("</do>");
}

/* if = "if" expr ":" statements { "elif" expr ":" statements } [ "else" ":" statements ] */
void parse_if(void)
{
	DBG_start("<if>");

	expect(TOK_IF);
	parse_expr();
	expect(TOK_COLON);

	parse_statements();

	while (token.type == TOK_ELIF) {
		expect(TOK_ELIF);
		parse_expr();
		expect(TOK_COLON);
		parse_statements();
	}

	if (token.type == TOK_ELSE) {
		expect(TOK_ELSE);
		expect(TOK_COLON);
		parse_statements();
	}

	DBG_end("</if>");
}

/* input = "input" id [ "[" simple "]" ] */
void parse_input(void)
{
	DBG_start("<input>");

	expect(TOK_INPUT);
	expect(TOK_ID);

	if (token.type == TOK_LBRACK) {
		expect(TOK_LBRACK);
		parse_simple();
		expect(TOK_RBRACK);
	}

	DBG_end("</input>");
}

/* output = "output" ( string | expr ) { "&" ( string | expr ) } */
void parse_output(void)
{
	DBG_start("<output>");

	expect(TOK_OUTPUT);

	if (token.type == TOK_STR) {
		expect(TOK_STR);
	} else if (STARTS_EXPR(token.type)) {
		parse_expr();
	} else {
		abort_compile(ERR_MISSING_STRING_OR_EXPRESSION, token.type);
	}

	while (token.type == TOK_CAT) {
		expect(TOK_CAT);

		if (token.type == TOK_STR) {
			expect(TOK_STR);
		} else {
			parse_expr();
		}

	}

	DBG_end("</output>");
}

/* while = "while" expr ":" statements */
void parse_while(void)
{
	DBG_start("<while>");

	expect(TOK_WHILE);
	parse_expr();
	expect(TOK_COLON);
	parse_statements();

	DBG_end("</while>");
}

/* expr = simple [ relop simple ] */
void parse_expr(void)
{
	DBG_start("<expr>");
	
	parse_simple();

	if (IS_RELOP(token.type)) {
		get_token(&token);
		parse_simple();
	}

	DBG_end("</expr>");
}

/* simple = [ "-" ] term { addop term } */
void parse_simple(void)
{
	DBG_start("<simple>");

	if (token.type == TOK_MINUS) {
		expect(TOK_MINUS);
	}
	parse_term();
	
	while (IS_ADDOP(token.type)) {
		get_token(&token);
		parse_term();
	}

	DBG_end("</simple>");
}

/* term = factor { mulop factor } */
void parse_term(void)
{
	DBG_start("<term>");

	if (!STARTS_FACTOR(token.type)) {
		abort_compile(ERR_MISSING_FACTOR);
	}
	parse_factor();

	while (IS_MULOP(token.type)) {
		get_token(&token);
		parse_factor();
	}

	DBG_end("</term>");
}

/* factor = id [ "[" simple "]" | "(" expr { "," expr } ")" ] | num | "(" expr ")" | "not" factor | "true" | "false" */
void parse_factor(void)
{
	DBG_start("<factor>");

	switch (token.type) {
		case TOK_ID:
			expect(TOK_ID);

			if (token.type == TOK_LBRACK) {
				expect(TOK_LBRACK);
				parse_simple();
				expect(TOK_RBRACK);
			}
			break;
		case TOK_LPAR:
			expect(TOK_LPAR);
			
			while (token.type == TOK_COMMA) {
				expect(TOK_COMMA);
				parse_expr();
			}

			expect(TOK_RPAR);
			break;
		case TOK_NUM:
			expect(TOK_NUM);
			break;
		case TOK_NOT:
			expect(TOK_NOT);
			parse_factor();
			break;
		case TOK_TRUE:
			expect(TOK_TRUE);
			break;
		case TOK_FALSE:
			expect(TOK_FALSE);
			break;
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
	
	/* I added a break after each case to get rid of warnings */
	switch (err) {
/*	
		case ERR_ILLEGAL_ARRAY_OPERATION:
			break;
		case ERR_ILLEGAL_INDEXED_ARRAY_ALLOCATION:
			break;
		case ERR_MULTIPLE_DEFINITION:
			break;
		case ERR_NOT_A_FUNCTION:
			break;
		case ERR_NOT_AN_ARRAY:
			break;
		case ERR_NOT_A_PROCEDURE:
			break;
		case ERR_NOT_A_VARIABLE:
			break;
		case ERR_SCALAR_VARIABLE_EXPECTED:
			break;
		case ERR_TOO_FEW_ARGUMENTS:
			break;
		case ERR_TOO_MANY_ARGUMENTS:
			break;
		case ERR_UNKNOWN_IDENTIFIER:
			break;

 * This has been comment out due to warnings and it is not currently
 * being used
 */
		case ERR_UNREACHABLE:
			s = va_arg(args, char *);
			break;

		default:
			break;
	}

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
			leprintf("unreachable: %s", s);
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
			leprintf("expected array allocation or expression, but found %s",
			get_token_string(t));
			break;

		case ERR_MISSING_STRING_OR_EXPRESSION:
			t = va_arg(args, int);
			leprintf("expected string or expression, but found %s", get_token_string(t));
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
