#include <stdio.h>
#include <stdlib.h>

#include "include/config.h"
#include "list/list.h"
#include "include/text.h"
#include "hashtable/hashtable.h"
#include "list/list_dump.h"

int main (int argc, char *argv[])
{
        log_init("hashtable_logs.html");

        FILE *input_file = fopen("input.txt", "r");
        text_t text = {};
        get_text(input_file, &text, "input.txt");

        delete_punctuations(&text);
        divide_text_on_words(&text);

        list_t hashtable[HASHTABLE_SIZE] = {};

        for (int i = 0; i < HASHTABLE_SIZE; i++) {
                list_ctor(hashtable + i, 5, sizeof(data_t), sizeof(word_t*));
        }

        char hash_mode = 0;
        if (argc == 1)
                hash_mode = 0;
        else
                hash_mode = *argv[1];
        call_choosed_hash_words(&text, hashtable, hash_mode);
        write_lists_sizes_in_file(hashtable, "graphics/one.txt");

        list_graph(hashtable, "list_graph.dot", "list");

        text_dtor(&text);

        for (int i = 0; i < HASHTABLE_SIZE; i++)
                list_dtor(hashtable + i);
        log_dtor();
        return 0;
}
