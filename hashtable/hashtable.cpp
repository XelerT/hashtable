#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "hashtable.h"

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
        default:
                hash_words(text, hashtable, get_my_hash);
                return 0;
        }
        return 0;
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
                list_insert(hashtable + hash, (unsigned char*) (text->words + i), hashtable[hash].size, sizeof(text->words));
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
