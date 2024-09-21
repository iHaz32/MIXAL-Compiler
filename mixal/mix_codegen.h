#ifndef MIX_CODEGEN_H
#define MIX_CODEGEN_H

#include "../syntax_tree/node_types.h"
#include "../syntax_tree/syntax_tree.h"
#include "../symbol_table/symbol_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

void generate_mix_code(TreeNode *node);  // generates mixal code from the ast

#endif
