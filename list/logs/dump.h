#ifndef GRAPH_DUMP_H
#define GRAPH_DUMP_H

#include "..\list.h"
#include "log.h"

enum list_errors {
        NULL_LIST_PTR = 1,
        NULL_DATA_PTR = 2,
        NULL_FREE_PTR = 4,
        NULL_CAPACITY = 8
};

void list_graph_dump (list_t *list, FILE *log_file);
int list_dump (list_t *list, const char *file_name, const char *func, const int line, const char *list_name, FILE *log, const char *img);
int list_error (list_t *list);

#endif /*GRAPH_DUMP*/
