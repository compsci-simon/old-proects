/**
 * @file    token.c
 * @brief   Utility functions for AMPL-2020 tokens.
 * @author  W.H.K. Bester (whkbester@cs.sun.ac.za)
 * @date    2020-08-10
 */

#include <assert.h>
#include "token.h"

/* --- the token strings ---------------------------------------------------- */

static char *toknames[] = {
	"end-of-file", "identifier", "numeric literal", "string literal", "'array'",
	"'as'", "'back'", "'boolean'", "'chillax'", "'do'", "'elif'", "'else'",
	"'end'", "'false'", "'if'", "'input'", "'integer'", "'let'", "'main'",
	"'not'", "'output'", "'program'", "'returns'", "'takes'", "'true'",
	"'vars'", "'while'", "'='", "'>='", "'>'", "'<='", "'<'", "'/='", "'-'",
	"'or'", "'+'", "'and'", "'/'", "'mod'", "'*'", "'('", "')'", "'&'", "','",
	"':'", "';'", "'['", "']'"
};

/* --- functions ------------------------------------------------------------ */

const char *get_token_string(TokenType type)
{
	assert(type >= 0 && type < (sizeof(toknames)/sizeof(char *)));
	return toknames[type];
}
