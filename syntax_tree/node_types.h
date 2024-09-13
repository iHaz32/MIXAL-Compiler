#ifndef NODE_TYPES_H
#define NODE_TYPES_H

typedef enum {
    NODE_PROGRAM,      // node for the program (root of ast)
    NODE_ASSIGNMENT,   // node for assignment operation (:=)
    NODE_IF,           // node for if statement
    NODE_ELSE,         // node for else statement
    NODE_REPEAT,       // node for repeat-until loop
    NODE_READ,         // node for read statement
    NODE_WRITE,        // node for write statement
    NODE_SEQ,          // node for sequence of statements
    NODE_ADD,          // node for addition (+)
    NODE_SUBTRACT,     // node for subtraction (-)
    NODE_MULTIPLY,     // node for multiplication (*)
    NODE_DIVIDE,       // node for division (/)
    NODE_LT,           // node for less than operator (<)
    NODE_EQ,           // node for equality operator (=)
    NODE_NUMBER,       // node for a number (constant)
    NODE_ID            // node for an identifier (variable)
} NodeType;

#endif
