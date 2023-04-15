#include <stdio.h>
#include <assert.h>
#include "list_dump.h"
#include "../include/text.h"

int list_dump (list_t *list)
{
        const char *list_errors[] = {
                "Listpointer is null.",
                "Node pointer in list is null.",
        };

        FILE *dump_file = fopen("dump.html", "w");

        int errors = list_error(list);
        fprintf(dump_file, "<pre>\n");

        if (!errors) {
                fprintf(dump_file, "</pre>\n");
                return 0;
        }

        for (int i = 0; (errors << i) != 0; i++) {
                switch (errors & ~(~0u << (i + 1))) {                                                                      //1111 1111
                case NULL_LIST_PTR:                                                                                     //1111 1100
                        fprintf(dump_file, "<p style=\"color:red;\">%s</p></pre>\n", list_errors[i]);                  //0000 0011
                        return NULL_LIST_PTR;                                                                          //&0000 1010
                default:
                        fprintf(stderr, "SMTH WENT WRONG: FILE %s, LINE: %d", __FILE__, __LINE__);
                }
        }

        fprintf(dump_file, "</pre>\n");

        return errors;
}

int list_error (list_t *list)
{
        int errors = 0;
        if (list == nullptr)
                errors |= NULL_LIST_PTR;

        return errors;
}

//----------------------------------------------------------------------------------------------------------------------------

static const char digraph_atr_sample[] = "digraph %s {                                  \
                                                rankdir=\"LR\";                         \
                                                \ngraph [dpi = %d];                     \
                                                \nsplines = %s;                         \
                                                \n";

static const char node_atr_sample[] = "\nnode%p[shape = \"%s\", style = \"%s, filled\",                                            \
		                                \nheight = %d, width = %d, fixedsize = %s,                               \
		                                \nfillcolor = \"%s\",                                                    \
		                                \nfontsize = %d,                                                         \
		                                \npenwidth = %d                                                          \
		                                \n];\n";

static const char edge_atr_sample[] = "\nedge[penwidth = %d, color = \"%s\"];";

static const char node_sample_str[]  = "node%p [label = \"%s\"];\n";
static const char node_sample_dbl[]  = "node%p [label = \"%lg\"];\n";
static const char node_sample_c[]    = "node%p [label = \"%c\"];\n";
static const char node_sample_dec[]  = "node%p [shape = record label = \"{ %d | sub_type: %d}\"];\n";

static const char nodes_tie_atr[] = "node%p -> node%p [color = %s arrowsize = %d penwidth = %d];\n";


static FILE *dot_file = nullptr;
#define gv_print(form,...) fprintf(dot_file, form, __VA_ARGS__)

void list_graph (list_t *hashtable, const char *dot_file_name, const char* png_name)
{
        if (!hashtable) {
                fprintf(stderr, "Hashtable pointer is null. \nIn File: \n\t%s \nLine: \n\t%d.\n", __FILE__, __LINE__);
                return;
        }
        /*FILE * */dot_file = fopen(dot_file_name, "w");
        if (!dot_file) {
                fprintf(stderr, "Dot file pointer is null. \nIn File: \n\t%s \nLine: \n\t%d.\n", __FILE__, __LINE__);
                return;
        }
        digraph_t G = {};

        gv_print(digraph_atr_sample, "G", G.dpi, G.splines);    /*graph_name, dpi, splines, edge_sample, node_sample*/

        for (size_t n = 0; n < HASHTABLE_SIZE; n++) {
                print_gv_nodes(hashtable + n);
                tie_gv_nodes(hashtable + n);
        }

        fprintf(dot_file, "\n}");
        fclose(dot_file);
        log_pic(3, "<img src=\"%s\" width=\"460\" alt=\"%s\">", png_name, png_name);
}

void print_gv_nodes (list_t *list)
{
        assert(list);

        for (size_t i = 1; i < list->size; i++) {
                // printf("list = %ld; elem = %ld\n", n, i);
                gv_print(node_atr_sample, list->data + i, list->data_atr.shape, list->data_atr.style, list->data_atr.height,
                list->data_atr.width, list->data_atr.fixedsize, list->data_atr.fillcolor,list->data_atr.fontsize,
                list->data_atr.penwidth);

                gv_print(node_sample_str, &list->data[i], (char*) list->data[i].data);
                // printf("string %s\n", (char*) ((word_t*)(list->data[i].data)));
        }
}

void tie_gv_nodes (list_t *list)
{
        assert(list);

        for (size_t i = 1; i < list->size; i++) {
                gv_print(nodes_tie_atr, list->data + i, list->data + list->data[i].next, "black", list->data_atr.arrowsize, list->data_atr.penwidth);
                gv_print(nodes_tie_atr, list->data + i, list->data + list->data[i].prev, "red", list->data_atr.arrowsize, list->data_atr.penwidth);
        }
}

