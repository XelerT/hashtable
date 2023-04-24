
#include <stdio.h>
#include <stdlib.h>

#include "include/config.h"
#include "list/list.h"
#include "include/text.h"
#include "hashtable/hashtable.h"
#include "list/list_dump.h"

int main ()
{
        test_all_hash_funcs("input.txt");

        return 0;
}
