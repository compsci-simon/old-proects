/**
 * @file    symboltable.c
 * @brief   A symbol table for AMPL-2020.
 * @author  W.H.K. Bester (whkbester@cs.sun.ac.za)
 * @date    2020-08-10
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "boolean.h"
#include "error.h"
#include "hashtable.h"
#include "symboltable.h"
#include "token.h"

/* --- global static variables ---------------------------------------------- */

static HashTab *table, *saved_table;
/* TODO: Nothing here, but note that the next variable keeps a running coount of
 * the number of variables in the current symbol table.  It will be necessary
 * during code generation, to compute the size of the local variable array of a
 * method frame in the Java virtual machine.
 */
static unsigned int curr_offset;

/* --- function prototypes -------------------------------------------------- */

static void valstr(void *key, void *p, char *str);
static void freeprop(void *p);
static unsigned int shift_hash(void *key, unsigned int size);
static int key_strcmp(void *val1, void *val2);
static void freekey(void *k);
static void freeval(void *v);
void print_test(HashTab *ht);

/* --- symbol table interface ----------------------------------------------- */

void init_symbol_table(void)
{
	saved_table = NULL;
	if ((table = ht_init(0.75f, shift_hash, key_strcmp)) == NULL) {
		eprintf("Symbol table could not be initialised");
	}
	curr_offset = 1;
}

Boolean open_subroutine(char *id, IDprop *prop)
{
	/* TODO:
	 * - Insert the sumroutine name into the global symbol table; return TRUE or
	 *   FALSE, depending on whether or not the insertion succeeded.
	 * - Save the global symbol table to saved_table, initialise a new hash
	 *   table for the subroutine, and reset the current offset.
	 */

	 Boolean insert_success = insert_name(id, prop);
	 if (insert_success) {
		if ((saved_table = ht_init(0.75f, shift_hash, key_strcmp)) == NULL) {
			eprintf("Symbol table could not be initialised");
		}
	 	saved_table = table;
		ht_free(table, &freekey, &freeval);
		table = NULL;
		if ((table = ht_init(0.75f, shift_hash, key_strcmp)) == NULL) {
			eprintf("Symbol table could not be initialised");
		}
	 }
	 return insert_success;
}

void close_subroutine(void)
{
	/* TODO: Release the subroutine table, and reactivate the global table. */
	table = NULL;
	ht_free(table, &freekey, &freeval);
	if ((table = ht_init(0.75f, shift_hash, key_strcmp)) == NULL) {
		eprintf("Symbol table could not be initialised");
	}
	table = saved_table;
	ht_free(saved_table, &freekey, &freeval);
}

Boolean insert_name(char *id, IDprop *prop)
{
	/* TODO: Insert the properties of the identifier into the hash table, and
	 * remember to increment the current offset pointer if the identifier is a
	 * variable.
	 *
	 * VERY IMPORTANT: Remember to read the documentation of this function in
	 * the header file.
	 */
	
	int insert_exit_code;
	 
	if (prop->type & TYPE_INTEGER || prop->type & TYPE_BOOLEAN) {
		prop->offset = curr_offset;
		curr_offset++;
	}

	insert_exit_code = ht_insert(table, id, prop);
	if (insert_exit_code == EXIT_SUCCESS) {
		free(id);
		freeprop(prop);
		return TRUE;
	} else {
		if (prop->type & TYPE_INTEGER || prop->type & TYPE_BOOLEAN) {
			curr_offset--;
		}
		return FALSE;
	}

}

Boolean find_name(char *id, IDprop **prop)
{
	Boolean found;

	/* TODO: Nothing... unless you need something here to prevent local
	 * variables from hiding function names.
	 */
	found = ht_search(table, id, (void **) prop);
	if (!found && saved_table != NULL) {
		found = ht_search(saved_table, id, (void **) prop);
		if (found && !IS_CALLABLE_TYPE((*prop)->type)) {
			found = FALSE;
		}
	}

	return found;
}

int get_variables_width(void)
{
	return curr_offset;
}

void release_symbol_table(void)
{
	/* TODO: Free the underlying structures of the symbol table. */
	ht_free(table, &freekey, &freeval);
}

void print_symbol_table(void)
{
	ht_print(table, valstr);
}

/* --- utility functions ---------------------------------------------------- */

static void valstr(void *key, void *p, char *str)
{
	char *keystr = (char *) key;
	IDprop *idpp = (IDprop *) p;

	/* TODO: Nothing, but this should give you an idea of how to look at the
	 * contents of the symbol table.
	 */

	sprintf(str, "%s.%d", keystr, idpp->offset);
	sprintf(str, "%s@%d[%s]", keystr, idpp->offset,
			get_valtype_string(idpp->type));
}

/* TODO: Here you should add your own utility functions, in particular, for
 * deallocation, hashing, and key comparison.  For hashing, you MUST NOT use the
 * simple strategy of summing the integer values of characters.  I suggest you
 * use some kind of cyclic bit shift hash.
 */

static unsigned int shift_hash(void *key, unsigned int size)
{
	unsigned int hash = 0;
	char *cp = (char *)key;
	const int bit_shift = 2;

	for (int i = 0; cp[i] != '\0'; i++) {
		hash = (hash+(int)cp[i]) << bit_shift;
	}

	return hash % size;
}

static void freekey(void *k)
{
	k = NULL;
}

static void freeval(void *v)
{
	v = NULL;
}

static int key_strcmp(void *val1, void *val2)
{
	return strcmp((const char *)val1, (const char *)val2);
}

static void freeprop(void *p)
{
	free(p);
}




