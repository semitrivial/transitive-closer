/*
 * Transitive Closer : Given an N-Triples input file, transitively close
 * all the predicates contained in it, and print the results to a specified
 * output file.
 *
 * On github: https://github.com/semitrivial/transitive-closer
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "macro.h"

#define READ_BLOCK_SIZE 1048576  // 1024 * 1024.  For QUICK_GETC.

#define MAX_IRI_LEN 1024
#define MAX_STRING_LEN (MAX_IRI_LEN*3 + 1024)

#define TRANS_ERROR( err )\
  do\
  {\
    fprintf( stderr, "Error while parsing line %d:\n", line );\
    fprintf( stderr, "%s\n", err );\
    abort();\
  }\
  while(0)

typedef struct RELN reln;
typedef struct TRIE trie;
typedef struct TRIE_WRAP trie_wrap;

struct RELN
{
  reln *next;
  reln *prev;
  trie *subj;
  trie *obj;
};

struct TRIE
{
  trie *parent;
  char *label;
  trie **children;
  trie_wrap *first_ancestor;
  trie_wrap *last_ancestor;
  reln *first_reln;
  reln *last_reln;
  int is_reln;
};

struct TRIE_WRAP
{
  trie_wrap *next;
  trie_wrap *prev;
  trie *t;
};

/*
 * Global variables
 */
extern trie *iris;
extern trie *predicates;

/*
 * trans.c
 */
void initialize(void);
void parse_triples_file( FILE *fp );
void compute_transitive_closure( void );
void write_transitive_closure( FILE *fp );
void add_char( char c, char *bptr, char *end, int line );
void set_iri( trie **which, int *f, char *buf, char *bptr );
void set_iri( trie **which, int *f, char *buf, char *bptr );
void add_reln( trie *pred_in_predtrie, trie *subj, trie *pred, trie *obj );
void free_ancestry_data( trie *t );
void compute_transitive_closure_recursive( trie *t );
void write_transitive_closure_recursive( FILE *fp, trie *t );
void write_trie_of_full_relns( FILE *fp, trie *t );

/*
 * trie.c
 */
trie *blank_trie(void);
trie *trie_strdup( char *buf, trie *base );
trie *trie_search( char *buf, trie *base );
char *trie_to_static( trie *t );
