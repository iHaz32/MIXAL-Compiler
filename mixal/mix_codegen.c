#include "mix_codegen.h"
#include "../syntax_tree/node_types.h"
#include "../syntax_tree/syntax_tree.h"
#include "../symbol_table/symbol_table.h"

#define DEBUG 0

FILE *mixFile;                // file pointer for mixal code
static int if_label_count = 0;  // Counter for generating unique if-statement labels
static int repeat_label_count = 0;  // Counter for generating unique repeat-statement labels
static int write_label_count = 0; // Counter for generating unique write labels
static int temp_count = 1;  // Counter for generating unique temporary variables
extern symbol *symbolList;  // External symbol list pointer


void createMixFile() {
    mixFile = fopen("mix.mixal", "w");  // open log file for writing
    if (mixFile == NULL) {
        fprintf(stderr, "ERROR: Can not open mixal file.\n");  // print error message if file cannot be opened
    }
}

int is_numeric(const char *str) {
    if (str == NULL || *str == '\0') {
        return 0; // Empty or null string is not numeric
    }

    while (*str) {
        if (!isdigit((unsigned char)*str)) {
            return 0; // Non-digit character found
        }
        str++;
    }

    return 1; // All characters were digits
}

static void generate_expression(TreeNode *node) {
    if (node == NULL) return;

    switch (node->type) {
        case NODE_NUMBER:
            if (DEBUG) printf("NODE_NUMBER\n");  // For debugging
            fprintf(mixFile, " ENTA %s\n", node->value);  // Load the number into the accumulator
            break;
        case NODE_ID:
            if (DEBUG) printf("NODE_ID\n");  // For debugging

            symbol *sym = find_symbol(node->value, symbolList);
            if (sym != NULL) {
                fprintf(mixFile, " LDA %d\n", sym->memoryLocation);  // Load the variable value into the accumulator using its memory address
            }

            break;
       case NODE_ADD:
            if (DEBUG) printf("NODE_ADD\n");  // For debugging
            int addTemp = temp_count;
            // Generate code for left operand
            generate_expression(node->left);
            // Store result temporarily
            fprintf(mixFile, " STA TEMP%d\n", addTemp);
            temp_count++;
            // Generate code for right operand
            generate_expression(node->right);
            // Load temporary result and add
            fprintf(mixFile, " ADD TEMP%d\n", addTemp);  // Load previous temp
            break;
        case NODE_SUBTRACT:
            if (DEBUG) printf("NODE_SUBTRACT\n");  // For debugging
            int subTemp = temp_count;
            // Generate code for left operand
            generate_expression(node->left);
            // Store result temporarily
            fprintf(mixFile, " STA TEMP%d\n", subTemp);
            temp_count++;
            // Generate code for right operand
            generate_expression(node->right);
            // Load temporary result and subtract
            fprintf(mixFile, " SUB TEMP%d\n", subTemp);  // Load previous temp

            // A = 0-A
            fprintf(mixFile, " STA OPPTEMP\n");
            fprintf(mixFile, " ENTA 0\n");
            fprintf(mixFile, " SUB OPPTEMP\n");
            break;
        case NODE_MULTIPLY:
            if (DEBUG) printf("NODE_MULTIPLY\n");  // For debugging
            int mulTemp = temp_count;
            // Generate code for left operand
            generate_expression(node->left);
            // Store result temporarily
            fprintf(mixFile, " STA TEMP%d\n", mulTemp);
            temp_count++;
            // Generate code for right operand
            generate_expression(node->right);
            // Load temporary result and multiply
            fprintf(mixFile, " MUL TEMP%d\n", mulTemp);  // Load previous temp
            fprintf(mixFile, " STX TEMP%d\n", mulTemp);
            fprintf(mixFile, " LDA TEMP%d\n", mulTemp);
            fprintf(mixFile, " ENTX 0\n");
            break;
        case NODE_DIVIDE:
            if (DEBUG) printf("NODE_DIVIDE\n");  // For debugging
            int divTemp = temp_count;
            // Generate code for left operand
            generate_expression(node->left);
            // Store result temporarily
            fprintf(mixFile, " STA TEMP%d\n", divTemp);
            temp_count++;
            // Generate code for right operand
            generate_expression(node->right);
            
            // A = 1/A (Swap A and TEMP)
            fprintf(mixFile, " STA SWAPTEMP\n");  // swaptemp = A
            fprintf(mixFile, " LDX SWAPTEMP\n");  // X = swaptemp => X = A
            fprintf(mixFile, " LDA TEMP%d\n", divTemp);  // A = temp
            fprintf(mixFile, " STX TEMP%d\n", divTemp);  // temp = X
            fprintf(mixFile, " STA SWAPTEMP\n");
            fprintf(mixFile, " LDX SWAPTEMP\n");
            fprintf(mixFile, " ENTA 0\n");
            fprintf(mixFile, " DIV TEMP%d\n", divTemp);

            break;
        case NODE_LT:
            if (DEBUG) printf("NODE_LT\n");  // For debugging
            generate_expression(node->left);   // Evaluate left operand

            if (is_numeric(node->right->value)) {
                fprintf(mixFile, " ENTX %s\n", node->right->value);
                fprintf(mixFile, " CMPA S\n");
            } else {
                symbol *sym = find_symbol(node->right->value, symbolList);
                if (sym != NULL) {
                    fprintf(mixFile, " CMPA %d\n", sym->memoryLocation);
                } else {
                    fprintf(stderr, "ERROR: Symbol %s was not found in symbol list.\n", node->right->value);
                }
            }
            
            break;
        case NODE_EQ:
            if (DEBUG) printf("NODE_EQ\n");  // For debugging
            generate_expression(node->left);   // Evaluate left operand

            if (is_numeric(node->right->value)) {
                fprintf(mixFile, " ENTX %s\n", node->right->value);
                fprintf(mixFile, " CMPA S\n");
            } else {
                symbol *sym = find_symbol(node->right->value, symbolList);
                if (sym != NULL) {
                    fprintf(mixFile, " CMPA %d\n", sym->memoryLocation);
                } else {
                    fprintf(stderr, "ERROR: Symbol %s was not found in symbol list.\n", node->right->value);
                }
            }

            break;
        default:
            if (DEBUG) printf("(default in expression)\n");  // For debugging
            break;
    }
}



