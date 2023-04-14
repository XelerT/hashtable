#ifndef UTILS_H
#define UTILS_H

const int MAX_NAME_LENGTH = 64;
const int DEFAULT_N_VARS  = 16;
const int DEFAULT_N_FUNCS = 16;
const int DEFAULT_N_TABLS =  4;

struct graph_node_atr_t {
        const char *shape     = "rectangle";
        const char *style     =   "rounded";
        const char *fixedsize =     "false";
        const char *fillcolor =   "#00b899";
        int height   =  3;
        int width    =  2;
        int fontsize = 30;
        int penwidth =  5;
};

struct edge_t {
        int penwidth      =       5;
        const char *color = "black";
};

struct digraph_t {
        int dpi             =     300;
        const char *splines = "ortho";
};

enum main_errors {
        NULL_CALLOC   = 0xCA110C,
        NULL_FILE_PTR = 0xF11E,
        ALLOC_ERR     = 0xBADA110C,
        REALLOC_ERR   = 0xBADA110C,
        LEX_ERROR     = 0xBAD1E4,
        NULL_SPRINTF  = 0xBAD366,
        TOO_LONG      = 0x100104
};

void my_rol (unsigned char *val, size_t sizeof_val, size_t max_iter);

#endif /*UTILS_H*/
