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

        // test_all_hash_funcs("input.txt");

        FILE *input_file = fopen("input.txt", "r");
        text_t text = {};
        get_text(input_file, &text, "input.txt");
        fclose(input_file);

        delete_punctuations(&text);
        divide_text_on_words(&text);

        char hash_mode = 0;
        if (argc == 1)
                hash_mode = 0;
        else
                hash_mode = *argv[1];
        hashtable_t hashtable = {};
        hashtable_ctor(&hashtable, HASHTABLE_SIZE);

        hash_words(&text, &hashtable, hash_mode);

        FILE *file_to_find = fopen("to_find.txt", "r");
        if (!file_to_find) {
                log(1, "NULL FILE PTR.");
                text_dtor(&text);
                hashtable_dtor(&hashtable);
                log_dtor();

                return NULL_FILE_PTR;
        }

        text_t text_words2find = {};
        get_text(file_to_find, &text_words2find, "to_find.txt");
        fclose(file_to_find);

        delete_punctuations(&text_words2find);
        divide_text_on_words(&text_words2find);

        printf("Found %ld words\n", find_words_crc32(&hashtable, text_words2find.words, text_words2find.n_words));

        text_dtor(&text);
        text_dtor(&text_words2find);
        hashtable_dtor(&hashtable);

        log_dtor();
        return 0;
}
