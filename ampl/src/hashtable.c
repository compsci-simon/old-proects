/**
 * @file    hashtable.c
 * @brief   A generic hash table.
 * @author  W.H.K. Bester (whkbester@cs.sun.ac.za)
 * @date    2020-08-10
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashtable.h"

#define INITIAL_DELTA_INDEX  4
#define PRINT_BUFFER_SIZE 1024

/** an entry in the hash table */
typedef struct htentry HTentry;
struct htentry {
	void    *key;       /*<< the key                      */
	void    *value;     /*<< the value                    */
	HTentry *next_ptr;  /*<< the next entry in the bucket */
};

/** a hash table container */
struct hashtab {
	/** a pointer to the underlying table                              */
	HTentry **table;
	/** the current size of the underlying table                       */
	unsigned int size;
	/** the current number of entries                                  */
	unsigned int num_entries;
	/** the maximum load factor before the underlying table is resized */
	float max_loadfactor;
	/** the index into the delta array                                 */
	unsigned short idx;
	/** a pointer to the hash function                                 */
	unsigned int (*hash)(void *, unsigned int);
	/** a pointer to the comparison function                           */
	int (*cmp)(void *, void *);
};

/* --- function prototypes -------------------------------------------------- */

static int getsize(HashTab *ht);
static void rehash(HashTab *ht);
static void freekey(void *k);
static void freeval(void *v);

/* TODO: For this implementation, we want to ensure we *always* have a hash
 * table that is of prime size.  To that end, the next array stores the
 * difference between a power of two and the largest prime less than that
 * particular power of two.  When you rehash, compute the new prime size using
 * the following array.
 */

/** the array of differences between a power-of-two and the largest prime less
 * than that power-of-two.                                                    */
unsigned short delta[] = { 0, 0, 1, 1, 3, 1, 3, 1, 5, 3, 3, 9, 3, 1, 3, 19, 15,
1, 5, 1, 3, 9, 3, 15, 3, 39, 5, 39, 57, 3, 35, 1 };

#define MAX_IDX (sizeof(delta) / sizeof(short))

/* --- hash table interface ------------------------------------------------- */

HashTab *ht_init(float loadfactor,
				 unsigned int (*hash)(void *, unsigned int),
				 int (*cmp)(void *, void *))
{
	HashTab *ht;

	/* TODO:
	 * - Initalise a hash table structure by calling an allocation function
	 *   twice:
	 *   (1) once to allocate space for a HashTab variable, and
	 *   (2) once to allocate space for the table field of this new HashTab
	 *       variable.
	 * - If any allocation fails, free anything that has already been allocated
	 *   successfully, and return NULL.
	 * - Also set up the other fields of the newly-allocated HashTab structure
	 *   appropriately.
	 */

	 ht = malloc(sizeof(HashTab));

	 if (!ht) {
		return NULL;
	 } else {
		(*ht).num_entries = 0;
		(*ht).max_loadfactor = loadfactor;
		(*ht).idx = INITIAL_DELTA_INDEX;
		(*ht).hash = hash;
		(*ht).cmp = cmp;
		(*ht).size = getsize(ht);
	 }

	(*ht).table = malloc(sizeof(HTentry)*((*ht).size));

	if (!((*ht).table)) {
		free(ht);
		return NULL;
	}
	
	return ht;
}

int ht_insert(HashTab *ht, void *key, void *value)
{
	int k;
	HTentry *p;

	/* TODO: Insert a new key--value pair, rehashing if necessary.  The best way
	 * to go about rehashing is to put the necessary elements into a static
	 * function called rehash.  Remember to free space (the "old" table) you do
	 * not use any longer.  Also, if something goes wrong, use the #define'd
	 * constants in hashtable.h for return values; remember, unless it runs out
	 * of memory, no operation on a hash table may terminate the program.
	 */

	int i = 0;

	if (ht_search(ht, key, &value)) {
		return HASH_TABLE_KEY_VALUE_PAIR_EXISTS;
	}

	if ((ht->num_entries + 1.00)/(ht->size) >= ht->max_loadfactor) {
		rehash(ht);
	}

	k = ht->hash(key, ht->size);

	for (p = ht->table[k]; TRUE; p = p->next_ptr, i++) {
		if (i == 0 && p == NULL) {
			p = (HTentry *)malloc(sizeof(HTentry));
			p->key = malloc(sizeof(void *));
			p->value = malloc(sizeof(void *));
			
			p->key = key;
			p->value = value;
			p->next_ptr = NULL;
			
			ht->table[k] = (HTentry *)malloc(sizeof(HTentry));
			ht->table[k] = p;
			break;
		} else if (p->next_ptr == NULL) {
			p->next_ptr = (HTentry *)malloc(sizeof(HTentry));
			p->next_ptr->key = malloc(sizeof(void *));
			p->next_ptr->value = malloc(sizeof(void *));

			p->next_ptr->key = key;
			p->next_ptr->value = value;
			p->next_ptr->next_ptr = NULL;
			break;
		}
	}

	(ht->num_entries)++;
	return EXIT_SUCCESS;
}

