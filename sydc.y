%{
#include "symbol_table/symbol_table.h"
#include "syntax_tree/syntax_tree.h"
#include "syntax_tree/node_types.h"
#include "mixal/mix_codegen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *logFile;                // file pointer for logging
struct TreeNode *root = NULL;  // pointer to the root of the syntax tree
symbol *symbolList = NULL;     // pointer to the head of the symbol list

void yyerror(const char *s);   // function to handle parsing errors
int yylex(void);               // function to perform lexical analysis
void expand_node(struct TreeNode *node);  // function to evaluate and expand the syntax tree

%}

%union {
    int yint;                  // integer value
    char ystr[100];            // string value
    struct TreeNode* node;     // tree node pointer
}

%token <yint> DEC_CONST
%token <ystr> ID
%token READ WRITE IF THEN ELSE END REPEAT UNTIL
%token AS LT EQ PLUS MINUS TIMES DIVIDE LPAREN RPAREN SEMICOLON

%type <node> program stmt_seq stmt assign_stmt if_stmt repeat_stmt read_stmt write_stmt exp rel_exp simple_exp term factor

%%

program:
    stmt_seq { 
        $$ = create_node(NODE_PROGRAM, $1, NULL, NULL);
        root = $$;  // program node as the roof of ast
        fprintf(logFile, "Bison: Parsed program\n"); 
    }
    ;

stmt_seq:
    stmt_seq SEMICOLON stmt {
        $$ = create_node(NODE_SEQ, $1, $3, NULL);  
        fprintf(logFile, "Bison: Created stmt_seq with ';'\n"); 
    }
    | stmt {
        $$ = $1;
        fprintf(logFile, "Bison: Created stmt_seq with stmt\n"); 
    }
    ;

stmt:
    assign_stmt
    | if_stmt
    | repeat_stmt
    | read_stmt
    | write_stmt
    ;

assign_stmt:
    ID AS exp {
        $$ = create_node(NODE_ASSIGNMENT, create_node(NODE_ID, NULL, NULL, $1), $3, NULL); 
        fprintf(logFile, "Bison: Created assign_stmt\n");
    }
    ;

read_stmt:
    READ ID {
        $$ = create_node(NODE_READ, NULL, NULL, strdup($2)); 
        fprintf(logFile, "Bison: Created read_stmt\n");
    }
    ;

write_stmt:
    WRITE ID {
        $$ = create_node(NODE_WRITE, NULL, NULL, strdup($2)); 
        fprintf(logFile, "Bison: Created write_stmt\n");
    }
    ;

if_stmt:
    IF exp THEN stmt_seq END {
        $$ = create_node(NODE_IF, $2, $4, NULL); 
        fprintf(logFile, "Bison: Created if_stmt\n");
    }
    | IF exp THEN stmt_seq ELSE stmt_seq END {
        $$ = create_node(NODE_IF, $2, create_node(NODE_ELSE, $4, $6, NULL), NULL); 
        fprintf(logFile, "Bison: Created if-else stmt\n");
    }
    ;

repeat_stmt:
    REPEAT stmt_seq UNTIL exp {
        $$ = create_node(NODE_REPEAT, $2, $4, NULL); 
        fprintf(logFile, "Bison: Created repeat_stmt\n");
    }
    ;

exp:
    rel_exp {
        $$ = $1;
        fprintf(logFile, "Bison: Parsed exp\n");
    }
    ;

rel_exp:
    simple_exp
    | rel_exp LT simple_exp {
        $$ = create_node(NODE_LT, $1, $3, NULL);  
        fprintf(logFile, "Bison: Created rel_exp with '<'\n");
    }
    | rel_exp EQ simple_exp {
        $$ = create_node(NODE_EQ, $1, $3, NULL);  
        fprintf(logFile, "Bison: Created rel_exp with '='\n");
    }
    ;

simple_exp:
    term
    | simple_exp PLUS term {
        $$ = create_node(NODE_ADD, $1, $3, NULL);  
        fprintf(logFile, "Bison: Created simple_exp with '+'\n");
    }
    | simple_exp MINUS term {
        $$ = create_node(NODE_SUBTRACT, $1, $3, NULL);  
        fprintf(logFile, "Bison: Created simple_exp with '-'\n");
    }
    ;

