/******************************************************************************
 * Junky plugin For GCC
 *
 * Inject random junk instructions during 'GIMPLE' compilation pass
 * Full of black magic so do not modify unless necessary
 * 
 * We DO NOT have to free any memory since plugin only runs at compile time
 * and lives for a very short time before compilation is done
 * 
 * This plugin supports GCC >= 4.8. Higher version might work as well.
 * 
 * xxxzzz@2022-10
 *****************************************************************************/

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

#define GCC_VER     "4.8"
#define MAX_NEW_FN  10
#define RANDOM_SZ   8192

// it won't work unless set to 1 so junky is always opensource
int plugin_is_GPL_compatible = 1;

static gimple_opt_pass optpass;
static uint8_t rand_buf[RANDOM_SZ];
static int junk_num;
static int rand_pos = 0;
static struct plugin_info junky_info = {
    .version = "1.0",
    .help = "-fplugin-arg-junky-junknum=<num>"
            "-fplugin-arg-junky-junkpfx=function_prefix1,function_prefix2..."
};

static struct plugin_gcc_version junky_ver = {
    .basever = GCC_VER,
};

typedef enum {
    JUNK_MATH,
    JUNK_ASSIGN,
    JUNK_NEW_FN,
    JUNK_JMP_FN
} junk_type_e;

static int xrand();
static tree make_junk_fn(void);
static char* make_junk_var_name(char* buff, uint8_t minlen, uint8_t maxlen);

vec<tree> junk_fns;
vec<tree> junk_vars;
vec<char*> target_prefix;

static void 
init_junky(void)
{
    printf("[junky] initializing _______\n");
    const int globals = 10 + xrand()%10;
    for (int i = 0; i < globals; i++) {
        char buf[64];
        memset(buf, 0, 64);
        tree t = add_new_static_var(integer_type_node);
        tree name = get_identifier(make_junk_var_name(buf, 5, 10));
        DECL_NAME(t) = name;
        TREE_STATIC(t) = 1;
        DECL_ARTIFICIAL(t) = 1;
        change_decl_assembler_name(t, name);
        junk_vars.safe_push(t);
    }
}

static int 
xrand() {
	if (!rand_pos) {
		int rfd = open("/dev/urandom", O_RDONLY);
        int rsz = (int)read(rfd, rand_buf, RANDOM_SZ);
		close(rfd);
		if (rsz < RANDOM_SZ) {
			printf("[junky] unable to generate random\n");
			exit(1);
			return 0;
		}
		printf("[junky] generated enough random data\n");	
	}
	int ret = 0;
	for (int i = 0; i < 4; i++)
		ret = (ret | rand_buf[rand_pos+i]<<(i*8));
	rand_pos += 4;
	if (rand_pos >= RANDOM_SZ) rand_pos = 0;
    return abs(ret);
}

static tree 
find_junk_fn(void)
{
    unsigned len = junk_fns.length();
    if (len == 0) {
        return make_junk_fn();
    }
    return junk_fns[xrand() % len];
}

static tree 
find_junk_var(void)
{
    unsigned len = junk_vars.length();
    if (len == 0) {
        return NULL_TREE;
    }
    return junk_vars[xrand() % len];
}

static bool 
is_junk_fn(tree decl)
{
    unsigned i;
    tree junk;

    FOR_EACH_VEC_ELT(junk_fns, i, junk) {
        if (junk == decl) return true;
    }
    return false;
}

static bool 
is_target(tree decl)
{
	char* pfx;
	unsigned i;
	FOR_EACH_VEC_ELT(target_prefix, i, pfx) {
		if (strncmp(get_name(decl), pfx, strlen(pfx)) == 0) return true;
	}
	return false;
}

static char* 
make_junk_var_name(char* buff, uint8_t minlen, uint8_t maxlen)
{
    int delta = xrand() % (maxlen - minlen);
    for (uint8_t i = 0; i < (minlen + delta); i++) {
        if (!i || (xrand() % 2)) {
            buff[i] = 'a' + (xrand() + junk_fns.length())%26;
        }
        else buff[i] = '0' + (xrand()%10);
    }
    return buff;
}

static gimple 
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

// inject some stmts into the function to make it looks 'normal'
static void 
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
    //tsi_next(&stmt_iter);
    tree return_stmt = build1(RETURN_EXPR, void_type_node, NULL_TREE);
    tsi_link_after(&stmt_iter, return_stmt, TSI_CONTINUE_LINKING);
    DECL_SAVED_TREE(fndecl) = bind_blk_expr;
    BLOCK_VARS(block) = BIND_EXPR_VARS(bind_blk_expr);
}

static tree 
make_junk_fn(void)
{
    char fnname[16] = {0};
    tree decl, resdecl, initial, typelst;
    sprintf(fnname, "sub_%X", xrand() % 10000);
    typelst = build_function_type_list(void_type_node, NULL_TREE);
    decl = build_fn_decl(fnname, typelst);
    SET_DECL_ASSEMBLER_NAME(decl, get_identifier(fnname));

    resdecl = build_decl(BUILTINS_LOCATION, RESULT_DECL, NULL_TREE, void_type_node);
    DECL_ARTIFICIAL(resdecl) = 1;
    DECL_CONTEXT(resdecl) = decl;
    DECL_RESULT(decl) = resdecl;
    
    initial = make_node(BLOCK);
    masquerade_junk_fn(decl, initial);
    TREE_USED(initial) = 1;
    DECL_INITIAL(decl) = initial;
    DECL_UNINLINABLE(decl) = 1;
    DECL_EXTERNAL(decl) = 0;
    DECL_PRESERVE_P(decl) = 1;

    TREE_USED(decl) = 1;
    TREE_PUBLIC(decl) = 1;
    TREE_STATIC(decl) = 1;
    DECL_ARTIFICIAL(decl) = 1;

    gimplify_function_tree(decl);
    cgraph_add_new_function(decl, false);

    junk_fns.safe_push(decl);
    printf("[junky] made junk fn: %s\n", fnname);
    return decl;
}


