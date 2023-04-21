#ifndef OPTIMISATION_H
#define OPTIMISATION_H

#include "../include/text.h"
#include "../list/list.h"

bool minus_opt_find_word_in_list (list_t *list, word_t *word, size_t position);
bool avx_wordcmp (word_t *word1, word_t *word2);

#endif /*OPTIMISATION_H*/
