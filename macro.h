/*
 * Transitive Closer : Given an N-Triples input file, transitively close
 * all the predicates contained in it, and print the results to a specified
 * output file.
 *
 * On github: https://github.com/semitrivial/transitive-closer
 */

/*
 * Memory allocation macro
 */
#define CREATE(result, type, number)\
do\
{\
    if (!((result) = (type *) calloc ((number), sizeof(type))))\
    {\
        fprintf(stderr, "Malloc failure at %s:%d\n", __FILE__, __LINE__ );\
        abort();\
    }\
} while(0)

/*
 * Link object into doubly-linked list
 */
#define LINK2(link, first, last, next, prev)\
do\
{\
   if ( !(first) )\
   {\
      (first) = (link);\
      (last) = (link);\
   }\
   else\
      (last)->next = (link);\
   (link)->next = NULL;\
   if (first == link)\
      (link)->prev = NULL;\
   else\
      (link)->prev = (last);\
   (last) = (link);\
} while(0)

/*
 * Unlink object from doubly-linked list
 */
#define UNLINK2(link, first, last, next, prev)\
do\
{\
        if ( !(link)->prev )\
        {\
         (first) = (link)->next;\
           if ((first))\
              (first)->prev = NULL;\
        }\
        else\
        {\
         (link)->prev->next = (link)->next;\
        }\
        if ( !(link)->next )\
        {\
         (last) = (link)->prev;\
           if ((last))\
              (last)->next = NULL;\
        }\
        else\
        {\
         (link)->next->prev = (link)->prev;\
        }\
} while(0)

/*
 * Quickly read char from file
 */
#define QUICK_GETC( ch, fp )\
do\
{\
  if ( read_ptr == read_end )\
  {\
    fread_len = fread( read_buf, sizeof(char), READ_BLOCK_SIZE, fp );\
    if ( fread_len < READ_BLOCK_SIZE )\
      read_buf[fread_len] = '\0';\
    read_ptr = read_buf;\
  }\
  ch = *read_ptr++;\
}\
while(0)
