/* junky extensible module */

#include "common.h"

// you may add more features here

gimple 
build_junk_math_stmt(gimple_stmt_iterator *gsi, enum tree_code code)
{
    int rand_max = 1<<24;
    if (code == LSHIFT_EXPR || code == RSHIFT_EXPR) {
        rand_max = 1<<3;//don't shift too much
    }

    gimple g = gimple_build_assign_with_ops (code,
        make_ssa_name(integer_type_node, NULL),
        find_junk_var(),
        build_int_cst(integer_type_node, xrand()%rand_max)
    );
    gsi_insert_before(gsi, g, GSI_NEW_STMT);
    gsi_next(gsi);
    return gimple_build_assign(find_junk_var(), gimple_assign_lhs(g));
}

// make the function looks 'normal'
void 
masquerade_junk_fn(tree fndecl, tree block)
{
    tree stmt_list = alloc_stmt_list();
    tree_stmt_iterator stmt_iter = tsi_start(stmt_list);
    tree bind_blk_expr = build3(BIND_EXPR, void_type_node, NULL, stmt_list, block);
    tree var1 = find_junk_var();
    tree var1_inc_expr = build2(xrand()%2 ? PREINCREMENT_EXPR : POSTDECREMENT_EXPR, 
        integer_type_node, var1, build_int_cst(integer_type_node, xrand()%(1<<16)));

    const enum tree_code ops[] = { MINUS_EXPR, PLUS_EXPR };
    tree var2_op_expr = build2(ops[xrand()%2], 
        integer_type_node, build_int_cst(integer_type_node, xrand()%(1<<24)), var1
    );
    if (xrand()%2) {
        tree modify_var2 = build2(
            MODIFY_EXPR, integer_type_node, find_junk_var(), var2_op_expr
        );
        tsi_link_after(&stmt_iter, modify_var2, TSI_CONTINUE_LINKING);
    }

    tree modify_global = build2(
        MODIFY_EXPR, integer_type_node, find_junk_var(), var1_inc_expr
    );
    tsi_link_after(&stmt_iter, modify_global, TSI_CONTINUE_LINKING);
    tree return_stmt = build1(RETURN_EXPR, void_type_node, NULL_TREE);
    tsi_link_after(&stmt_iter, return_stmt, TSI_CONTINUE_LINKING);
    DECL_SAVED_TREE(fndecl) = bind_blk_expr;
    BLOCK_VARS(block) = BIND_EXPR_VARS(bind_blk_expr);
}
