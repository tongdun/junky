/* common header for junky */
#pragma once

#include <stdio.h>
#include <gcc-plugin.h>
#include <coretypes.h>
#include <tree.h>
#include <tree-pass.h>
#include <tree-iterator.h>
#include <tree-flow.h>
#include <function.h>
#include <cgraph.h>
#include <gimple.h>
#include <vec.h>

/* find a global variable for update/read */
tree 
find_junk_var(void);

/* generate a genuine random integer */
int 
xrand();

/* output debug info and can be disabled in runtime */
void 
debug_info(const char *fmt, ...);

/* inject some instructions into our junk function */
void 
masquerade_junk_fn(tree fndecl, tree block);

/* build some junk instructions which will be combined in user's function */
gimple 
build_junk_math_stmt(gimple_stmt_iterator *gsi, enum tree_code code);


