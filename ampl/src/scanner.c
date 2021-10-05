/**
 * @file    scanner.c
 * @brief   The scanner for AMPL-2020.
 * @author  W.H.K. Bester (whkbester@cs.sun.ac.za)
 * @date    2020-08-10
 */

#include <ctype.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "boolean.h"
#include "error.h"
#include "scanner.h"
#include "token.h"

/* --- type definitions and constants --------------------------------------- */

typedef struct {
	char      *word;                   /* the reserved word, i.e., the lexeme */
	TokenType  type;                   /* the associated token type           */
} ResWord;

/* --- global static variables ---------------------------------------------- */

static FILE *src_file;                 /* the source file pointer             */
static int   ch;                       /* the next source character           */
static int   column_number;            /* the current column number           */

static ResWord reserved[] = {          /* reserved words                      */
	{ "and",       TOK_AND       },
	{ "array",     TOK_ARRAY     },
	{ "as",        TOK_AS        },
	{ "back",      TOK_BACK      },
	{ "boolean",   TOK_BOOLEAN   },
	{ "chillax",   TOK_CHILLAX   },
	{ "do",        TOK_DO        },
	{ "elif",      TOK_ELIF      },
	{ "else",      TOK_ELSE      },
	{ "end",       TOK_END       },
	{ "false",     TOK_FALSE     },
	{ "if",        TOK_IF        },
	{ "input",     TOK_INPUT     },
	{ "integer",   TOK_INTEGER   },
	{ "let",       TOK_LET       },
	{ "main",      TOK_MAIN      },
	{ "mod",       TOK_MOD       },
	{ "not",       TOK_NOT       },
	{ "or",        TOK_OR        },
	{ "output",    TOK_OUTPUT    },
	{ "program",   TOK_PROGRAM   },
	{ "returns",   TOK_RETURNS   },
	{ "takes",     TOK_TAKES     },
	{ "true",      TOK_TRUE      },
	{ "vars",      TOK_VARS      },
	{ "while",     TOK_WHILE     }
};

#define NUM_RESERVED_WORDS     (sizeof(reserved) / sizeof(ResWord))
#define MAX_INITIAL_STRING_LEN (1024)

/* --- function prototypes -------------------------------------------------- */

static void next_char(void);
static void process_number(Token *token);
static void process_string(Token *token);
static void process_word(Token *token);
static void skip_comment(void);

/* --- scanner interface ---------------------------------------------------- */

void init_scanner(FILE *in_file)
{
	src_file = in_file;
	position.line = 1;
	position.col = column_number = 0;
	next_char();
}

void get_token(Token *token)
{
	/* remove whitespace */
	/* TODO: Skip all whitespace characters before the start of the token. */
	while (isspace(ch) || ch == 10) {
		next_char();
	}

	/* remember token start */
	position.col = column_number;

	/* get the next token */
	if (ch != EOF) {

		if (isalpha(ch) || ch == '_') {
			/* process a word */
			process_word(token);

		} else if (isdigit(ch)) {
			/* process a number */
			process_number(token);

		} else switch (ch) {

			/* process a string */
			case '"':
				position.col = column_number;
				next_char();
				process_string(token);
				next_char();
				break;

			/* TODO: Trigger comment skipping, and process all the remaining
			 * tokens.
			 */
			 case '{':
			 	next_char();
				position.col--;
				skip_comment();
				get_token(token);
				break;
			/* relational operators */
			case '=':
				token->type = TOK_EQ;
				next_char();
				break;
			case '>':
				token->type = TOK_GT;
				next_char();
				if (ch == '=') {
					token->type = TOK_GE;
					next_char();
				}
				break;
			case '<':
				token->type = TOK_LT;
				next_char();
				if (ch == '=') {
					token->type = TOK_LE;
					next_char();
				}
				break;
			case '/':
				token->type = TOK_DIV;
				next_char();
				if (ch == '=') {
					token->type = TOK_NE;
					next_char();
					break;
				}
				break;
			/* additive operators */
			case '-':
				token->type = TOK_MINUS;
				next_char();
				break;
			case '+':
				token->type = TOK_PLUS;
				next_char();
				break;

			/* multiplicative operators */
			case '%':
				token->type = TOK_MOD;
				next_char();
				break;
			case '*':
				token->type = TOK_MUL;
				next_char();
				break;

			/* other non-alphabetic operators */
			case '(':
				token->type = TOK_LPAR;
				next_char();
				break;
			case ')':
				token->type = TOK_RPAR;
				next_char();
				break;
			case '&':
				token->type = TOK_CAT;
				next_char();
				break;
			case ',':
				token->type = TOK_COMMA;
				next_char();
				break;
			case ':':
				token->type = TOK_COLON;
				next_char();
				break;
			case ';':
				token->type = TOK_SEMICOLON;
				next_char();
				break;
			case '[':
				token->type = TOK_LBRACK;
				next_char();
				break;
			case ']':
				token->type = TOK_RBRACK;
				next_char();
				break;
			default:
				leprintf("illegal character '%c' (ASCII #%d)", ch, (int)ch);
		}
	} else {
		token->type = TOK_EOF;
	}
}

/* --- utility functions ---------------------------------------------------- */

