#include <string.h>
#include <stdlib.h>
#include <assert.h>

 #include "list.h"
#include "../include/text.h"

static size_t fill_free (list_t *list, size_t position, size_t sizeof_elem);

int list_ctor (list_t *list, size_t capacity, size_t sizeof_data, size_t sizeof_elem)
{
        assert(list);

        list->capacity = capacity;

        list->data = (data_t*) calloc(list->capacity, sizeof_data);
        if (!list->data)
                return NULL_CALLOC;

        list->free = list->data + 1;
        list->size = 0;
        fill_free(list, 1, sizeof_elem);

        return 0;
}

static size_t fill_free (list_t *list, size_t position, size_t sizeof_elem)
{
        assert(list);

        size_t i = 0;
        size_t free_elem = FREE_ELEM;

        for (i = position; i < list->capacity - 1; i++) {
                memcpy(&list->data[i].data, &free_elem, sizeof_elem);
                list->data[i].next = i + 1;
                list->data[i].prev = i - 1;
        }
        memcpy(&list->data[i].data, &free_elem, sizeof_elem);
        list->data[i].next = 0;
        list->data[i].prev = list->capacity - 2;

        return i - 1;
}

int list_insert (list_t *list, void *val, size_t position, size_t sizeof_elem)
{
        assert(list);

        if (list->size * 2 >= list->capacity)
                if (list_resize(list, 2, sizeof_elem))
                        return RESIZE_ERR;

        data_t *data = list->data;
        data_t *free = list->free;
        size_t new_free = free->next;

        memcpy(&free->data, val, sizeof_elem);

        free->next = data[position].next;
        free->prev = position;

        data[position].next = free - data;
        data[new_free].prev = free - data;

        list->free       = data + new_free;
        list->free->prev = free - data;
        list->size++;

        return 0;
}

int list_resize (list_t *list, size_t coeff, size_t sizeof_elem)
{
        assert(list);

        data_t *data = (data_t*) realloc(list->data, list->capacity * coeff * sizeof(data_t));
        if (!data) {
                log_error(1, "RESIZE RETURN NULL.");
                return RESIZE_ERR;
        }

        list->free = list->free - list->data + data;
        list->data = data;
        list->capacity = list->capacity * coeff;

        fill_free(list, list->free - list->data, sizeof_elem);

        return 0;
}

elem_t list_delete (list_t *list, size_t position, size_t sizeof_elem)
{
        assert(list && "Null list");

        data_t *data = list->data;
        data_t *free = list->free;
        elem_t deleted = 0;
        elem_t free_elem = FREE_ELEM;

        if (position < list->capacity) {
                memcpy(&deleted, &data[position].data, sizeof_elem);

                memcpy(&data[position].data, &free_elem, sizeof_elem);
                data[data[position].next].prev = data[position].prev;
                data[data[position].prev].next = data[position].next;

                data[position].next = free - data;
                data[position].prev = free->prev;

                free->prev = position;
                list->free = data + position;

                return deleted;
        } else {
                fprintf(stderr, "Position is greater than list capacity. Func: %s\n", __PRETTY_FUNCTION__);
        }

        return 0;
}

int list_dtor (list_t *list)
{
        if (list && list->data) {
                free(list->data);
                list->data = nullptr;
                return 0;
        }

        return NULL_LIST_PTR;
}

int list_linearize (list_t *list, size_t sizeof_elem)
{
        data_t *new_data_ptr = (data_t*) calloc(list->capacity, sizeof(data_t));
        if (new_data_ptr == nullptr)
                return NULL_CALLOC;

        size_t i = 0;
        size_t next_elem = list->data[0].next;
        for (i = 1; i < list->size + 1; i++) {
                new_data_ptr[i].data = list->data[next_elem].data;
                next_elem = list->data[next_elem].next;
                new_data_ptr[i].prev = i - 1;
                new_data_ptr[i - 1].next = i;
        }

        new_data_ptr[i].data = list->data[next_elem].data;
        new_data_ptr[i].next = 0;
        new_data_ptr->prev = i - 1;

        free(list->data);
        list->data = new_data_ptr;
        fill_free(list, i, sizeof_elem);

        return 0;
}
