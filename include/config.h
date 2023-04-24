#ifndef CONFIG_H
#define CONFIG_H

typedef size_t elem_t;

#define LOG_ON
#define ASSERT_ON
#define DEBUG

// #define NO_OPTIMISATION
// #define CYCLE_OPTIMISATION
// #define AVX_OPTIMISATION
// #define ASM_CRC32_OPTIMISATION
// #define INL_ASM_CRC32_OPTIMISATION

#ifdef INL_ASM_CRC32_OPTIMISATION

#define AVX_OPTIMISATION
#define ASM_CRC32_OPTIMISATION

#elifdef  ASM_CRC32_OPTIMISATION

#define AVX_OPTIMISATION

#endif /*INL_ASM_CRC32_OPTIMISATION*/

// #define PRE_ALIGNED_WORDS

#include "../debug/debug.h"
#include "../log/log.h"
#include "utils.h"

#endif /*CONFIG_H*/
