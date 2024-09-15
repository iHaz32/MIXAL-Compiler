#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct symbol_tag {
    char *name;          // symbol name
    int value;          // symbol value
    int memoryLocation; // symbol memory location
    struct symbol_tag *next;  // pointer to next symbol in list
} symbol;

extern int nextMemoryLocation; // Global variable for memory location indexing

void add_symbol(symbol *sym, symbol **symbolList);  // add a symbol to the list
symbol *find_symbol(const char *name, symbol *symbolList);  // find a symbol by name
symbol *create_symbol(const char *name);  // create a new symbol
void print_symbols(FILE *logFile, symbol *symbolList);  // print all symbols to log file

#endif