static void 
inject_junk_stmt(gimple_stmt_iterator *gsi, const int n, ...)
{
    const enum tree_code codes[] = {
        PLUS_EXPR, MINUS_EXPR, MULT_EXPR, LSHIFT_EXPR, RSHIFT_EXPR
    };
    const int n_codes = sizeof(codes) / sizeof(tree_code);
    tree node = NULL_TREE;
    gimple stmt;
    junk_type_e junk_type;

    va_list ptr;
    va_start(ptr, n);
    int choose = xrand() % n;
    for (int i = 0; i < n; i++) {
        junk_type_e tmp = (junk_type_e)va_arg(ptr, int);
        if (i == choose) {
            junk_type = tmp;
            break;
        }
    }
    va_end(ptr);
    if (junk_fns.length() > MAX_NEW_FN && junk_type == JUNK_NEW_FN) {
        junk_type = JUNK_JMP_FN;
    }

    switch (junk_type) {
        case JUNK_MATH:
            stmt = build_junk_math_stmt(gsi, codes[xrand()%n_codes]);
            break;
        case JUNK_NEW_FN:
            node = make_junk_fn();
        case JUNK_JMP_FN:
            if (node == NULL_TREE) {
                node = find_junk_fn();
            }
            stmt = gimple_build_call(node, 0);
            break;
        default:
            stmt = gimple_build_assign(find_junk_var(), build_int_cst(integer_type_node, xrand()));
    }
    gsi_insert_before(gsi, stmt, GSI_NEW_STMT);
    gsi_next(gsi);
    return;
}


static unsigned int 
plugin_exec(void)
{
    unsigned seq = 0;
    basic_block bb;
    static bool initted = false;

    if (!initted) {
        init_junky();
        initted = true;
    }
    if (!is_target(cfun->decl)) return 0;
    if (is_junk_fn(cfun->decl)) {
        printf("[junky] weird! we got a junk fn, which should never happen.\n");
    }

    FOR_EACH_BB_FN(bb, cfun) {
        if (!junk_num) {
            printf("[junky] max junk reached !\n");
            return 0;
        }
        printf("[junky] junking: %s, blk: %d, remain: %d\n",  get_name(cfun->decl), seq++, junk_num);
        for (gimple_stmt_iterator gsi = gsi_start_bb(bb); !gsi_end_p(gsi)&&(junk_num>0); gsi_next(&gsi)) 
        {
            inject_junk_stmt(&gsi, 2, JUNK_NEW_FN, JUNK_JMP_FN);
            junk_num--;
            int n_junk_stmt = xrand()%20 + 1;
            for (int njunk = 0; junk_num > 0 && njunk < n_junk_stmt; njunk++) {
                inject_junk_stmt(&gsi, 4, 
                    JUNK_ASSIGN, JUNK_NEW_FN, JUNK_MATH, JUNK_JMP_FN
                );
                junk_num--;
            }
        }
    }
    return 0;
}

int 
plugin_init(struct plugin_name_args *info, struct plugin_gcc_version *ver)
{
    const char* plugin_name = "junky";
    struct register_pass_info pass;
    if (strncmp(ver->basever, junky_ver.basever, strlen(GCC_VER))) return -1;

    printf("[junky] -------------------------------\n");
    printf("[junky] JUNKY activated!\n");
    printf("[junky] -------------------------------\n");
	optpass.pass.type = GIMPLE_PASS;
	optpass.pass.name = plugin_name;
	optpass.pass.execute = plugin_exec;
    pass.pass = &(optpass.pass);
    pass.reference_pass_name = "ssa";
    pass.ref_pass_instance_number = 1;
    pass.pos_op = PASS_POS_INSERT_AFTER;

    register_callback(plugin_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass);
    register_callback(plugin_name, PLUGIN_INFO, NULL, &junky_info);

    for (int i = 0; i < info->argc; i++) {
        if (strncmp("junknum", info->argv[i].key, 7) == 0) {
			junk_num = atoi(info->argv[i].value);
			continue;
		}
		if (strncmp("junkpfx", info->argv[i].key, 7) == 0) {
			printf("[junky] user specified target prefix:%s\n", info->argv[i].value);
			char* tk = strtok(info->argv[i].value, ",");
			while (tk) {
				size_t len = strlen(tk);
				char* buf = new char[len+1];
				memset(buf, 0, len+1);
				strncpy(buf, tk, len);
				target_prefix.safe_push(buf);
				tk = strtok(NULL, ",");
			}
			
		}
    }
	
	if (target_prefix.length() == 0) {
		printf("[junky] no target prefix\n");
		return 1;
	}
    if (junk_num <= 0 || junk_num > 65536) {
        printf("[junky] invalid junk number\n");
		return 1;
    }
    printf("[junky] max junk allowed: %d\n", junk_num);
    return 0;
}

