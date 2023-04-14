#include <assert.h>
#include "hashtable.h"

int hash_words (text_t *text, list_t *hashtable, int (*get_word_hash)(const void*))
{
        assert(text);
        assert(hashtable);

        int hash = 0;

        for (int i = 0; i < text->n_words; i++) {
                hash = get_word_hash(text->words + i);
                if (hash >= HASHTABLE_SIZE) {
                        log_error(1, "HASH IS TOO BIG.");
                        return BIG_HASH;
                }
                list_insert(hashtable + hash, text->words + i, hashtable[hash].size, sizeof(text->words));
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

