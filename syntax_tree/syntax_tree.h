#ifndef SYNTAX_TREE_H
#define SYNTAX_TREE_H

#include "node_types.h"
#include "../symbol_table/symbol_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct TreeNode {
    NodeType type;          // type of the node (now an enum)
    struct TreeNode *left;  // pointer to the left child
    struct TreeNode *right; // pointer to the right child
    char* value;            // value of the node (if it's a number or identifier)
} TreeNode;

TreeNode* create_node(NodeType type, TreeNode *left, TreeNode *right, char* value);  // create a new tree node
int decide_expression(TreeNode *node, symbol *symbolList);  // evaluate the expression represented by the tree
void print_tree(FILE *logFile, TreeNode *node, int level);  // print the tree structure to the log file

#endif