# Hashtable
---

## Overview

1) [Introduction](#what-is-it?)
2) [Hashfunctions](#hashfuncs-overview)
2) [Optimisation](#optimisation)
3) [Tests without prealocation](#general-performance)
4) [Tests with prealocation](#prealigned-and-prealloced-words)
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

## Hashfuncs overview

Before testing hashtable we need to choose the best hash function. We have 6 function to compare:

Number |    Func    | Description
-------|----------- | -------------
 1 | one            | hash = 1
 2 | ascii          | hash = word[0]
 3 | length         | hash = strlen(word)
 4 | ascii_sum      | hash = word[0] + ... + word[n]
 5 | rol            | for i in strlen(word): hash = rol(hash) ^ word[i]
 6 | crc32          | ...

Element distribution for each function is on the graph (y - number of elements in list, x - list number):

![Alt graphs](https://github.com/XelerT/hashtable/blob/main/imgs/graphs.png)

After analyzing graphs, we can say that rol and crc32 can be used as hash-functions.

>**Note:** Tests were done using hashtable size equals 521.


## Optimisation
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

</details>


Without any optimisations we have these results:

<pre>
# started on Fri Apr 28 13:00:24 2023


 Performance counter stats for './hash-tables.out' (10 runs):

   435,666,338,866      cycles:u                                                                ( +-  9.51% )
   289,238,099,557      instructions:u                   #    1.19  insn per cycle              ( +-  9.57% )
     7,069,852,233      cache-references:u                                                      ( +-  9.56% )
     1,325,549,662      cache-misses:u                   #   33.969 % of all cache refs         ( +-  9.14% )
     4,192,165,736      bus-cycles:u                                                            ( +-  9.51% )

            17.545 +- 0.262 seconds time elapsed  ( +-  1.49% )


</pre>

Main time-consuming functions:

<pre>
-   69.53%    69.53%  hash-tables.out  hash-tables.out       [.] get_crc32_hash           main
         - 68.02% find_words_crc32
            - 67.98% find_elem
                 get_crc32_hash
         + 1.43% hash_words
-   24.46%    13.06%  hash-tables.out  hash-tables.out       [.] find_word_in_list
      - main
         + 12.39% find_words_crc32
   + 11.40% find_word_in_list
-    5.39%     5.39%  hash-tables.out  libc.so.6             [.]
   - main
      + 5.28% find_words_crc32
</pre>

## Assembler optimisation

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

Result of this optimisation is very good. Time performance boost is 245% and cache-misses reduced by 20.8%.

<pre>
# started on Fri Apr 28 13:03:21 2023


 Performance counter stats for './hash-tables.out' (10 runs):

   126,165,249,108      cycles:u                                                                ( +-  9.56% )
   115,004,589,124      instructions:u                   #    1.66  insn per cycle              ( +-  9.57% )
     6,864,306,202      cache-references:u                                                      ( +-  9.56% )
       499,472,709      cache-misses:u                   #   13.220 % of all cache refs         ( +-  9.56% )
     1,214,329,005      bus-cycles:u                                                            ( +-  9.56% )

            5.0796 +- 0.0135 seconds time elapsed  ( +-  0.27% )


</pre>

### Inlined asm

Using inlining, we can try to get more from assembly optimisation. We will use inline assembly in function which find elements.

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
# started on Fri Apr 28 13:30:00 2023


 Performance counter stats for './hash-tables.out' (10 runs):

   126,118,217,905      cycles:u                                                                ( +-  9.57% )
   113,863,804,342      instructions:u                   #    1.64  insn per cycle              ( +-  9.57% )
     6,823,876,663      cache-references:u                                                      ( +-  9.59% )
       508,751,355      cache-misses:u                   #   13.555 % of all cache refs         ( +-  9.45% )
     1,213,961,264      bus-cycles:u                                                            ( +-  9.57% )

            5.0787 +- 0.0129 seconds time elapsed  ( +-  0.25% )


</pre>

We don't have time performance boost. Therefore we stop optimising this function.

<pre>
-   74.12%    39.31%  hash-tables.out  hash-tables.out       [.] find_word_in_list
   + 37.76% _start
   - 34.85% find_word_in_list
-   18.94%    18.94%  hash-tables.out  hash-tables.out       [.] .hash
      - main
         + 18.19% find_words_crc32
-   15.30%    15.30%  hash-tables.out  libc.so.6             [.] 0x0000000000155bab
   - main
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
# started on Fri Apr 28 13:05:05 2023


 Performance counter stats for './hash-tables.out' (10 runs):

   108,317,935,603      cycles:u                                                                ( +-  9.57% )
    91,856,658,090      instructions:u                   #    1.54  insn per cycle              ( +-  9.57% )
     7,364,653,954      cache-references:u                                                      ( +-  9.55% )
       484,105,309      cache-misses:u                   #   11.933 % of all cache refs         ( +-  9.54% )
     1,042,521,607      bus-cycles:u                                                            ( +-  9.57% )

           4.36241 +- 0.00683 seconds time elapsed  ( +-  0.16% )


</pre>

We get 16% improvement in time and don't have any change in cache-misses if we take error into account.

## Strcmp

Now lets change strcmp. We will compare words which contain their length and pointer to string. For comparing strings we will use avx intrinsics. We didn't align and didn't prealloced strings before searching tests ([See next part with aligned and prealloced words](#prealigned-and-prealloced-words)):

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
# started on Fri Apr 28 13:05:50 2023


 Performance counter stats for './hash-tables.out' (10 runs):

   117,925,379,164      cycles:u                                                                ( +-  9.57% )
   131,448,359,972      instructions:u                   #    2.03  insn per cycle              ( +-  9.57% )
     4,585,284,673      cache-references:u                                                      ( +-  9.55% )
       461,886,162      cache-misses:u                   #   18.289 % of all cache refs         ( +-  9.53% )
     1,135,010,810      bus-cycles:u                                                            ( +-  9.57% )

            4.7471 +- 0.0125 seconds time elapsed  ( +-  0.26% )


</pre>

We have downgrade in time performance and have much worse results in cache-misses. Therefore we will optimise next function.

Results after:
<pre>
+   66.21%     0.87%  hash-tables.out  hash-tables.out       [.] find_elem_inlined_asm (inlined)
-   64.69%    20.44%  hash-tables.out  hash-tables.out       [.] find_word_in_list
   - 44.25% find_word_in_list
      + 44.17% avx_wordcmp
</pre>


# Prealigned and prealloced words

Now we will briefly repeat previous part of work but we will align words using **aligned_alloc()**, preparing them for avx optimisation before testing search.

## Gereral performance

<pre>
# started on Fri Apr 28 13:06:39 2023


 Performance counter stats for './hash-tables.out' (10 runs):

   398,270,300,397      cycles:u                                                                ( +-  9.57% )
   287,027,505,576      instructions:u                   #    1.31  insn per cycle              ( +-  9.57% )
     7,438,727,985      cache-references:u                                                      ( +-  9.58% )
     1,452,248,583      cache-misses:u                   #   35.583 % of all cache refs         ( +-  9.56% )
     3,832,555,188      bus-cycles:u                                                            ( +-  9.57% )

           16.0166 +- 0.0100 seconds time elapsed  ( +-  0.06% )


</pre>

## Assembler optimisation

Stats after using crc32 in assembly:

<pre>
# started on Fri Apr 28 13:09:20 2023


 Performance counter stats for './hash-tables.out' (10 runs):

   127,712,154,761      cycles:u                                                                ( +-  9.57% )
   112,774,403,060      instructions:u                   #    1.61  insn per cycle              ( +-  9.57% )
     7,249,243,794      cache-references:u                                                      ( +-  9.57% )
       995,732,862      cache-misses:u                   #   24.971 % of all cache refs         ( +-  9.64% )
     1,228,981,874      bus-cycles:u                                                            ( +-  9.57% )

           5.14597 +- 0.00920 seconds time elapsed  ( +-  0.18% )


</pre>

Execution time reduced by 211%.

### Inlining

Stats after inlining:

<pre>
# started on Fri Apr 28 13:10:13 2023


 Performance counter stats for './hash-tables.out' (10 runs):

   126,299,673,587      cycles:u                                                                ( +-  9.57% )
   111,633,623,626      instructions:u                   #    1.61  insn per cycle              ( +-  9.57% )
     7,295,683,839      cache-references:u                                                      ( +-  9.55% )
       981,095,931      cache-misses:u                   #   24.569 % of all cache refs         ( +-  9.53% )
     1,215,578,817      bus-cycles:u                                                            ( +-  9.57% )

            5.0958 +- 0.0105 seconds time elapsed  ( +-  0.21% )


</pre>

There we also don't have time performance boost.

## Recursion and strcmp

### Recursion

After removing recursion we have these stats:

<pre>
# started on Fri Apr 28 13:11:05 2023


 Performance counter stats for './hash-tables.out' (10 runs):

   107,519,857,079      cycles:u                                                                ( +-  9.57% )
    89,626,473,183      instructions:u                   #    1.52  insn per cycle              ( +-  9.57% )
     7,863,716,115      cache-references:u                                                      ( +-  9.57% )
       936,681,003      cache-misses:u                   #   21.685 % of all cache refs         ( +-  9.56% )
     1,034,597,332      bus-cycles:u                                                            ( +-  9.57% )

            4.3375 +- 0.0113 seconds time elapsed  ( +-  0.26% )


</pre>

Boost in time is 17.5%.

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
# started on Fri Apr 28 13:16:35 2023


 Performance counter stats for './hash-tables.out' (10 runs):

   108,504,445,415      cycles:u                                                                ( +-  9.57% )
   103,776,027,276      instructions:u                   #    1.74  insn per cycle              ( +-  9.57% )
     5,022,120,826      cache-references:u                                                      ( +-  9.58% )
       762,949,010      cache-misses:u                   #   27.644 % of all cache refs         ( +-  9.56% )
     1,044,143,344      bus-cycles:u                                                            ( +-  9.57% )

           4.37398 +- 0.00599 seconds time elapsed  ( +-  0.14% )


</pre>

We have small difference in time performance but we made cache-misses worse.

## Conclusion

Stats relative to general performance:

Optimisation     | In time allocation, s | Preallocated, s        
-----------------|------------------|-------------         
No               |      17.545000          |      16.016600              
Assambly crc32         |      5.079600 ( -245%)   |      5.145970  ( -211%)      
+Inlined Assembly crc32 |      5.078700 ( -245%)   |      5.095800  ( -214%)      
+Cycle instead recursion            |      4.362410 ( -302%)   |      4.337500  ( -269%)      
+AVX strcmp       |      4.747100 ( -269%)   |      4.373980  ( -266%)      


We can say that we don't need to use AVX optimisation because it becomes the most time time consuming function. Strcmp is only 1% of searching function. Maybe if we had word data base only with very long words we would have some benefit from this optimisation.

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

Without avx optimisation and using prealigned and prealloced words we have 269% improvement in time performance and have improvement in cache-misses on 14%.
Without prealigned words we have 302% boost in time performance and we reduced cache-misses by 22%.

With prealloced words time performance is 0.5% better but with error we don't have any benefit.

Ded's performance coefficient:

$\xi = 1000 \cdot \frac{\text{perf boost}} {\text{asm lines}} = 310$
