# Hashtable
---

## Overview

1) [Introduction](#what-is-it?)
2) [Hashfunctions](#hashfunctions-overview)
2) [Optimization](#optimization)
3) [Tests without prealocation](#general-performance)
4) [Tests with prealocation](#prealigned-and-preallocated-words)
5) [Conclusion](#conclusion)

### What is it?

A hashtable or hash map is a data structure that implements an associative array. A hashtable uses a hash function to compute an index to put object inside. Simlpy, hashtable is an array of lists.

![Alt hashtable]( https://github.com/XelerT/hashtable/blob/main/imgs/Screenshot_20230421_164942.png)

### What's happening
1. Load text and parsing it.
2. If word was already added, skip it.
2. Calculate hashcodes for every word.
3. This hashcodes is the indexes of lists in hashtable.
4. If words has the same hash they are inserted into next position in list.

## Hashfunctions overview

Before testing hashtable we need to choose the best hash function. We have 6 functions to compare:

Number |    Func    | Description
-------|----------- | -------------
 1 | one            | hash = 1
 2 | ascii          | hash = word[0]
 3 | length         | hash = strlen(word)
 4 | ascii_sum      | hash = word[0] + ... + word[n]
 5 | rol            | for i in strlen(word): hash = rol(hash) ^ word[i]
 6 | crc32          | ...

Element distribution for each function is on the graph (vertical axis - number of elements in list, horizontal axis - list number):

![Alt graphs](https://github.com/XelerT/hashtable/blob/main/imgs/graphs.png)

After analyzing graphs, we can say that rol and crc32 can be used as hash-functions.

>**Note:** Tests were done using hashtable size equals 521.


## Optimization
---
In this part of project **crc32** was chosen as hash function.

As data base was chosen **Atlas Shrugged** (~17 000 unique words), as search data used file with words from text and words that
do not meet in it, total number of words to search is 570391 and 67 isn't in text. We will search words in hashtable 50 times.

### General performance

As main performance argument we will choose execution time of program. Therefore we will test performance 10 times and then will get average execution time using perf.

<details>

<summary>How to run tests</summary>

We made testing process very simple. You need to run bash script to test hashtable and hashfunctions by yourself. You need to choose mode of testing:
1) graphs - test hashfunctions and draw graphs using python.
2) tests  - run program in different modes using perf.

        $ bash run_tests.sh
        Enter mode of hashtable tests: tests

Also you can generate README.md file with your last perf stats:

        $ make write_readme

Used perf utils:

        $ perf stat -o perf_stats/perf_stat1.txt -r 10 -e cycles,instructions,cache-references,cache-misses,bus-cycles ./hash-tables.out
        $ perf record --call-graph dwarf ./hash-tables.out
</details>


Without any optimizations we have these results:

<pre>
!*!*!* 1*!*!*!
</pre>

Main time-consuming functions:

<pre>
   - 68.02% find_words_crc32
        - 67.98% find_elem
           get_crc32_hash
        + 1.43% hash_words
   + 12.39% find_words_crc32
   + 11.40% find_word_in_list
   + 5.28%  find_words_crc32
</pre>

## Assembly optimization

Firstly we need to optimise the most consuming function - get_crc32_hash:

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

For optimising this function we can use assembly:

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

Result of this optimization is very good. Execution time decreased by 3.45 times and cache-misses reduced by 20.8%.

<pre>
!*!*!* 4*!*!*!
</pre>

### Inlined assembly

Using inlining, we can try to get more from assembly optimization. We will use inline assembly in function which find elements.

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

We don't have time performance boost. Therefore we stop optimising this function.

<pre>
-   74.12%    39.31%  hash-tables.out  hash-tables.out       [.] find_word_in_list
   - 34.85% find_word_in_list
-   18.94%    18.94%  hash-tables.out  hash-tables.out       [.] .hash
        + 18.19% find_words_crc32
-   15.30%    15.30%  hash-tables.out  libc.so.6             [.] 0x0000000000155bab
      + 15.05% find_words_crc32
</pre>

## Recursion

Now let's optimise find_word_in_list function. Main time expenses is recursive way of checking list's element:

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
    3.67 │       mov   (%rax),%rsi
    0.05 │       mov   -0x18(%rbp),%rdx
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

Firstly we change recursion using cycle:

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

Program has been improved, being 1.16x faster and we don't have any change in cache-misses if we take error into account.

## Strcmp

Now lets change strcmp. We will compare words which contain their length and pointer to string. For comparing strings we will use avx intrinsics. We didn't align and didn't preallocated strings before searching tests ([See next part with aligned and preallocated words](#prealigned-and-preallocated-words)):

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

We have downgrade in time performance and have much worse results in cache-misses. Therefore we will optimise next function.

Results after:
<pre>
+   66.21%     0.87%  hash-tables.out  hash-tables.out       [.] find_elem_inlined_asm (inlined)
-   64.69%    20.44%  hash-tables.out  hash-tables.out       [.] find_word_in_list
   - 44.25% find_word_in_list
      + 44.17% avx_wordcmp
</pre>


# Prealigned and preallocated words

Now we will briefly repeat previous part of work but we will align words using **aligned_alloc()**, preparing them for avx optimization before testing search.

## Gereral performance

<pre>
!*!*!* 6*!*!*!
</pre>

## Assembly optimization

Stats after using crc32 in assembly:

<pre>
!*!*!* 9*!*!*!
</pre>

Execution time reduced by 3.11 times.

### Inlining

Stats after inlining:

<pre>
!*!*!*10*!*!*!
</pre>

There we also don't have time performance boost.

## Recursion and strcmp

### Recursion

After removing recursion we have these stats:

<pre>
!*!*!* 7*!*!*!
</pre>

Time reduced by 1.18 times.

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

        __m256i avx_str1 = _mm256_load_si256((__m256i*) word1->ptr);
        __m256i avx_str2 = _mm256_load_si256((__m256i*) word2->ptr);

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

We have small difference in time performance but we made cache-misses worse.

## Conclusion

Stats relative to general performance:

!*!*!*TABLE!*!*!*

We can say that we don't need to use AVX optimization because it becomes the most time consuming function. Strcmp is only 1% of find_word_in_list function. Maybe if we had word data base only with very long words we would have some benefit from this optimization.

<pre>
-   68.51%    34.08%  hash-tables.out  hash-tables.out       [.] find_word_in_list
   - 34.42% find_word_in_list
        ...
        1.06% strcmp@plt
   + 33.66% _start
+   21.97%    21.97%  hash-tables.out  hash-tables.out       [.] .hash
</pre>

<pre>
-   74.45%    24.42%  hash-tables.out  hash-tables.out       [.] find_word_in_list
   - 50.03% find_word_in_list
      + 49.98% avx_wordcmp
</pre>

Without avx optimization and using prealigned and preallocated words we have reduced execution time by 3.69 times and we have improvement in cache-misses on 14%.
Without prealigned words we have reduced execution time by 4.02 times and we decreased cache-misses by 22%.

Without preallocated words program is working 1.01 times slower but with error we don't have any benefit.

Ded's performance coefficients:

$\xi_1 = 1000 \cdot \frac{\text{perf boost}} {\text{asm lines}} = 400$

$\xi_2 = 1000 \cdot \frac{\text{perf boost}} {\text{asm lines}} = 369$
