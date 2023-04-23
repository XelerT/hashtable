#ifndef OPTIMISATION_H
#define OPTIMISATION_H

#include "../include/text.h"
#include "../list/list.h"

bool avx_wordcmp (word_t *word1, word_t *word2);
int find_elem_inlined_asm (hashtable_t *hashtable, word_t *word);

#endif /*OPTIMISATION_H*/
