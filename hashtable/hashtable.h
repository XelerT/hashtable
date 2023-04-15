#ifndef HASHTABlE_H
#define HASHTABLE_H

#include "../include/text.h"
#include "../list/list.h"

enum hash_errors {
        BIG_HASH = 0xB164A4
};

int    call_choosed_hash_words (text_t *text, list_t *hashtable, char hash_mode);
int    hash_words              (text_t *text, list_t *hashtable, size_t (*get_word_hash)(word_t*));
size_t get_one_hash            (word_t *word);
size_t get_ascii_hash          (word_t *word);
size_t get_length_hash         (word_t *word);
size_t get_sum_ascii_hash      (word_t *word);
size_t get_rol_hash            (word_t *word);
size_t get_ror_hash            (word_t *word);
size_t get_my_hash             (word_t *word);

#endif /*HASHTABLE_H*/