/**
 * @file    token.h
 * @brief   Data type definitions of the lexical analyser (scanner) of
 *          AMPL-2020.
 * @author  W.H.K. Bester (whkbester@cs.sun.ac.za)
 * @date    2020-08-10
 */

#ifndef TOKEN_H
#define TOKEN_H

/** the maximum length of an identifier */
#define MAX_ID_LENGTH 32

/** the types of tokens that the scanner recognises */
typedef enum {

	TOK_EOF,     /* end-of-file    */
	TOK_ID,      /* identifier     */
	TOK_NUM,     /* number literal */
	TOK_STR,     /* string literal */

	/* keywords */
	/* note the boolean operators AND and OR, and the remainder operator REM --
	 * although written out as string literals -- are still treated as operators
	 */
	TOK_ARRAY,
	TOK_AS,
	TOK_BACK,
	TOK_BOOLEAN,
	TOK_CHILLAX,
	TOK_DO,
	TOK_ELIF,
	TOK_ELSE,
	TOK_END,
	TOK_FALSE,
	TOK_IF,
	TOK_INPUT,
	TOK_INTEGER,
	TOK_LET,
	TOK_MAIN,
	TOK_NOT,
	TOK_OUTPUT,
	TOK_PROGRAM,
	TOK_RETURNS,
	TOK_TAKES,
	TOK_TRUE,
	TOK_VARS,
	TOK_WHILE,

	/* relational operators */
	/* the order is significant -- it allows us to do range checking in the
	 * parser */
	TOK_EQ,
	TOK_GE,
	TOK_GT,
	TOK_LE,
	TOK_LT,
	TOK_NE,

	/* additive operators */
	TOK_MINUS,
	TOK_OR,
	TOK_PLUS,

	/* multiplicative operators */
	TOK_AND,
	TOK_DIV,
	TOK_MOD,
	TOK_MUL,

	/* other non-alphabetic operators */
	TOK_LPAR,
	TOK_RPAR,
	TOK_CAT,
	TOK_COMMA,
	TOK_COLON,
	TOK_SEMICOLON,
	TOK_LBRACK,
	TOK_RBRACK

} TokenType;

/** the token data type */
typedef struct {
	TokenType  type;                        /**< the type of the token        */
	union {
		int    value;                      /**< numeric value (for integers) */
		char   lexeme[MAX_ID_LENGTH+1];    /**< lexeme for identifiers       */
		char  *string;                     /**< string (for write)           */
	};
} Token;

/**
 * Returns a string representation of the specified token type.
 *
 * @param[in]   type
 *     the token for which to get a string representation
 * @return      a (constant) string representation of the specified token type
 */
const char *get_token_string(TokenType type);

#endif /* TOKEN_H */
