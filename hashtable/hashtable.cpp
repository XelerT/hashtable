#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "hashtable.h"

int hashtable_ctor (hashtable_t *hashtable, size_t capacity)
{
        assert(hashtable);

        hashtable->lists = (list_t*) calloc(capacity, sizeof(list_t));
        if (!hashtable->lists) {
                log(1, "Null calloc for hashtable lists.");
                return NULL_CALLOC;
        }

        for (size_t i = 0; i < capacity; i++) {
                list_ctor(hashtable->lists + i, 5, sizeof(data_t), sizeof(word_t*));
        }
        hashtable->capacity = capacity;

        return 0;
}

int hashtable_dtor (hashtable_t *hashtable)
{
        assert(hashtable);
        assert(hashtable->lists);

        for (size_t i = 0; i < hashtable->capacity; i++)
                list_dtor(hashtable->lists + i);

        free(hashtable->lists);
        hashtable->lists = nullptr;

        return 0;
}

int write_lists_sizes_in_file (hashtable_t *hashtable, char *buf)
{
        assert(hashtable);
        assert(buf);

        int n_wr_chars = 0;
        int buf_size   = 0;

        for (size_t i = 0; i < HASHTABLE_SIZE; i++) {
                sprintf(buf + buf_size, " %ld%n", hashtable->lists[i].size, &n_wr_chars);
                buf_size += n_wr_chars;
        }

        if (!n_wr_chars)
                return NULL_FWRITE;

        return buf_size;
}

int hash_words (text_t *text, hashtable_t *hashtable, char hash_mode)
{
        assert(text);
        assert(hashtable);

        switch (hash_mode) {
        case '1':
                fill_words_hashtable(text, hashtable, get_one_hash);
                break;
        case '2':
                fill_words_hashtable(text, hashtable, get_ascii_hash);
                break;
        case '3':
                fill_words_hashtable(text, hashtable, get_length_hash);
                break;
        case '4':
                fill_words_hashtable(text, hashtable, get_sum_ascii_hash);
                break;
        case '5':
                fill_words_hashtable(text, hashtable, get_rol_hash);
                break;
        case '6':
                fill_words_hashtable(text, hashtable, get_ror_hash);
                break;
        case '7':
                fill_words_hashtable(text, hashtable, get_my_hash);
                break;
        default:
                fill_words_hashtable(text, hashtable, get_crc32_hash);
        }
        return 0;
}

size_t find_words_crc32 (hashtable_t *hashtable, word_t *words, size_t n_words)
{
        assert(hashtable);
        assert(words);

        size_t found_n_words = 0;

        clock_t start_time = clock();

        for (size_t i = 0; i < n_words; i++) {
                if (!find_elem(hashtable, words + i, get_crc32_hash))
                        found_n_words++;
        }

        clock_t end_time = clock();

        float delta = 1000.f * (float) (end_time - start_time) / CLOCKS_PER_SEC;
        printf("Time to find %ld words was %.4f ms.\n", n_words, delta);

        return found_n_words;
}

int find_elem (hashtable_t *hashtable, word_t *word, hash_func_t get_word_hash)
{
        assert(hashtable);
        assert(word);

        size_t hash = get_word_hash(word);
        if (hash >= hashtable->capacity) {
                log(1, "HASH IS TOO BIG.");
                hash = hash % hashtable->capacity;
        }

        if (!find_word_in_list(hashtable->lists + hash, word, 1))
                return NO_ELEMENT;

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

bool find_word_in_hashtable (hashtable_t *hashtable, word_t *word)
{
        assert(hashtable);
        assert(word);

        for (size_t i = 0; i < hashtable->capacity; i++) {
                if (find_word_in_list(hashtable->lists + i, word, 1))
                        return true;
        }

        return false;
}

int fill_words_hashtable (text_t *text, hashtable_t *hashtable, hash_func_t get_word_hash)
{
        assert(text);
        assert(hashtable);

        size_t hash = 0;
        int again = 0;

        for (size_t i = 0; i < text->n_words; i++) {
                hash = get_word_hash(text->words + i);
                if (hash >= hashtable->capacity)
                        hash = hash % hashtable->capacity;
                if (find_word_in_list(hashtable->lists + hash, text->words + i, 1)) {
                        again++;
                        continue;
                }
                list_insert(hashtable->lists + hash, (void*) (text->words + i), hashtable->lists[hash].size, sizeof(text->words));
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

static const int  N_HASH_FUNCS = 8;
const char *hash_funcs_names[] = {
                                        "one",
                                        "ascii",
                                        "length",
                                        "ascii_sum",
                                        "rol",
                                        "ror",
                                        "my",
                                        "crc32"
};

int test_all_hash_funcs (const char *input_file_name)
{
        assert(input_file_name);

        FILE *input_file = fopen(input_file_name, "r");
        if (!input_file) {
                log(1, "NULL FILE.");
                return NULL_FILE_PTR;
        }
        text_t text = {};
        get_text(input_file, &text, input_file_name);
        fclose(input_file);

        delete_punctuations(&text);
        divide_text_on_words(&text);

        hashtable_t hashtable = {};
        hashtable_ctor(&hashtable, HASHTABLE_SIZE);
        for (size_t n = 0; n < hashtable.capacity; n++)
                list_dtor(hashtable.lists + n);

        hash_func_t funcs_array[N_HASH_FUNCS] = {
                get_one_hash,
                get_ascii_hash,
                get_length_hash,
                get_sum_ascii_hash,
                get_rol_hash,
                get_ror_hash,
                get_my_hash,
                get_crc32_hash
        };

        FILE *output = fopen("graphics/test_all.txt", "w");
        char buf[HASHTABLE_SIZE * 64] = {'\0'};
        int buf_size = 0;
        int n_wr_chars = 0;

        if (!output) {
                log_error(1, "FILE DIDN\'T OPEN!");
                return NULL_FILE_PTR;
        }
        for (int i = 0; i < N_HASH_FUNCS; i++) {
                for (size_t n = 0; n < hashtable.capacity; n++)
                        list_ctor(hashtable.lists + n, 5, sizeof(data_t), sizeof(word_t*));

                fill_words_hashtable(&text, &hashtable, funcs_array[i]);

                sprintf(buf + buf_size, "%s %n", hash_funcs_names[i], &n_wr_chars);
                buf_size += n_wr_chars;
                buf_size += write_lists_sizes_in_file(&hashtable, buf + buf_size);
                buf[buf_size++] = '\n';

                for (size_t n = 0; n < hashtable.capacity; n++)
                        list_dtor(hashtable.lists + n);
                // printf("Tested hash function number %d\n", i);
        }

        fwrite(buf, sizeof(char), buf_size, output);
        fclose(output);

        text_dtor(&text);
        hashtable_dtor(&hashtable);

        return 0;
}
