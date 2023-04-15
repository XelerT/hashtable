#ifndef LIST_DUMP_H
#define LIST_DUMP_H

#include "list.h"

int  list_dump      (list_t *list);
int  list_error     (list_t *list);
void list_graph     (list_t *list, const char *dot_file_name, const char* png_name);
void print_gv_nodes (list_t *hashtable);
void tie_gv_nodes   (list_t *hashtable);

#endif /*LIST_DUMP_H*/
