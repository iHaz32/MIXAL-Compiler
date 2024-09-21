#include "mix_codegen.h"
#include "../syntax_tree/node_types.h"
#include "../syntax_tree/syntax_tree.h"
#include "../symbol_table/symbol_table.h"

#define DEBUG 0

static int label_count = 0;  // Counter for generating unique labels
static int temp_count = 1;  // Counter for generating unique temporary variables
extern symbol *symbolList;  // External symbol list pointer

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
            printf("ENTA %s\n", node->value);  // Load the number into the accumulator
            break;
        case NODE_ID:
            if (DEBUG) printf("NODE_ID\n");  // For debugging
            printf("LDA %d\n", find_memory_location(node->value));  // Load the variable value into the accumulator using its memory address
            break;
       case NODE_ADD:
            if (DEBUG) printf("NODE_ADD\n");  // For debugging
            int addTemp = temp_count;
            // Generate code for left operand
            generate_expression(node->left);
            // Store result temporarily
            printf("STA TEMP%d\n", addTemp);
            temp_count++;
            // Generate code for right operand
            generate_expression(node->right);
            // Load temporary result and add
            printf("ADD TEMP%d\n", addTemp);  // Load previous temp
            break;
        case NODE_SUBTRACT:
            if (DEBUG) printf("NODE_SUBTRACT\n");  // For debugging
            int subTemp = temp_count;
            // Generate code for left operand
            generate_expression(node->left);
            // Store result temporarily
            printf("STA TEMP%d\n", subTemp);
            temp_count++;
            // Generate code for right operand
            generate_expression(node->right);
            // Load temporary result and subtract
            printf("SUB TEMP%d\n", subTemp);  // Load previous temp

            // A = 0-A
            printf("STA OPPTEMP\n");
            printf("ENTA 0\n");
            printf("SUB OPPTEMP\n");
            break;
        case NODE_MULTIPLY:
            if (DEBUG) printf("NODE_MULTIPLY\n");  // For debugging
            int mulTemp = temp_count;
            // Generate code for left operand
            generate_expression(node->left);
            // Store result temporarily
            printf("STA TEMP%d\n", mulTemp);
            temp_count++;
            // Generate code for right operand
            generate_expression(node->right);
            // Load temporary result and multiply
            printf("MUL TEMP%d\n", mulTemp);  // Load previous temp
            printf("STX TEMP%d\n", mulTemp);
            printf("LDA TEMP%d\n", mulTemp);
            printf("ENTX 0\n");
            break;
        case NODE_DIVIDE:
            if (DEBUG) printf("NODE_DIVIDE\n");  // For debugging
            int divTemp = temp_count;
            // Generate code for left operand
            generate_expression(node->left);
            // Store result temporarily
            printf("STA TEMP%d\n", divTemp);
            temp_count++;
            // Generate code for right operand
            generate_expression(node->right);
            
            // A = 1/A (Swap A and TEMP)
            printf("STA SWAPTEMP\n");  // swaptemp = A
            printf("LDX SWAPTEMP\n");  // X = swaptemp => X = A
            printf("LDA TEMP%d\n", divTemp);  // A = temp
            printf("STX TEMP%d\n", divTemp);  // temp = X
            printf("STA SWAPTEMP\n");
            printf("LDX SWAPTEMP\n");
            printf("ENTA 0\n");
            printf("DIV TEMP%d\n", divTemp);

            break;
        case NODE_LT:
            if (DEBUG) printf("NODE_LT\n");  // For debugging
            generate_expression(node->left);   // Evaluate left operand
            printf("STA TEMP\n");              // Store result temporarily
            generate_expression(node->right);  // Evaluate right operand
            printf("LDA TEMP\n");              // Load result from TEMP
            printf("CMPA %d\n", find_memory_location(node->right->value));  // Compare with right operand value
            printf("BRN L%d\n", label_count);  // Branch to Lx if less than (negative result)
            printf("LDA #0\n");                // Load 0 into accumulator (false)
            printf("BR L%d\n", label_count + 1);  // Branch to Lx+1
            printf("L%d: LDA #1\n", label_count);  // Load 1 into accumulator (true)
            printf("L%d:\n", label_count + 1);  // Label Lx+1
            label_count += 2;
            break;
        case NODE_EQ:
            if (DEBUG) printf("NODE_EQ\n");  // For debugging
            generate_expression(node->left);   // Evaluate left operand
            printf("STA TEMP\n");              // Store result temporarily
            generate_expression(node->right);  // Evaluate right operand
            printf("LDA TEMP\n");              // Load result from TEMP
            printf("CMPA %d\n", find_memory_location(node->right->value));  // Compare with right operand value
            printf("BRZ L%d\n", label_count);  // Branch to Lx if zero (equal)
            printf("LDA #0\n");                // Load 0 into accumulator (false)
            printf("BR L%d\n", label_count + 1);  // Branch to Lx+1
            printf("L%d: LDA #1\n", label_count);  // Load 1 into accumulator (true)
            printf("L%d:\n", label_count + 1);  // Label Lx+1
            label_count += 2;
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
            printf("\n\nMIX:\n");
            generate_mix_code(node->left);  // Generate code for the program body
            break;
        case NODE_ASSIGNMENT:
            if (DEBUG) printf("NODE_ASSIGNMENT\n");  // For debugging
            generate_expression(node->right);  // Generate code for the expression
            printf("STA %d\n", find_memory_location(node->left->value));  // Store result into the variable using memory address
            break;
        case NODE_IF:
            if (DEBUG) printf("NODE_IF\n");  // For debugging
            generate_expression(node->left);  // Evaluate the condition
            printf("BRZ L%d\n", label_count);  // Branch to Lx if condition is false
            generate_mix_code(node->right);   // Generate code for the true branch
            printf("L%d:\n", label_count); // Label for the end of true branch
            label_count++;  // Increment label count for the next section
            break;
        case NODE_ELSE:
            if (DEBUG) printf("NODE_ELSE\n");  // For debugging
            printf("BR L%d\n", label_count);  // Branch to skip the else part
            printf("L%d:\n", label_count - 1); // Label for the else part
            generate_mix_code(node->left);  // Generate code for the false branch
            printf("L%d:\n", label_count);  // Label for the end of else part
            label_count++;  // Increment label count for the next section
            break;
        case NODE_REPEAT:
            if (DEBUG) printf("NODE_REPEAT\n");  // For debugging
            printf("L%d:\n", label_count++);  // Label for the start of the loop
            generate_mix_code(node->left);   // Generate code for the loop body
            generate_expression(node->right);  // Evaluate the condition
            printf("BRZ L%d\n", label_count - 1);  // Branch to end if condition is true
            break;
        case NODE_READ:
            if (DEBUG) printf("NODE_READ\n");  // For debugging
            printf("INP\n");  // Input value from user
            printf("STA %d\n", find_memory_location(node->value));  // Store the input into the variable using memory address
            break;
        case NODE_WRITE:
            if (DEBUG) printf("NODE_WRITE\n");  // For debugging
            printf("OUT %d\n", find_memory_location(node->value));  // Output value
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