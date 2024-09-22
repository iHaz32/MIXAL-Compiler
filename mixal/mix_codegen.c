#include "mix_codegen.h"
#include "../syntax_tree/node_types.h"
#include "../syntax_tree/syntax_tree.h"
#include "../symbol_table/symbol_table.h"

#define DEBUG 0

FILE *mixFile;                // file pointer for mixal code
static int if_label_count = 0;  // counter for generating unique if-statement labels
static int repeat_label_count = 0;  // counter for generating unique repeat-statement labels
static int write_label_count = 0;  // counter for generating unique write labels
static int temp_count = 1;  // counter for generating unique temporary variables
extern symbol *symbolList;  // external symbol list pointer

void createMixFile() {
    mixFile = fopen("mix.txt", "w");  // open log file for writing
    if (mixFile == NULL) {
        fprintf(stderr, "ERROR: Can not open mixal file.\n");  // print error message if file cannot be opened
    }
}

int is_numeric(const char *str) {
    if (str == NULL || *str == '\0') {
        return 0;  // empty or null string is not numeric
    }

    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return 0;  // non-digit character found
        }
        str++;
    }

    return 1;  // all characters were digits
}

static void generate_expression(TreeNode *node) {
    if (node == NULL) return;

    switch (node->type) {
        case NODE_NUMBER:
            if (DEBUG) printf("NODE_NUMBER\n");  // for debugging
            fprintf(mixFile, " ENTA %s\n", node->value);  // load the number into the accumulator
            break;
        case NODE_ID:
            if (DEBUG) printf("NODE_ID\n");  // for debugging

            symbol *sym = find_symbol(node->value, symbolList);
            if (sym != NULL) {
                fprintf(mixFile, " LDA %d\n", sym->memoryLocation);  // load the variable value into the accumulator using its memory address
            }

            break;
       case NODE_ADD:
            if (DEBUG) printf("NODE_ADD\n");  // for debugging
            int addTemp = temp_count;
            generate_expression(node->left);  // generate code for left operand
            fprintf(mixFile, " STA TEMP%d\n", addTemp);  // store result temporarily
            temp_count++;
            generate_expression(node->right);  // generate code for right operand
            fprintf(mixFile, " ADD TEMP%d\n", addTemp);  // load previous temp and add
            break;
        case NODE_SUBTRACT:
            if (DEBUG) printf("NODE_SUBTRACT\n");  // for debugging
            int subTemp = temp_count;
            generate_expression(node->left);  // generate code for left operand
            fprintf(mixFile, " STA TEMP%d\n", subTemp);  // store result temporarily
            temp_count++;
            generate_expression(node->right);  // generate code for right operand
            fprintf(mixFile, " SUB TEMP%d\n", subTemp);  // load previous temp and subtract

            fprintf(mixFile, " STA OPPTEMP\n");  // A = 0 - A
            fprintf(mixFile, " ENTA 0\n");  // set accumulator to 0
            fprintf(mixFile, " SUB OPPTEMP\n");  // subtract the result from 0
            break;
        case NODE_MULTIPLY:
            if (DEBUG) printf("NODE_MULTIPLY\n");  // for debugging
            int mulTemp = temp_count;
            generate_expression(node->left);  // generate code for left operand
            fprintf(mixFile, " STA TEMP%d\n", mulTemp);  // store result temporarily
            temp_count++;
            generate_expression(node->right);  // generate code for right operand
            fprintf(mixFile, " MUL TEMP%d\n", mulTemp);  // load previous temp and multiply
            fprintf(mixFile, " STX TEMP%d\n", mulTemp);  // store the result
            fprintf(mixFile, " LDA TEMP%d\n", mulTemp);  // load result into accumulator
            fprintf(mixFile, " ENTX 0\n");  // clear index register
            break;
        case NODE_DIVIDE:
            if (DEBUG) printf("NODE_DIVIDE\n");  // for debugging
            int divTemp = temp_count;
            generate_expression(node->left);  // generate code for left operand
            fprintf(mixFile, " STA TEMP%d\n", divTemp);  // store result temporarily
            temp_count++;
            generate_expression(node->right);  // generate code for right operand

            // A = 1/A (swap A and TEMP)
            fprintf(mixFile, " STA SWAPTEMP\n");  // swaptemp = A
            fprintf(mixFile, " LDX SWAPTEMP\n");  // X = swaptemp => X = A
            fprintf(mixFile, " LDA TEMP%d\n", divTemp);  // A = temp
            fprintf(mixFile, " STX TEMP%d\n", divTemp);  // temp = X
            fprintf(mixFile, " STA SWAPTEMP\n");
            fprintf(mixFile, " LDX SWAPTEMP\n");
            fprintf(mixFile, " ENTA 0\n");
            fprintf(mixFile, " DIV TEMP%d\n", divTemp);  // divide A by temp
            break;
        case NODE_LT:
            if (DEBUG) printf("NODE_LT\n");  // for debugging
            generate_expression(node->left);  // evaluate left operand

            if (is_numeric(node->right->value)) {
                fprintf(mixFile, " ENTX %s\n", node->right->value);  // load right operand
                fprintf(mixFile, " CMPA S\n");  // compare accumulator with right operand
            } else {
                symbol *sym = find_symbol(node->right->value, symbolList);
                if (sym != NULL) {
                    fprintf(mixFile, " CMPA %d\n", sym->memoryLocation);  // compare accumulator with memory location
                } else {
                    fprintf(stderr, "ERROR: Symbol %s was not found in symbol list.\n", node->right->value);
                }
            }
            
            break;
        case NODE_EQ:
            if (DEBUG) printf("NODE_EQ\n");  // for debugging
            generate_expression(node->left);  // evaluate left operand

            if (is_numeric(node->right->value)) {
                fprintf(mixFile, " ENTX %s\n", node->right->value);  // load right operand
                fprintf(mixFile, " CMPA S\n");  // compare accumulator with right operand
            } else {
                symbol *sym = find_symbol(node->right->value, symbolList);
                if (sym != NULL) {
                    fprintf(mixFile, " CMPA %d\n", sym->memoryLocation);  // compare accumulator with memory location
                } else {
                    fprintf(stderr, "ERROR: Symbol %s was not found in symbol list.\n", node->right->value);
                }
            }

            break;
        default:
            if (DEBUG) printf("(default in expression)\n");  // for debugging
            break;
    }
}


