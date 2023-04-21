#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <immintrin.h>

#include "hashtable.h"
#include "optimisation.h"

bool minus_opt_find_word_in_list (list_t *list, word_t *word, size_t position)
{
        assert(list);
        assert(word);

        if (list->size == 0)
                return false;

        for (size_t i = 0; i < list->size; i++) {
                if (!strcmp(((word_t*) list->data[position].data)->ptr, word->ptr))
                        return true;
        }
        return false;
}

bool avx_wordcmp (word_t *word1, word_t *word2)
{
        assert(word1);
        assert(word2);

        if (word1->length != word2->length) {
                return false;
        }

        char str1[m256_BYTE_CAPACITY] = {'\0'};
        char str2[m256_BYTE_CAPACITY] = {'\0'};

        memcpy(str1, word1->ptr, word1->length);
        memcpy(str2, word2->ptr, word2->length);

        __m256i avx_str1 = _mm256_lddqu_si256((__m256i*) str1);
        __m256i avx_str2 = _mm256_lddqu_si256((__m256i*) str2);

        int next = _mm256_testc_si256(avx_str1, avx_str2);
        if (next)
                return true;

        return false;
}
