# Hashtable
---
### What is it?

A hashtable or hash map is a data structure that implements an associative array. A hashtable uses a hash function to compute an index to put object inside. Simlpy, hashtable is an array of lists.

![Alt hashtable]( https://github.com/XelerT/hashtable/blob/main/imgs/Screenshot_20230421_164942.png)

### What's happening
1. Load text and parsing it.
2. If word was already added, skip it.
2. Calculate hashcodes for every word.
3. This hashcodes is the indexes of lists in hashtable.
4. If words has the same hash they are pasted on next position in list.

## Hash-funcs overview

We have 6 hash-functions for comparison:

Number |    Func    | Description
-------|----------- | -------------
 1 | one            | hash = 1
 2 | ascii          | hash = word[0]
 3 | length         | hash = strlen(word)
 4 | ascii_sum      | hash = word[0] + ... + word[n]
 5 | rol            | for i in strlen(word) hash = rol(hash) ^ word[i]
 6 | crc32          | ...

Element distribution for each function is on the graph (y - number of elements in list, x - list number):

![Alt graphs](https://github.com/XelerT/hashtable/blob/main/imgs/graphs.png)

After analyzing graphs, we can say that ascii_sum, rol, crc32 can be used as hash-functions.

>**Note:** Tests were done using hashtable size equals 32.


## Optimisation
---
In this part of project **crc32** was chosen as hash function.

As data base was chosen **Atlas Shrugged** (~17 000 unique words), as search data used file with words from text and words that
do not meet in it, total number of words to search is 570391 (one full text and a little bit more) and 67 isn't in text.

### General performance

As main performance argument we will choose execution time of program. Therefore we will test performance 5 times and then will get average execution time.

Without any optimisations we have this results:

<pre>

!*!*!* 1*!*!*!

</pre>

Main time-consuming functions:

<pre>
+   68.73%    31.14%  hash-tables.out  hash-tables.out       [.] find_word_in_list
+   22.71%    22.71%  hash-tables.out  hash-tables.out       [.] get_crc32_hash
</pre>

## Recursion

We started our optimisation from get_word_in_list, as the main time consuming function. Main time expenses is recursive way of checking list's element and, as supposed, strcmp.

```C
bool find_word_in_list (list_t *list, word_t *word, size_t position)
{
        assert(list);
        assert(word);

        if (list->size == 0)
                return false;

/*       │ 89:   mov   -0x10(%rbp),%rax
         │       mov   (%rax),%rcx
         │       mov   -0x8(%rbp),%rax
    3.67 │       mov   (%rax),%rsi                                                                                       0.05 │       mov   -0x18(%rbp),%rdx
         │       mov   %rdx,%rax
         │       add   %rax,%rax
    3.52 │       add   %rdx,%rax
         │       shl   $0x3,%rax
         │       add   %rsi,%rax
    3.50 │       mov   (%rax),%rax
   22.97 │       mov   (%rax),%rax
    0.13 │       mov   %rcx,%rsi
         │       mov   %rax,%rdi
    3.72 │     → call  strcmp@plt 			*/
        if (!strcmp(((word_t*) list->data[position].data)->ptr, word->ptr)) {
                return true;
        } else if (list->data[position].next != 0) {
                return find_word_in_list(list, word, list->data[position].next);
        }

/* 12.67 │124:   leave
    7.82 │     ← ret  			*/
        return false;
}
```

Firstly we change recursion using cycle.

```C
bool find_word_in_list (list_t *list, word_t *word, size_t position)
{
        assert(list);
        assert(word);

        if (list->size == 0)
                return false;

        while (list->data[position].next) {
                if (!strcmp(((word_t*) list->data[position].data)->ptr, word->ptr))
                        return true;
                position++;
        }
        return false;
}
```

 Stats:

<pre>

!*!*!* 2*!*!*!

</pre>

We get 27% improvement in time and cache-misses percentage decreased by 0.3%.

## Strcmp

Now lets change strcmp. We will compare words which contain their length and pointer to string. For comparing strings we will use avx intrinsics. We didn't alligne strings before searching tests ([See next part with aligned words](#prealigned-words)).

```C
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
```

<pre>

!*!*!* 3*!*!*!

</pre>

We don't have time performance boost and have worse results in cache-misses. Inlining function also doesn't have any countable effect. Therefore we will optimise next function.

Results after:
<pre>
+   60.23%    18.86%  hash-tables.out  hash-tables.out       [.] find_word_in_list
+   29.50%    29.50%  hash-tables.out  hash-tables.out       [.] get_crc32_hash
+   15.33%    15.33%  hash-tables.out  libc.so.6             [.] 0x0000000000155bab
</pre>

## Assembler optimisation

Next function to optimise is get_crc32_hash.

```C
size_t get_crc32_hash (word_t *word)
{
        assert(word);

        int hash = 0xFFFFFFFF;
        int bit = 0;

     /* 3.72 │      addq   $0x1,-0x8(%rbp)
        7.14 │9a:   cmpq   $0x7,-0x8(%rbp)  */
        for (size_t i = 0; i < word->length; i++) {
                char c = word->ptr[i];
                for (size_t j = 0; j < 8; j++) {
        /*   3.83 │71:  movsbl -0x19(%rbp),%eax
             3.74 │     xor    -0x18(%rbp),%eax
             2.13 │     and    $0x1,%eax
             5.85 │     mov    %eax,-0x14(%rbp)  */
                        bit = (c ^ hash) & 1;
                        hash >>= 1;

       //  13.13 │      cmpl   $0x0,-0x14(%rbp)
                        if (bit) {
                /*  3.83 │71:   movsbl -0x19(%rbp),%eax
                    3.74 │      xor    -0x18(%rbp),%eax
                    2.13 │      and    $0x1,%eax
                    5.85 │      mov    %eax,-0x14(%rbp)   */
                                hash = hash ^ 0xEDB88320;
                        }
       //  20.09 │92:   sarb   -0x19(%rbp)
                        c >>= 1;
                }
        }
        return ~hash;
}
```

For optimising this function we can use assembly function.

```nasm

asm_get_crc32_hash:

        mov rax, 0xFFFFFFFF
        mov rcx, qword [rdi + 8]
        mov rsi, qword [rdi]
.hash:
        crc32 eax, byte [rsi]
        inc rsi
        loop .hash

        ret
```

Result of this optimisation is pretty good. Time performance boost is 34% and mispredicted caches increased by 0.4%.

<pre>

!*!*!* 4*!*!*!

</pre>

### Inlined asm

Using inlining, we can try to get more from assembler optimisation. We will inline assembler in function which find elements.

```C

int find_elem_inlined_asm (hashtable_t *hashtable, word_t *word)
{
        assert(hashtable);
        assert(word);

        size_t hash = 0;

        asm (
                "mov %0, 0xFFFFFFFF\n\t"
                "mov rcx, qword [%1]\n\t"
                "mov rsi, qword [%1 - 0x8]\n\t"
                ".hash:\n"
                "crc32 %0, byte ptr [rsi]\n\t"
                "inc rsi\n\t"
                "loop .hash\n"
                : "=a" (hash)
                : "D" (word)
                : "rcx", "rsi", "memory"
        );

        if (hash >= hashtable->capacity)
                hash = hash % hashtable->capacity;

        if (!find_word_in_list(hashtable->lists + hash, word, 1))
                return NO_ELEMENT;

        return 0;
}

```

<pre>

!*!*!* 5*!*!*!

</pre>

We don't have time performance, but we decreased cache-misses by 0.3%. Therefore we stopped optimising this function.

# Prealigned words

Now we will aligne words and prepare them for avx optimisation before search tests.

This part briefly repeats stats from previous part of work but with aligned words before searching them in hashtable.

## Gereral performance

<pre>

!*!*!* 6*!*!*!

</pre>

## Recursion and strcmp

### Recursion

Stats after recursion deletion:

<pre>

!*!*!* 7*!*!*!

</pre>

Boost in time is 19%, cache-misses increased by 0.1%.

### Strcmp

New words compare function:

```C
bool avx_wordcmp (word_t *word1, word_t *word2)
{
        assert(word1);
        assert(word2);

        if (word1->length != word2->length) {
                return false;
        }

        __m256i avx_str1 = _mm256_lddqu_si256((__m256i*) word1->ptr);
        __m256i avx_str2 = _mm256_lddqu_si256((__m256i*) word2->ptr);

        int next = _mm256_testc_si256(avx_str1, avx_str2);
        if (next)
                return true;

        return false;
}
```

Stats after using words compare function:

<pre>

!*!*!* 8*!*!*!

</pre>

Time performance boost is 11% and percentage of cache-misses increased by 0.3%. By that we overtake our previous measurements and get an advantage from words alignment.

## Assembler optimisation

Stats after using crc32 in assembler:

<pre>

!*!*!* 9*!*!*!

</pre>

Execution time reduced by 40% and cache-misses reduced by 0.3%.

### Inlining

Stats after inlining:

<pre>

!*!*!*10*!*!*!

</pre>

There we also have time performance boost in 1.3%.

## Conclusion

Using prealigned words we have 95.8% improvement in time performance and increased cache-misses by 0.1% if we taking the error into account.
Without prealigned words we have 72.4% boost in time performance and we don't have any changes in cache-misses if we taking into account the error.

Ded's performance coefficient:

$\xi = 1000 \cdot \frac{\text{perf boost}} {\text{asm lines}} = 200$
