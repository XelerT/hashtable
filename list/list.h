#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include <assert.h>

#include "../include/utils.h"
#include "../include/config.h"

enum list {
        FREE_ELEM = -1
};

enum errors {
        NULL_LIST_PTR  =  0xBADDADA,
        NULL_NEXT_PTR  = -2,
        NULL_PREV_PTR  = -3,
        NO_SIZE        = -4,
        INSERT_FAIL    = 0xBADFEL,
        RESIZE_ERR     = -5,
};

struct data_t {
        elem_t data = 0;
        size_t next = 0;
        size_t prev = 0;
};

struct list_t {
        data_t *data    = {};
        data_t *free    = nullptr;
        size_t capacity = 0;
        size_t size     = 0;
};

int list_ctor       (list_t *list, size_t capacity, size_t sizeof_data, size_t sizeof_elem);
int list_insert     (list_t *list, unsigned char *val, size_t position, size_t sizeof_elem);
elem_t list_delete  (list_t *list, size_t position, size_t sizeof_elem);
int list_dtor       (list_t *list);
int list_resize     (list_t *list, size_t coeff);
int list_linearize  (list_t *list, size_t sizeof_elem);

#endif /*LIST_H*/
