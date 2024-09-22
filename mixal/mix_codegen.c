#include "mix_codegen.h"
#include "../syntax_tree/node_types.h"
#include "../syntax_tree/syntax_tree.h"
#include "../symbol_table/symbol_table.h"

#define DEBUG 0

FILE *mixFile;                // file pointer for mixal code
static int label_count = 0;  // Counter for generating unique labels
static int temp_count = 1;  // Counter for generating unique temporary variables
extern symbol *symbolList;  // External symbol list pointer


void createMixFile() {
    mixFile = fopen("mix.txt", "w");  // open log file for writing
    if (mixFile == NULL) {
        fprintf(stderr, "ERROR: Can not open mixal file.\n");  // print error message if file cannot be opened
    }
}

// Helper function to generate a new label
static char* new_label() {
    static char label[20];
    snprintf(label, sizeof(label), "L%d", label_count++);
    return label;
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

// Function to find the memory location of a symbol by its name or a constant
static int find_memory_location(const char *name) {
    if (is_numeric(name)) return atoi(name);  // If it is a constant return it as it is

    symbol *sym = symbolList;
    while (sym != NULL) {
        if (strcmp(sym->name, name) == 0) {
            return sym->memoryLocation;
        }
        sym = sym->next;
    }
    fprintf(stderr, "Error: Symbol %s not found.\n", name);
    return -1;  // Indicates an error
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
            fprintf(mixFile, " LDA %d\n", find_memory_location(node->value));  // Load the variable value into the accumulator using its memory address
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
                fprintf(mixFile, " CMPA =%d\n", node->right->value);
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
                fprintf(mixFile, " CMPA =%d\n", node->right->value);
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
            fprintf(mixFile, " STA %d\n", find_memory_location(node->left->value));  // Store result into the variable using memory address
            break;
        case NODE_IF:
            if (DEBUG) printf("NODE_IF\n");  // For debugging
            int currentLabel = label_count++;
    
            if ((node->type == NODE_IF) && (node->right->type != NODE_ELSE)) {
                // Generate code for the condition
                generate_expression(node->left);

                // Handle comparisons (expand as needed)
                if (node->left->type == NODE_LT) {
                    fprintf(mixFile, " JL THEN%d\n", currentLabel);
                } else if (node->left->type == NODE_EQ) {
                    fprintf(mixFile, " JE THEN%d\n", currentLabel);
                }
                
                // Unconditional jump to ENDIF
                fprintf(mixFile," JMP ENDIF%d\n", currentLabel);
                
                // THEN block
                fprintf(mixFile, "THEN%d\n", currentLabel);
                generate_mix_code(node->right); // Generate code for the 'then' part
                
                // End of if
                fprintf(mixFile, "ENDIF%d\n", currentLabel);

            } else if ((node->type == NODE_IF) && (node->right->type == NODE_ELSE)) {
                // Generate code for the condition
                generate_expression(node->left);

                // Handle comparisons
                if (node->left->type == NODE_LT) {
                    fprintf(mixFile, " JL THEN%d\n", currentLabel);
                } else if (node->left->type == NODE_EQ) {
                    fprintf(mixFile, " JE THEN%d\n", currentLabel);
                }
                
                // Unconditional jump to ELSE
                fprintf(mixFile, " JMP ELSE%d\n", currentLabel);
                
                // THEN block
                fprintf(mixFile, "THEN%d\n", currentLabel);
                generate_mix_code(node->right->left); // Generate code for the 'then' part
                fprintf(mixFile, " JMP ENDIF%d\n", currentLabel); // Jump to the end of if-else
                
                // ELSE block
                fprintf(mixFile, "ELSE%d\n", currentLabel);
                generate_mix_code(node->right->right); // Generate code for the 'else' part
                
                // End of if-else
                fprintf(mixFile, "ENDIF%d\n", currentLabel);
            }
            break;
        case NODE_REPEAT:
            if (DEBUG) printf("NODE_REPEAT\n");  // For debugging
            fprintf(mixFile, "L%d:\n", label_count++);  // Label for the start of the loop
            generate_mix_code(node->left);   // Generate code for the loop body
            generate_expression(node->right);  // Evaluate the condition
            fprintf(mixFile, "BRZ L%d\n", label_count - 1);  // Branch to end if condition is true
            break;
        case NODE_READ:
            if (DEBUG) printf("NODE_READ\n");  // For debugging
            fprintf(mixFile, "INP\n");  // Input value from user
            fprintf(mixFile, "STA %d\n", find_memory_location(node->value));  // Store the input into the variable using memory address
            break;
        case NODE_WRITE:
            if (DEBUG) printf("NODE_WRITE\n");  // For debugging
            fprintf(mixFile, "OUT %d\n", find_memory_location(node->value));  // Output value
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