Boolean ht_search(HashTab *ht, void *key, void **value)
{
	int k;
	HTentry *p;

	/* TODO: Nothing!  This function is complete, and it should explain by
	 * example how the hash table looks and must be accessed.
	 */

	k = ht->hash(key, ht->size);
	for (p = ht->table[k]; p; p = p->next_ptr) {
		if (ht->cmp(key, p->key) == 0) {
			*value = p->value;
			break;
		}
	}

	return (p ? TRUE : FALSE);
}

Boolean ht_free(HashTab *ht, void (*freekey)(void *k), void (*freeval)(void *v))
{
	unsigned int i;
	HTentry *p;

	/* free the nodes in the buckets */
	/* TODO */
	for (i = 0; i < ht->size; i++) {
		if (ht->table[i] != NULL) {
			for (p = ht->table[i]; p != NULL; p = p->next_ptr) {
				freekey(p->key);
				freeval(p->value);
				free(p);
			}
		}
	}

	/* free the table and container */
	/* TODO */
	free(ht->table);
	free(ht);
	ht->table = NULL;
	ht = NULL;

	return EXIT_SUCCESS;
}

void ht_print(HashTab *ht, void (*keyval2str)(void *k, void *v, char *b))
{
	unsigned int i;
	HTentry *p;
	char buffer[PRINT_BUFFER_SIZE];

	/* TODO: This function is complete and useful for testing, but you have to
	 * write your own keyval2str if you want to use it.
	 */

	for (i = 0; i < ht->size; i++) {
		printf("bucket[%2i]", i);
		for (p = ht->table[i]; p != NULL; p = p->next_ptr) {
			keyval2str(p->key, p->value, buffer);
			printf(" --> %s", buffer);
		}
		printf(" --> NULL\n");
	}
}

/* --- utility functions ---------------------------------------------------- */

/* TODO: I suggest completing the following helper functions for use in the
 * global functions ("exported" as part of this unit's public API) given above.
 * You may, however, elect not to use them, and then go about it in your own way
 * entirely.  The ball is in your court, so to speak, but remember: I have
 * suggested using these functions for a reason -- they should make your life
 * easier.
 */

static int getsize(HashTab *ht)
{
	/* TODO: Compute the current prime size of the hash table. */
	int start_size = 1;
	for (int i = 0; i < ht->idx; i++) {
		start_size *= 2;
	}
	start_size -= delta[ht->idx];

	return start_size;
}

static void rehash(HashTab *ht)
{
	/* TODO: Rehash the hash table by
	 * (1) allocating a new table that uses as size the next prime in the
	 *     "almost-double" array,
	 * (2) moving the entries in the existing tables to appropriate positions in
	 *     the new table, and
	 * (3) freeing the old table.
	 */
	HTentry *p;	
	ht->idx++;
	
	HashTab *nht = (HashTab *)malloc(sizeof(HashTab));
	nht->max_loadfactor = ht->max_loadfactor;
	nht->idx = ht->idx;
	nht->size = getsize(nht);
	nht->hash = ht->hash;
	nht->cmp = ht->cmp;
	nht->table = (HTentry **)malloc(sizeof(HTentry) * nht->size);

	for (unsigned int i = 0; i < ht->size; i++) {
		for (p = ht->table[i]; p != NULL; p = p->next_ptr) {
			ht_insert(nht, p->key, p->value);
		}
	}

	ht_free(ht, &freekey, &freeval);

	ht = (HashTab *)malloc(sizeof(HashTab));
	*ht = *nht;
}

static void freekey(void *k)
{
	k = NULL;
	free(k);
}

static void freeval(void *v)
{
	v = NULL;
	free(v);
}

int ht_find_id(HashTab *ht, void *key, void **value)
{
	HTentry *p;
	char *c;
	c = "simon";
	int k = ht->hash(key, ht->size);
	k = 3;
	key = &c;
	printf("key equals %s\n", (char*)key);
	for (p = ht->table[k]; p != NULL; p = p->next_ptr) {
		if (ht->cmp(p->key, key) == 0) {
			*value = p->value;
			return 1;
		}	
	}

	return 0;

}



