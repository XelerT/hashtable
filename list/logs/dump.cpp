#include "dump.h"

int list_dump (list_t *list, const char *file_name, const char *func, const int line,
               const char *list_name, FILE *log, const char *img)
{
        assert(list);
        assert(log);

        static char *list_errors[] = {
                "List pointer is null.",
                "Data pointer in list is null.",
                "Pointer on the free element in list is null.",
                "List has zero capacity."
        };

        int errors = list_error(list);
        fprintf(log, "<pre>\n");

        for (int i = 0; (errors << i) != 0; i++) {
                switch (errors & ~(~0 << i + 1)) {                                                                      //1111 1111
                case NULL_LIST_PTR:                                                                                     //1111 1100
                        fprintf(log, "<p style=\"color:red;\">%s<\/p></pre>\n", list_errors[i]);                            //0000 0011
                        return NULL_LIST_PTR;                                                                          //&0000 1010
                case NULL_DATA_PTR:                                                                                     //0000 0010
                        fprintf(log, "<p style=\"color:red;\">%s<\/p></pre>\n", list_errors[i]);
                        return NULL_DATA_PTR;
                case NULL_FREE_PTR:
                        fprintf(log, "Name of the file: %s\nName of the function: %s \
                                \nLine: %d\nName of the list: %s\n", file_name, func, line, list_name);
                        fprintf(log, "    prev | data | next\n");
                        for (size_t i = 0; i < list->capacity; i++) {
                                fprintf(log, "%lld) %5lld %5d %5lld\n", i, list->data[i].prev, list->data[i].data, list->data[i].next);
                        }
                        paste_img(log, img);

                        fprintf(log, "<p style=\"color:red;\">%s<\/p></pre>\n", list_errors[i]);
                        return NULL_FREE_PTR;
                case NULL_CAPACITY:
                        fprintf(log, "<p style=\"color:red;\">%s<\/p>\n", list_errors[i]);
                        break;
                }
        }

        fprintf(log, "</pre>\n");

        return errors;
}

int list_error (list_t *list)
{
        int errors = 0;
        if (list == nullptr) {
                errors |= NULL_LIST_PTR;
        } else {
                if (list->data == nullptr)
                        errors |= NULL_DATA_PTR;
                if (list->free == nullptr)
                        errors |= NULL_FREE_PTR;
                if (list->capacity == 0)
                        errors |= NULL_CAPACITY;
        }

        return errors;
}

void list_graph_dump (list_t *list, FILE *log_file)
{
        assert(list);
        assert(log_file);

        fprintf(log_file, "digraph G {                                                                    \
	                        \ngraph [dpi = 300];                                                      \
	                        \nsplines = ortho                                                         \
                                \n                                                                        \
	                        \nedge[penwidth = 5, color = \"black\"];                                  \
	                        \nnode[shape = rectangle, style = rounded, fixedsize = true,              \
	                        \nheight = 3, width = 2, fontsize = 30, penwidth = 5];                    \
                          ");

        fprintf(log_file, "\n{rank = same;\n");
        for (size_t i = 0; i < list->size; i++) {
                fprintf(log_file, "node%lld [label = \"indx %lld \n prev %lld \n %d \n next  %lld\"];\n",
                        i, i, list->data[i].prev, list->data[i].data, list->data[i].next);
        }

        fprintf(log_file, "\nnode[shape = \"rectangle\", style = \"rounded, filled\",                 \
		           \nheight = 3, width = 2, fixedsize = true,                                 \
		           \nfillcolor = \"#00b899\",                                                 \
		           \nfontsize = 30,                                                           \
		           \npenwidth = 5, color = \"black\"                                          \
		           \n]\n");
        for (size_t i = list->size; i < list->capacity; i++) {
                fprintf(log_file, "node%lld [label = \"indx %lld \n prev %lld \n %d \n next %lld\"];\n",
                        i, i, list->data[i].prev, list->data[i].data, list->data[i].next);
        }
        fprintf(log_file, "}\n");

        for (size_t i = 0; i < list->capacity; i++) {
                fprintf(log_file, "node%lld -> node%lld [width = 5, style = invis];\n", i, list->data[i].next);
        }
        for (size_t i = 0; i < list->capacity; i++) {           //create link between next elements
                fprintf(log_file, "node%lld -> node%lld [color = red];\n", i, list->data[i].next);
        }

        for (size_t i = 0; i < list->capacity; i++) {           //create link between previous elements
                fprintf(log_file, "node%lld -> node%lld [color = black];\n", i, list->data[i].prev);
        }

        fprintf(log_file, "\n}");
}
