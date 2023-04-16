#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "hashtable.h"

int write_lists_sizes_in_file (list_t *hashtable, const char *file_name)
{
        assert(hashtable);
        assert(file_name);

        FILE *output = fopen(file_name, "w");
        if (!output) {
                log_error(1, "FILE DIDN\'T OPEN!");
                return NULL_FILE_PTR;
        }
        char buf[HASHTABLE_SIZE * 16] = {'\0'};
        size_t n_wr_chars = 0;
        size_t buf_size   = 0;

        for (size_t i = 0; i < HASHTABLE_SIZE; i++) {
                sprintf(buf + buf_size, "%ld %ln", hashtable[i].size, &n_wr_chars);
                buf_size += n_wr_chars;
        }

        n_wr_chars = fwrite(buf, sizeof(char), buf_size, output);

        fclose(output);
        if (!n_wr_chars)
                return NULL_FWRITE;

        return 0;
}

int call_choosed_hash_words (text_t *text, list_t *hashtable, char hash_mode)
{
        assert(text);
        assert(hashtable);

        switch (hash_mode) {
        case '1':
                hash_words(text, hashtable, get_one_hash);
                break;
        case '2':
                hash_words(text, hashtable, get_ascii_hash);
                break;
        case '3':
                hash_words(text, hashtable, get_length_hash);
                break;
        case '4':
                hash_words(text, hashtable, get_sum_ascii_hash);
                break;
        case '5':
                hash_words(text, hashtable, get_rol_hash);
                break;
        case '6':
                hash_words(text, hashtable, get_ror_hash);
                break;
        case '7':
                hash_words(text, hashtable, get_my_hash);
                break;
        default:
                hash_words(text, hashtable, get_crc32_hash);
        }
        return 0;
}

int find_elem (list_t *hashtable, word_t *word, size_t (*get_word_hash)(word_t*))
{
        assert(hashtable);
        assert(word);

        size_t hash = get_word_hash(word);
        if (hash >= HASHTABLE_SIZE) {
                log(1, "HASH IS TOO BIG.");
                hash = hash % HASHTABLE_SIZE;
        }

        clock_t start_time = clock();

        if (!find_word_in_list(hashtable + hash, word, 1))
                printf("No such word.\n");

        clock_t end_time = clock();

        float delta = 1000.f * (float) (end_time - start_time) / CLOCKS_PER_SEC;
        printf("Time to find \"%s\" was %.4f ms.\n", word->ptr, delta);

        return 0;
}

bool find_word_in_list (list_t *list, word_t *word, size_t position)
{
        assert(list);
        assert(word);

        if (list->size == 0)
                return false;

        if (!strcmp((char*) list->data[position].data, word->ptr)) {
                return true;
        } else if (list->data[position].next != 0) {
                return find_word_in_list(list, word, list->data[position].next);
        }

        return false;
}

int hash_words (text_t *text, list_t *hashtable, size_t (*get_word_hash)(word_t*))
{
        assert(text);
        assert(hashtable);

        size_t hash = 0;

        for (size_t i = 0; i < text->n_words; i++) {
                hash = get_word_hash(text->words + i);
                if (hash >= HASHTABLE_SIZE) {
                        log(1, "HASH IS TOO BIG.");
                        hash = hash % HASHTABLE_SIZE;
                }
                list_insert(hashtable + hash, (void*) (text->words + i), hashtable[hash].size, sizeof(text->words));
        }

        return 0;
}

size_t get_one_hash (word_t *word)
{
        assert(word);

        return 1;
}

size_t get_ascii_hash (word_t *word)
{
        assert(word);

        return word->ptr[0];
}

size_t get_length_hash (word_t *word)
{
        assert(word);

        return word->length;
}

size_t get_sum_ascii_hash (word_t *word)
{
        assert(word);

        size_t hash = 0;
        for (size_t i = 0; i < word->length; i++) {
                hash += word->ptr[i];
        }
        return hash;
}

size_t get_rol_hash (word_t *word)
{
        assert(word);

        size_t hash = 0;
        for (size_t i = 0; i < word->length; i++) {
                my_rol(&hash, sizeof(hash), sizeof(hash) - 8);
                hash = hash ^ word->ptr[i];
        }
        return hash;
}

size_t get_ror_hash (word_t *word)
{
        assert(word);

        size_t hash = 0;
        for (size_t i = 0; i < word->length; i++) {
                my_ror(&hash, sizeof(hash), sizeof(hash) - 8);
                hash = hash ^ word->ptr[i];
        }
        return hash;
}

size_t get_my_hash (word_t *word)
{
        assert(word);

        size_t hash = 5381;

        for (size_t i = 0; i < word->length; i++) {
                hash = hash * 33 ^ word->ptr[i];
        }

        return hash;
}

size_t get_crc32_hash (word_t *word)
{
        assert(word);

        int hash = 0xFFFFFFFF;
        int bit = 0;

        for (size_t i = 0; i < word->length; i++) {
                char c = word->ptr[i];
                for (size_t j = 0; j < 8; j++) {
                        bit = (c ^ hash) & 1;
                        hash >>= 1;

                        if (bit)
                                hash = hash ^ 0xEDB88320;
                        c >>= 1;
                }
        }
        return ~hash;
}