void next_char(void)
{
	static char last_read = '\0';

	/* TODO:
	 * - Get the next character from the source file.
	 * - Increment the line number if the previous character is EOL.
	 * - Reset the global column number when necessary.
	 * - DO NOT USE feof!!!
	 */
	 last_read = ch;
	 ch = getc(src_file);
	 if (last_read == '\n') {
	 	position.line++;
		position.col = 0;
		column_number = 0;
	 } else {
		position.col++;
		column_number++;
	 }
}

void process_number(Token *token)
{
	/* TODO:
	 * - Build numbers up to the specificied maximum magnitude.
	 * - Store the value in the appropriate token field.
	 * - Set the appropriate token type.
	 * - "Remember" the correct column number globally.
	 */

	token->value = (ch - '0');
	token->type = TOK_NUM;
	next_char();
	while (isdigit(ch)) {
		if (token->value > (INT_MAX - (ch - '0'))/10) {
			// Throw an error here
			leprintf("number too large");
			return;
		}
		token->value = (token->value)*10 + (ch - '0');
		next_char();
	}
	if (isalpha(ch) || ch == '_') {
		leprintf("illegal character '%c' (ASCII #%d)", ch, (int)ch);
	}
	return;
}

void process_string(Token *token)
{
	size_t i, nstring = MAX_INITIAL_STRING_LEN;
	i = 0;
	/* TODO:
	 * - Allocate heap space of the maximum initial string length.
	 * - If a string is about to overflow while scanning it, double the amount
	 *   of space available.
	 * - ONLY printable ASCII characters are allowed; see man 3 isalpha.
	 * - Check the legality of escape codes.
	 * - Set the appropriate token type.
	 */
	char *cp;
	int start_col = position.col - 1;
	int start_line = position.line - 1;
	cp = malloc(nstring);
	while (ch != '"' && ch != EOF) {
		if (ch == 9) {
			leprintf("non-printable character (ASCII #%d)", (int)ch);
		}
		if (i + 1 == nstring) {
			nstring = 2*nstring;
			cp = realloc(cp, nstring);
		}
		*(cp + i) = ch;
		next_char();
		if (i >= 1) {
			if (*(cp + i) == '\\') {
				if (ch != 'n' && ch != 't' && ch != '"' && ch != '\\') {
					position.col--;
					leprintf("illegal escape code '\\%c' in string", ch);
				} else switch (ch) {
					case 'n':
						*(cp + i + 1) = 'n';
						break;
					case 't':
						*(cp + i + 1) = 't';
						break;
					case '"':
						*(cp + i + 1) = '"';
						break;
					default:
						*(cp + i + 1) = '\\';
						break;
				}
				i++;
				next_char();
			}
		}
		i++;
	}
	if (ch == EOF) {
		position.col = start_col;
		position.line = start_line;
		leprintf("string not closed");
	}
	*(cp + i) = '\0';
	token->string = malloc(nstring);
	token->type = TOK_STR;
	strcpy(token->string, cp);
}

void process_word(Token *token)
{
	char lexeme[MAX_ID_LENGTH+1];
	int i, cmp, low, mid, high;

	i = 0;

	position.col = column_number;

	/* check that the id length is less than the maximum */
	/* TODO */

	/* do a binary search through the array of reserved words */
	/* TODO */

	/* if id was not recognised as a reserved word, it is an identifier */
	token->type = TOK_ID;
	strcpy(token->lexeme, lexeme);
	
	while (isdigit(ch) || isalpha(ch) || ch == '_') {
		if (i < MAX_ID_LENGTH) {
			if (ch == '_' && isdigit(token->lexeme[i-1])) {
				leprintf("Illegal character. '_' not allowed after digit in identifier.\n");
			}
			token->lexeme[i] = ch;
			i++;
			next_char();
		} else {
			leprintf("identifier too long.\n");
		}
	}
	
	if (ch == '^') {
		leprintf("illegal character '%c' in identifier", ch);
	}
		
	token->lexeme[i] = '\0';	
	low = 0;
	high = NUM_RESERVED_WORDS;
	mid = (high + low)/2;
	cmp = strcmp(token->lexeme, reserved[mid].word);
	while (high - low > 1) {
		if (cmp < 0) {
			high = mid;
			mid = (high + low)/2;
			cmp = strcmp(token->lexeme, reserved[mid].word);
		} else if (cmp > 0) {
			low = mid;
			mid = (high + low)/2;
			cmp = strcmp(token->lexeme, reserved[mid].word);
		}
		if (cmp == 0) {
			token->type = reserved[mid].type;
			return;
		}
	}
	return;
}

void skip_comment(void)
{
	SourcePos start_pos;

	/* TODO:
	 * - Skip nested comments RECURSIVELY, which is to say, counting strategies
	 *   are not allowed.
	 * - Terminate with an error if comments are not nested properly.
	 */
	start_pos.col = position.col;
	start_pos.line = position.line;

	while (ch != '}') {
		if (ch == '{') {
			next_char();
			skip_comment();
		} else if (ch == EOF) {
			position.col = start_pos.col;
			position.line = start_pos.line;
			leprintf("comment not closed");
		} else {
			next_char();
		}
	}
	next_char();
	return;

	/* force line number of error reporting */
	position = start_pos;
}
