#ifndef HASHTABlE_H
#define HASHTABLE_H

#include "../include/text.h"
#include "../list/list.h"

enum hash_errors {
        BIG_HASH = 0xB164A4
};

struct hashtable_t {
        list_t *lists   = nullptr;
        size_t capacity =       0;
};

typedef size_t (*hash_func_t)(word_t*);

int    hashtable_ctor         (hashtable_t *hashtable, size_t capacity);
int    hashtable_dtor         (hashtable_t *hashtable);
int write_lists_sizes_in_file (hashtable_t *hashtable, char *buf);
int    hash_words             (text_t *text, hashtable_t *hashtable, char hash_mode);

int    find_elem              (hashtable_t *hashtable, word_t *word, hash_func_t get_word_hash);
bool   find_word_in_list      (list_t *list, word_t *word, size_t position);
bool   find_word_in_hashtable (hashtable_t *hashtable, word_t *word);
size_t find_words_crc32       (hashtable_t *hashtable, word_t *words, size_t n_words);
int    fill_words_hashtable   (text_t *text, hashtable_t *hashtable, hash_func_t get_word_hash);

size_t get_one_hash       (word_t *word);
size_t get_ascii_hash     (word_t *word);
size_t get_length_hash    (word_t *word);
size_t get_sum_ascii_hash (word_t *word);
size_t get_rol_hash       (word_t *word);
size_t get_ror_hash       (word_t *word);
size_t get_my_hash        (word_t *word);
size_t get_crc32_hash     (word_t *word);

int test_all_hash_funcs (const char *input_file_name);

#endif /*HASHTABLE_H*/
