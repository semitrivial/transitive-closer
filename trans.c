/*
 * Transitive Closer : Given an N-Triples input file, transitively close
 * all the predicates contained in it, and print the results to a specified
 * output file.
 *
 * On github: https://github.com/semitrivial/transitive-closer
 */
#include "trans.h"

reln *first_reln;
reln *last_reln;

int main( int argc, const char* argv[] )
{
  FILE *fp, *outfile;

  if ( argc != 3 )
  {
    printf( "Syntax: trans <n-triples file> <output file>\n\n" );
    return 1;
  }

  fp = fopen( argv[1], "r" );

  if ( !fp )
  {
    fprintf( stderr, "Error: Could not open %s for reading\n\n", argv[1] );
    return 0;
  }

  outfile = fopen( argv[2], "w" );

  if ( !outfile )
  {
    fprintf( stderr, "Error: Could not open %s for writing\n\n", argv[2] );
    return 0;
  }

  initialize();
  parse_triples_file( fp );

  fclose(fp);

  compute_transitive_closure();
  write_transitive_closure( outfile );

  fclose( outfile );

  return 1;
}

void initialize(void)
{
  iris = blank_trie();
  predicates = blank_trie();
}

void parse_triples_file( FILE *fp )
{
  char c, buf[MAX_IRI_LEN+1], *end = &buf[MAX_IRI_LEN], *bptr = buf;
  trie *subj, *pred, *pred_in_predtrie, *obj;
  int line = 1, fQuote = 0, fBrace = 0, fSubj = 0, fPred = 0, fObj = 0, fresh_line = 1, whitespaceable = 1, fAnything = 0;

  /*
   * Variables for QUICK_GETC
   */
  char read_buf[READ_BLOCK_SIZE], *read_end = &read_buf[READ_BLOCK_SIZE], *read_ptr = read_end;
  int fread_len;

  for ( ; ; )
  {
    QUICK_GETC( c, fp );

    if ( c == '\\' )
    {
      char next;

      QUICK_GETC( next, fp );
      if ( !next )
        TRANS_ERROR( "File ended with a backslash" );

      add_char( '\\', bptr++, end, line );
      add_char( next, bptr++, end, line );
      fAnything = 1;

      continue;
    }

    if ( !c )
      break;

    if ( fQuote )
    {
      if ( c == '\n' )
        TRANS_ERROR( "Line ends with unterminated quote" );

      add_char( c, bptr++, end, line );
      if ( c == '"' )
        fQuote = 0;

      continue;
    }

    if ( fBrace )
    {
      if ( c == '\n' )
        TRANS_ERROR( "Line ends with unmatched opening-brace, <" );

      add_char( c, bptr++, end, line );
      if ( c == '>' )
        fBrace = 0;

      continue;
    }

    if ( whitespaceable )
    {
      if ( c == ' ' || c == '\t' )
        continue;

      if ( c == '#' && fresh_line )
      {
        do
          QUICK_GETC( c, fp );
        while ( c && c != '\n' );

        line++;
        continue;
      }

      fresh_line = 0;
      whitespaceable = 0;
    }

    if ( c == ' ' || c == '\t' || ( c == '.' && fPred ) )
    {
      whitespaceable = 1;

      if ( !fSubj )
        set_iri( &subj, &fSubj, buf, bptr );
      else if ( !fPred )
      {
        set_iri( &pred, &fPred, buf, bptr );
        pred_in_predtrie = trie_strdup( buf, predicates );
      }
      else if ( !fObj )
      {
        int fPeriod = (c=='.');

        set_iri( &obj, &fObj, buf, bptr );

        for( ; ; )
        {
          QUICK_GETC( c, fp );

          if ( !c || c == '\n' )
          {
            if ( !fPeriod )
              TRANS_ERROR( "This line seems to be missing the ending period (.)" );

            break;
          }

          switch(c)
          {
            case ' ':
            case '\t':
              continue;

            case '.':
              if ( !fPeriod )
              {
                fPeriod = 1;
                continue;
              }
            default:
              TRANS_ERROR( "Unexpected character after subject, predicate, and object" );
          }
        }

        line++;
        add_reln( pred_in_predtrie, subj, pred, obj );
        fSubj = fPred = fObj = 0;
        fresh_line = 1;
        fAnything = 0;
      }

      bptr = buf;
      continue;
    }

    if ( c == '"' )
    {
      add_char( c, bptr++, end, line );
      fQuote = 1;
      fAnything = 1;
      continue;
    }

    if ( c == '<' )
    {
      add_char( c, bptr++, end, line );
      fBrace = 1;
      fAnything = 1;
      continue;
    }

    if ( c == '\n' )
    {
      if ( !fAnything )
      {
        line++;
        whitespaceable = 1;
        fresh_line = 1;
        continue;
      }
      TRANS_ERROR( "Line seems to end prematurely" );
    }

    fprintf( stderr, "Line %d: This line appears to contain the unexpected character '%c' not enclosed in quotes or in <>\n", line, c );
    abort();
  }
}

