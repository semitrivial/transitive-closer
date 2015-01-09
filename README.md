Transitive Closer:  Given an N-Triples input file, transitively close
all the predicates contained in it, and print the results to a specified
output file.

After compiling, run the program as follows:
./trans <input N-triples file> <output file>

In order to compile, use the included Makefile, or manually:
gcc -g -Wall trans.c trie.c -o trans

LICENSE: MIT
