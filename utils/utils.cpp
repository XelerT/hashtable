#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "../include/config.h"

void my_ror (void *original_val, size_t sizeof_val, char shift)
{
        assert(original_val);

        size_t val = 0;
        memcpy(&val, original_val, sizeof_val);

        val = (val >> shift) | (val << (32 - shift));
        memcpy(original_val, &val, sizeof_val);
}

void my_rol (void *original_val, size_t sizeof_val, char shift)
{
        assert(original_val);

        size_t val = 0;
        memcpy(&val, original_val, sizeof_val);

        val =  (val << shift) | (val >> (32 - shift));
        memcpy(original_val, &val, sizeof_val);
}