// Function to generate MIX code for a node
void generate_mix_code(TreeNode *node) {
    if (node == NULL) return;

    switch (node->type) {
        case NODE_PROGRAM:
            createMixFile();
            fprintf(mixFile, " ORIG 2000\n");  // First mixal command
            generate_mix_code(node->left);  // Generate code for the program body
            fprintf(mixFile, " END 2000\n");   // Last mixal command
            fclose(mixFile);
            break;
        case NODE_ASSIGNMENT:
            if (DEBUG) printf("NODE_ASSIGNMENT\n");  // For debugging
            generate_expression(node->right);  // Generate code for the expression

            symbol *sym = find_symbol(node->left->value, symbolList);
            if (sym == NULL) {
                sym = create_symbol(node->left->value);  // create new symbol if not found
                add_symbol(sym, &symbolList);  // add it to the symbol list
            }

            fprintf(mixFile, " STA %d\n", sym->memoryLocation);  // Store result into the variable using memory address
            break;
        case NODE_IF:
            if (DEBUG) printf("NODE_IF\n");  // For debugging
            int currentIfLabel = if_label_count++;
    
            if ((node->type == NODE_IF) && (node->right->type != NODE_ELSE)) {
                // Generate code for the condition
                generate_expression(node->left);

                // Handle comparisons (expand as needed)
                if (node->left->type == NODE_LT) {
                    fprintf(mixFile, " JL THEN%d\n", currentIfLabel);
                } else if (node->left->type == NODE_EQ) {
                    fprintf(mixFile, " JE THEN%d\n", currentIfLabel);
                }
                
                // Unconditional jump to ENDIF
                fprintf(mixFile," JMP ENDIF%d\n", currentIfLabel);
                
                // THEN block
                fprintf(mixFile, "THEN%d NOP\n", currentIfLabel);
                generate_mix_code(node->right); // Generate code for the 'then' part
                
                // End of if
                fprintf(mixFile, "ENDIF%d NOP\n", currentIfLabel);

            } else if ((node->type == NODE_IF) && (node->right->type == NODE_ELSE)) {
                // Generate code for the condition
                generate_expression(node->left);

                // Handle comparisons
                if (node->left->type == NODE_LT) {
                    fprintf(mixFile, " JL THEN%d\n", currentIfLabel);
                } else if (node->left->type == NODE_EQ) {
                    fprintf(mixFile, " JE THEN%d\n", currentIfLabel);
                }
                
                // Unconditional jump to ELSE
                fprintf(mixFile, " JMP ELSE%d\n", currentIfLabel);
                
                // THEN block
                fprintf(mixFile, "THEN%d NOP\n", currentIfLabel);
                generate_mix_code(node->right->left); // Generate code for the 'then' part
                fprintf(mixFile, " JMP ENDIF%d\n", currentIfLabel); // Jump to the end of if-else
                
                // ELSE block
                fprintf(mixFile, "ELSE%d NOP\n", currentIfLabel);
                generate_mix_code(node->right->right); // Generate code for the 'else' part
                
                // End of if-else
                fprintf(mixFile, "ENDIF%d NOP\n", currentIfLabel);
            }
            break;
        case NODE_REPEAT:
            if (DEBUG) printf("NODE_REPEAT\n");  // For debugging
            int currentRepeatLabel = repeat_label_count++;

            // Start of the repeat loop
            fprintf(mixFile, "REPEAT%d NOP\n", currentRepeatLabel);
            generate_mix_code(node->left);  // Generate code for the body of the repeat loop

            generate_expression(node->right);
            // Handle comparisons
            if (node->right->type == NODE_LT) {
                fprintf(mixFile, " JL ENDREPEAT%d\n", currentRepeatLabel);
            } else if (node->right->type == NODE_EQ) {
                fprintf(mixFile, " JE ENDREPEAT%d\n", currentRepeatLabel);
            }

            // Jump back to the start of the loop
            fprintf(mixFile, " JMP REPEAT%d\n", currentRepeatLabel);

            // End of the repeat loop
            fprintf(mixFile, "ENDREPEAT%d NOP\n", currentRepeatLabel);
            break;
        case NODE_READ:
            if (DEBUG) printf("NODE_READ\n");  // For debugging
            symbol *readSymbol = find_symbol(node->value, symbolList);
            if (readSymbol != NULL) {
                int input_buffer_address = 1000;  // Address for input
                int input_device = 19;             // Device number for input

                // Read input into the buffer
                fprintf(mixFile, " IN %d(%d)\n", input_buffer_address, input_device);

                // Check for input errors
                fprintf(mixFile, " JBUS *(%d)\n", input_device);

                // Load the input buffer into the accumulator
                fprintf(mixFile, " LDX %d(0:5)\n", input_buffer_address); // Load the input value

                // Convert to numeric format if necessary
                fprintf(mixFile, " NUM\n");

                // Store the converted value in the variable's memory location
                fprintf(mixFile, " STA %d(0:5)\n", readSymbol->memoryLocation);
            } else {
                fprintf(mixFile, "ERROR: Symbol %s not found in symbol list.\n", node->value);
            }
            break;
        case NODE_WRITE:
            if (DEBUG) printf("NODE_WRITE\n");  // For debugging
            symbol *writeSymbol = find_symbol(node->value, symbolList);
            
            if (writeSymbol != NULL) {
                int current_write = write_label_count++;

                fprintf(mixFile, " LDA %d(0:5)\n", writeSymbol->memoryLocation);

                fprintf(mixFile, " CHAR\n");

                fprintf(mixFile, " STA 1915(0:5)\n");

                fprintf(mixFile, " STX 1916(0:5)\n");

                fprintf(mixFile, " ENTX 45\n");
                fprintf(mixFile, " JAN KPO%d\n", current_write);
                fprintf(mixFile, " ENTX 44\n");
                fprintf(mixFile, "KPO%d NOP\n", current_write);

                fprintf(mixFile, " STX 1914(0:5)\n");

                fprintf(mixFile, " OUT 1914(2:3)\n");
            }
            break;
        case NODE_SEQ:
            if (DEBUG) printf("NODE_SEQ\n");  // For debugging
            generate_mix_code(node->left);  // Generate code for the first statement
            generate_mix_code(node->right); // Generate code for the second statement
            break;
        default:
            if (DEBUG) printf("(default in generation)\n");  // For debugging
            break;
    }
}