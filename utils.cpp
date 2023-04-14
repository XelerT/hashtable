#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "include/config.h"

void my_ror (unsigned char *original_val, size_t sizeof_val, size_t max_iter)
{
        assert(original_val);

        size_t mask = 1;
        size_t temp_val = 0;
        size_t val = 0;
        memcpy(&val, original_val, sizeof_val);
        size_t val2 = 0;
        size_t n_bits = sizeof_val * 8 - 1;

        printf("%x\n", val);
        for (size_t i = 0; i < max_iter; i++) {
                temp_val = val & mask;
                temp_val = temp_val << (n_bits - i);
                val = val >> 1;
                val |= temp_val;
        }
        memcpy(original_val, &val, sizeof_val);

        printf("%x\n", val);
}

void my_rol (unsigned char *original_val, size_t sizeof_val, size_t max_iter)
{
                assert(original_val);

        size_t mask = 1;
        size_t temp_val = 0;
        size_t val = 0;
        memcpy(&val, original_val, sizeof_val);
        size_t val2 = 0;
        size_t n_bits = sizeof_val * 8 - 1;
        mask = mask << n_bits;

        printf("%x\n", val);
        for (size_t i = 0; i < max_iter; i++) {
                temp_val = val & mask;
                temp_val = temp_val >> (n_bits - i);
                val = val << 1;
                val |= temp_val;
        }
        memcpy(original_val, &val, sizeof_val);

        printf("%x\n", val);
}
