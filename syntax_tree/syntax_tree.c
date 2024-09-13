#include "syntax_tree.h"

TreeNode* create_node(NodeType type, TreeNode *left, TreeNode *right, char* value) {
    TreeNode *newNode = (TreeNode *)malloc(sizeof(TreeNode));  // allocate memory for a new node
    newNode->type = type;  // use enum for node type
    newNode->left = left;  // set left child
    newNode->right = right;  // set right child
    newNode->value = value ? strdup(value) : NULL;  // copy value string or set to NULL
    return newNode;  // return the newly created node
}

int decide_expression(TreeNode *node, symbol *symbolList) {
    if (node == NULL) return 0;  // base case: if node is NULL, return 0

    switch (node->type) {
        case NODE_NUMBER:
            return atoi(node->value);  // convert value to integer

        case NODE_ID: {
            symbol *symbol = find_symbol(node->value, symbolList);  // find symbol by name
            return symbol ? symbol->value : 0;  // return symbol value or 0 if not found
        }

        case NODE_LT:
            return decide_expression(node->left, symbolList) < decide_expression(node->right, symbolList);

        case NODE_EQ:
            return decide_expression(node->left, symbolList) == decide_expression(node->right, symbolList);

        case NODE_ADD:
            return decide_expression(node->left, symbolList) + decide_expression(node->right, symbolList);

        case NODE_SUBTRACT:
            return decide_expression(node->left, symbolList) - decide_expression(node->right, symbolList);

        case NODE_MULTIPLY:
            return decide_expression(node->left, symbolList) * decide_expression(node->right, symbolList);

        case NODE_DIVIDE:
            return decide_expression(node->left, symbolList) / decide_expression(node->right, symbolList);

        default:
            return 0;  // return 0 for unknown node types
    }
}

void print_tree(FILE *logFile, TreeNode *node, int level) {
    if (node == NULL) return;  // base case: if node is NULL, return

    if (level == 0) fprintf(logFile, "\nSYNTAX TREE\n-----------\n");  // print header for the tree

    // Indent based on the level of the tree
    for (int i = 0; i < level; i++) {
        fprintf(logFile, "  ");  // indent for each level
    }

    // Print the node type based on enum
    switch (node->type) {
        case NODE_PROGRAM:
            fprintf(logFile, "PROGRAM");
            break;
        case NODE_ASSIGNMENT:
            fprintf(logFile, "AS (:=)");
            break;
        case NODE_IF:
            fprintf(logFile, "IF");
            break;
        case NODE_ELSE:
            fprintf(logFile, "ELSE");
            break;
        case NODE_REPEAT:
            fprintf(logFile, "REPEAT");
            break;
        case NODE_READ:
            fprintf(logFile, "READ");
            break;
        case NODE_WRITE:
            fprintf(logFile, "WRITE");
            break;
        case NODE_SEQ:
            fprintf(logFile, "SEQ");
            break;
        case NODE_LT:
            fprintf(logFile, "LT (<)");
            break;
        case NODE_EQ:
            fprintf(logFile, "EQ (=)");
            break;
        case NODE_ADD:
            fprintf(logFile, "ADD (+)");
            break;
        case NODE_SUBTRACT:
            fprintf(logFile, "SUB (-)");
            break;
        case NODE_MULTIPLY:
            fprintf(logFile, "MUL (*)");
            break;
        case NODE_DIVIDE:
            fprintf(logFile, "DIV (/)");
            break;
        case NODE_NUMBER:
            fprintf(logFile, "NUMBER");
            break;
        case NODE_ID:
            fprintf(logFile, "ID");
            break;
        default:
            fprintf(logFile, "unknown");
    }

    if (node->value) {
        fprintf(logFile, " (%s)", node->value);  // print node value if available
    }

    fprintf(logFile, "\n");

    // recursively print left and right subtrees
    print_tree(logFile, node->left, level + 1);
    print_tree(logFile, node->right, level + 1);
}