void set_iri( trie **which, int *f, char *buf, char *bptr )
{
  *bptr = '\0';
  *which = trie_strdup( buf, iris );
  *f = 1;
}

void add_reln( trie *pred_in_predtrie, trie *subj, trie *pred, trie *obj )
{
  reln *r;

  CREATE( r, reln, 1 );

  r->subj = subj;
  r->obj = obj;

  LINK2( r, pred_in_predtrie->first_reln, pred_in_predtrie->last_reln, next, prev );
}

void compute_transitive_closure( void )
{
  compute_transitive_closure_recursive( predicates );
}

void compute_transitive_closure_recursive( trie *t )
{
  if ( t->first_reln )
  {
    reln *r;

    for ( r = t->first_reln; r; r = r->next )
    {
      reln *r_new;
      trie_wrap *w, *w_new;

      if ( already_has_ancestor( r->obj, r->subj ) )
        continue;

      for ( w = r->subj->first_ancestor; w; w = w->next )
      {
        CREATE( r_new, reln, 1 );
        r_new->subj = w->t;
        r_new->obj = r->obj;

        LINK2( r_new, t->first_reln, t->last_reln, next, prev );
      }

      CREATE( w_new, trie_wrap, 1 );
      w_new->t = r->subj;

      LINK2( w_new, r->obj->first_ancestor, r->obj->last_ancestor, next, prev );
    }

    free_ancestry_data( t );
  }

  if ( t->children )
  {
    trie **child;

    for ( child = t->children; *child; child++ )
      compute_transitive_closure_recursive( *child );
  }
}

void free_ancestry_data( trie *t )
{
  reln *r;

  for ( r = t->first_reln; r; r = r->next )
  {
    trie_wrap *w, *w_next;

    for ( w = r->obj->first_ancestor; w; w = w_next )
    {
      w_next = w->next;

      free( w );
    }

    r->obj->first_ancestor = NULL;
    r->obj->last_ancestor = NULL;
  }
}

void write_transitive_closure( FILE *fp )
{
  write_transitive_closure_recursive( fp, predicates );
}

void write_transitive_closure_recursive( FILE *fp, trie *t )
{
  if ( t->first_reln )
  {
    reln *r;
    char predname[MAX_IRI_LEN+1];
    char full_reln[MAX_IRI_LEN*3 + 3];
    trie *avoid_dupes = blank_trie(), *node;

    sprintf( predname, "%s", trie_to_static(t) );

    for ( r = t->first_reln; r; r = r->next )
    {
      sprintf( full_reln, "%s %s ", trie_to_static( r->subj ), predname );
      sprintf( full_reln + strlen(full_reln), "%s", trie_to_static( r->obj ) );

      node = trie_strdup( full_reln, avoid_dupes );
      node->is_reln = 1;
    }

    write_trie_of_full_relns( fp, avoid_dupes );
  }

  if ( t->children )
  {
    trie **child;

    for ( child = t->children; *child; child++ )
      write_transitive_closure_recursive( fp, *child );
  }
}

void add_char( char c, char *bptr, char *end, int line )
{
  if ( bptr >= end )
    TRANS_ERROR( "An IRI exceeded the maximum IRI length" );

  *bptr = c;
}

void write_trie_of_full_relns( FILE *fp, trie *t )
{
  if ( t->is_reln )
    fprintf( fp, "%s .\n", trie_to_static( t ) );

  if ( t->children )
  {
    trie **child;

    for ( child = t->children; *child; child++ )
      write_trie_of_full_relns( fp, *child );

    free( t->children );
  }

  free( t->label );
  free( t );
}

int already_has_ancestor( trie *obj, trie *subj )
{
  trie_wrap *w;

  for ( w = obj->first_ancestor; w; w = w->next )
    if ( w->t == subj )
      return 1;

  return 0;
}