// function to generate MIX code for a node
void generate_mix_code(TreeNode *node) {
    if (node == NULL) return;

    switch (node->type) {
        case NODE_PROGRAM:
            createMixFile();  // create the mixal file
            fprintf(mixFile, " ORIG 2000\n");  // first mixal command
            generate_mix_code(node->left);  // generate code for the program body
            fprintf(mixFile, " END 2000\n");  // last mixal command
            fclose(mixFile);  // close the mixal file
            break;
        case NODE_ASSIGNMENT:
            if (DEBUG) printf("NODE_ASSIGNMENT\n");  // for debugging
            generate_expression(node->right);  // generate code for the expression

            symbol *sym = find_symbol(node->left->value, symbolList);
            if (sym == NULL) {
                sym = create_symbol(node->left->value);  // create new symbol if not found
                add_symbol(sym, &symbolList);  // add it to the symbol list
            }

            fprintf(mixFile, " STA %d\n", sym->memoryLocation);  // store result into the variable using memory address
            break;
        case NODE_IF:
            if (DEBUG) printf("NODE_IF\n");  // for debugging
            int currentIfLabel = if_label_count++;

            if ((node->type == NODE_IF) && (node->right->type != NODE_ELSE)) {
                generate_expression(node->left);  // generate code for the condition

                // handle comparisons
                if (node->left->type == NODE_LT) {
                    fprintf(mixFile, " JL THEN%d\n", currentIfLabel);  // jump to THEN if condition is less than
                } else if (node->left->type == NODE_EQ) {
                    fprintf(mixFile, " JE THEN%d\n", currentIfLabel);  // jump to THEN if condition is equal
                }

                fprintf(mixFile, " JMP ENDIF%d\n", currentIfLabel);  // unconditional jump to ENDIF
                fprintf(mixFile, "THEN%d NOP\n", currentIfLabel);  // THEN block
                generate_mix_code(node->right);  // generate code for THEN block
                fprintf(mixFile, "ENDIF%d NOP\n", currentIfLabel);  // ENDIF block
            }
            break;
        case NODE_REPEAT:
            if (DEBUG) printf("NODE_REPEAT\n");  // for debugging
            int currentRepeatLabel = repeat_label_count++;
            fprintf(mixFile, "REPEAT%d NOP\n", currentRepeatLabel);  // REPEAT block
            generate_mix_code(node->right);  // generate code for REPEAT block
            generate_expression(node->left);  // generate code for the condition

            fprintf(mixFile, " JMP REPEAT%d\n", currentRepeatLabel);  // jump to REPEAT if condition is met
            break;
        default:
            if (DEBUG) printf("(default in mix_code)\n");  // for debugging
            break;
    }
}