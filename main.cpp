#include <stdio.h>
#include "include/config.h"
#include "list/list.h"
#include "include/text.h"

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

        text_dtor(&text);

        for (int i = 0; i < HASHTABLE_SIZE; i++)
                list_dtor(hashtable + i);
        log_dtor();
        return 0;
}
