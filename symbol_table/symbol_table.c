#include "symbol_table.h"

// global variable to track the next available memory location
int nextMemoryLocation = 2500; // initialize to a starting address

void add_symbol(symbol *sym, symbol **symbolList) {
    sym->next = *symbolList;  // link new symbol to current head
    *symbolList = sym;  // update head to new symbol
}

symbol *find_symbol(const char *name, symbol *symbolList) {
    symbol *current = symbolList;  // start with the head of the list
    while (current != NULL) {  // traverse the list
        if (strcmp(current->name, name) == 0) {  // check for a match
            return current;  // return the found symbol
        }
        current = current->next;  // move to the next symbol
    }
    return NULL;  // symbol not found
}

symbol *create_symbol(const char *name) {
    symbol *sym = (symbol *)malloc(sizeof(symbol));  // allocate memory for new symbol
    sym->name = strdup(name);  // copy symbol name
    sym->value = 0;  // initialize value to 0
    sym->memoryLocation = nextMemoryLocation;  // assign current memory location
    nextMemoryLocation += 10;  // move to the next available memory location (adjust step as needed)
    sym->next = NULL;  // no next symbol yet
    return sym;  // return the newly created symbol
}

void print_symbols(FILE *logFile, symbol *symbolList) {
    symbol *current = symbolList;  // start with the head of the list
    fprintf(logFile, "\nSYMBOL LIST\n-----------\n");  // print header
    while (current != NULL) {  // traverse the list
        fprintf(logFile, "%s: %d (Location: %d)\n", current->name, current->value, current->memoryLocation);  // print symbol details
        current = current->next;  // move to the next symbol
    }
}