term:
    factor
    | term TIMES factor {
        $$ = create_node(NODE_MULTIPLY, $1, $3, NULL);  
        fprintf(logFile, "Bison: Created term with '*'\n");
    }
    | term DIVIDE factor {
        $$ = create_node(NODE_DIVIDE, $1, $3, NULL);  
        fprintf(logFile, "Bison: Created term with '/'\n");
    }
    ;

factor:
    LPAREN exp RPAREN { 
        $$ = $2;
        fprintf(logFile, "Bison: Created factor with parentheses\n");
    }
    | DEC_CONST {
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "%d", $1);
        $$ = create_node(NODE_NUMBER, NULL, NULL, strdup(buffer));  
        fprintf(logFile, "Bison: Created factor with DEC_CONST\n");
    }
    | ID {
        $$ = create_node(NODE_ID, NULL, NULL, strdup($1));  
        fprintf(logFile, "Bison: Created factor with ID\n");
    }
    ;

%%

void expand_node(TreeNode *node) {
    if (node == NULL) return;  // if node is null, return

    switch (node->type) {  // switch based on node type
        case NODE_PROGRAM: {
            expand_node(node->left);
            break;
        }
        case NODE_ASSIGNMENT: {  // handle assignment node
            symbol *symbol = find_symbol(node->left->value, symbolList);
            if (symbol == NULL) {
                symbol = create_symbol(node->left->value);  // create new symbol if not found
                add_symbol(symbol, &symbolList);  // add it to the symbol list
            }
            symbol->value = decide_expression(node->right, symbolList);  // update symbol value
            fprintf(logFile, "(%s -> %d)\n", symbol->name, symbol->value);  // log symbol and its value
            break;
        }
        case NODE_IF: {  // handle if node
            int conditionIsTrue = decide_expression(node->left, symbolList);  // evaluate condition
            if (conditionIsTrue) {
                expand_node(node->right);  // expand right child if condition is true
            } else if (node->right && node->right->type == NODE_ELSE) {
                expand_node(node->right->right);  // expand else branch if condition is false
            }
            break;
        }
        case NODE_REPEAT: {  // handle repeat node
            do {
                expand_node(node->left);  // expand the loop body
            } while (!decide_expression(node->right, symbolList));  // repeat until condition becomes true
            break;
        }
        case NODE_READ: {  // handle read node
            symbol *symbol = find_symbol(node->value, symbolList);
            if (symbol == NULL) {
                symbol = create_symbol(node->value);  // create new symbol if not found
                add_symbol(symbol, &symbolList);  // add it to the symbol list
            }
            break;
        }
        case NODE_WRITE: {  // handle write node
            symbol *symbol = find_symbol(node->value, symbolList);  // find symbol by name
            if (symbol != NULL) {
                printf("%d", symbol->value);  // print symbol value
            } else {
                printf("Semantic Error: variable %s not found", node->value);
            }
            break;
        }
        case NODE_SEQ: {  // handle sequence of statements node
            expand_node(node->left);  // expand left child
            expand_node(node->right);  // expand right child
            break;
        }
        default:  // default case, handle unrecognized node type
            break;
    }
}

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);  // print parsing error message to standard error
}

void createLogFile() {
    logFile = fopen("log.txt", "w");  // open log file for writing
    if (logFile == NULL) {
        fprintf(stderr, "Error opening log file!\n");  // print error message if file cannot be opened
    } else {
        fprintf(logFile, "LEX/BISON LOGS\n--------------\n");  // initialize log file with a header
    }
}

int main() {
    createLogFile();  // create log file

    yyparse();  // parse input

    print_tree(logFile, root, 0);  // print syntax tree to log file

    fprintf(logFile, "\nASSIGNMENTS\n-----------\n");  // help message

    expand_node(root);  // evaluate and execute program

    print_symbols(logFile, symbolList);  // print symbol table to log file

    generate_mix_code(root);  // generate mixal code from the ast

    fclose(logFile);  // close log file

    return 0;  // exit program
}