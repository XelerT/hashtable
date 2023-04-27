# Hashtable
---

## Overview

1) [Introduction](#what-is-it?)
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
4. If words has the same hash they are are inserted into next position in list.

## Hash-funcs overview

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
do not meet in it, total number of words to search is 570391 and 67 isn't in text. We will search words in hashtable 5 times.

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


Without any optimisations we have this results:

<pre>
# started on Thu Apr 27 09:55:55 2023


 Performance counter stats for './hash-tables.out' (10 runs):

    52,089,683,811      cycles:u                                                                ( +-  9.53% )
    37,067,852,121      instructions:u                   #    1.29  insn per cycle              ( +-  9.57% )
       831,042,517      cache-references:u                                                      ( +-  9.52% )
        63,095,350      cache-misses:u                   #   13.822 % of all cache refs         ( +-  7.57% )
       501,660,578      bus-cycles:u                                                            ( +-  9.52% )

            2.1046 +- 0.0113 seconds time elapsed  ( +-  0.53% )


</pre>

Main time-consuming functions:

<pre>
-   66.32%    66.31%  hash-tables.out  hash-tables.out       [.] get_crc32_hash
      - main
         + 55.15% find_words_crc32
         + 11.10% hash_words
-   22.69%    11.95%  hash-tables.out  hash-tables.out       [.] find_word_in_list
        + 11.50% _start
        + 10.74% find_word_in_list
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

Result of this optimisation is very good. Time performance boost is 196% and cache-misses reduced by 5.5%.

<pre>
# started on Thu Apr 27 09:56:18 2023


 Performance counter stats for './hash-tables.out' (10 runs):

    17,420,255,784      cycles:u                                                                ( +-  9.56% )
    16,574,313,711      instructions:u                   #    1.73  insn per cycle              ( +-  9.57% )
       813,839,750      cache-references:u                                                      ( +-  9.58% )
        37,387,852      cache-misses:u                   #    8.352 % of all cache refs         ( +-  9.15% )
       167,624,636      bus-cycles:u                                                            ( +-  9.56% )

           0.70960 +- 0.00118 seconds time elapsed  ( +-  0.17% )


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
# started on Thu Apr 27 09:56:26 2023


 Performance counter stats for './hash-tables.out' (10 runs):

    17,524,611,910      cycles:u                                                                ( +-  9.56% )
    16,460,237,725      instructions:u                   #    1.71  insn per cycle              ( +-  9.57% )
       810,235,791      cache-references:u                                                      ( +-  9.56% )
        36,933,494      cache-misses:u                   #    8.316 % of all cache refs         ( +-  9.20% )
       168,681,121      bus-cycles:u                                                            ( +-  9.56% )

           0.71451 +- 0.00184 seconds time elapsed  ( +-  0.26% )


</pre>

We don't have time performance boost. Therefore we stopped optimising this function.

<pre>
+   66.37%     1.08%  hash-tables.out  hash-tables.out       [.] find_elem_inlined_asm (inlined)
+   62.68%    31.34%  hash-tables.out  hash-tables.out       [.] find_word_in_list

</pre>

## Recursion

Now let's optimise get_word_in_list function. Main time expenses is recursive way of checking list's element:

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
# started on Thu Apr 27 09:56:34 2023


 Performance counter stats for './hash-tables.out' (10 runs):

    15,340,506,011      cycles:u                                                                ( +-  9.57% )
    13,875,621,168      instructions:u                   #    1.64  insn per cycle              ( +-  9.57% )
       876,930,244      cache-references:u                                                      ( +-  9.58% )
        37,589,941      cache-misses:u                   #    7.785 % of all cache refs         ( +-  9.32% )
       147,612,401      bus-cycles:u                                                            ( +-  9.57% )

           0.62583 +- 0.00124 seconds time elapsed  ( +-  0.20% )


</pre>

We get 14% improvement in time and don't have any change in cache-misses if we take error into account.

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
# started on Thu Apr 27 09:56:42 2023


 Performance counter stats for './hash-tables.out' (10 runs):

    16,421,661,557      cycles:u                                                                ( +-  9.57% )
    18,513,034,554      instructions:u                   #    2.05  insn per cycle              ( +-  9.57% )
       544,304,449      cache-references:u                                                      ( +-  9.58% )
        40,300,203      cache-misses:u                   #   13.504 % of all cache refs         ( +-  9.48% )
       158,188,150      bus-cycles:u                                                            ( +-  9.57% )

           0.67035 +- 0.00199 seconds time elapsed  ( +-  0.30% )


</pre>

We have downgrade in time performance boost and have much worse results in cache-misses. Therefore we will optimise next function.

Results after:
<pre>
+   66.21%     0.87%  hash-tables.out  hash-tables.out       [.] find_elem_inlined_asm (inlined)
-   64.69%    20.44%  hash-tables.out  hash-tables.out       [.] find_word_in_list
   - 44.25% find_word_in_list
      + 44.17% avx_wordcmp
</pre>


# Prealigned and prealloced words

Now we will briefly repeat previous part of work but we will aligne words using **aligned_alloc()**, preparing them for avx optimisation before testing search.

## Gereral performance

<pre>
# started on Thu Apr 27 09:56:50 2023


 Performance counter stats for './hash-tables.out' (10 runs):

    49,623,508,827      cycles:u                                                                ( +-  9.57% )
    37,385,070,117      instructions:u                   #    1.37  insn per cycle              ( +-  9.57% )
       886,291,124      cache-references:u                                                      ( +-  9.58% )
        99,567,778      cache-misses:u                   #   20.450 % of all cache refs         ( +-  9.55% )
       477,523,244      bus-cycles:u                                                            ( +-  9.57% )

           2.00808 +- 0.00242 seconds time elapsed  ( +-  0.12% )


</pre>

## Assembler optimisation

Stats after using crc32 in assembly:

<pre>
# started on Thu Apr 27 09:57:11 2023


 Performance counter stats for './hash-tables.out' (10 runs):

    18,002,737,268      cycles:u                                                                ( +-  9.57% )
    16,889,223,424      instructions:u                   #    1.70  insn per cycle              ( +-  9.57% )
       871,339,419      cache-references:u                                                      ( +-  9.57% )
        87,950,704      cache-misses:u                   #   18.354 % of all cache refs         ( +-  9.38% )
       173,230,147      bus-cycles:u                                                            ( +-  9.57% )

          0.738348 +- 0.000990 seconds time elapsed  ( +-  0.13% )


</pre>

Execution time reduced by 172%.

### Inlining

Stats after inlining:

<pre>
# started on Thu Apr 27 09:57:20 2023


 Performance counter stats for './hash-tables.out' (10 runs):

    17,853,610,009      cycles:u                                                                ( +-  9.56% )
    16,775,145,261      instructions:u                   #    1.71  insn per cycle              ( +-  9.57% )
       878,108,331      cache-references:u                                                      ( +-  9.59% )
        87,260,191      cache-misses:u                   #   18.091 % of all cache refs         ( +-  9.28% )
       171,794,443      bus-cycles:u                                                            ( +-  9.56% )

           0.73485 +- 0.00312 seconds time elapsed  ( +-  0.42% )


</pre>

There we also don't have time performance boost.

## Recursion and strcmp

### Recursion

Stats after recursion deletion:

<pre>
# started on Thu Apr 27 09:57:28 2023


 Performance counter stats for './hash-tables.out' (10 runs):

    15,808,396,618      cycles:u                                                                ( +-  9.56% )
    14,190,532,059      instructions:u                   #    1.63  insn per cycle              ( +-  9.57% )
       950,121,548      cache-references:u                                                      ( +-  9.57% )
        86,379,617      cache-misses:u                   #   16.509 % of all cache refs         ( +-  9.27% )
       152,115,545      bus-cycles:u                                                            ( +-  9.56% )

           0.65053 +- 0.00121 seconds time elapsed  ( +-  0.19% )


</pre>

Boost in time is 13%.

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
# started on Thu Apr 27 09:57:36 2023


 Performance counter stats for './hash-tables.out' (10 runs):

    15,765,605,511      cycles:u                                                                ( +-  9.57% )
    15,848,639,828      instructions:u                   #    1.83  insn per cycle              ( +-  9.57% )
       617,968,566      cache-references:u                                                      ( +-  9.61% )
        75,244,448      cache-misses:u                   #   22.276 % of all cache refs         ( +-  9.36% )
       151,702,992      bus-cycles:u                                                            ( +-  9.57% )

           0.64829 +- 0.00105 seconds time elapsed  ( +-  0.16% )


</pre>

We don't have any difference in time performance but make cache-misses worse.

## Conclusion

Stats relative to general performance:

Optimisation     | Intime allocation | Preallocated        
-----------------|------------------|-------------         
No               |      2.104600          |      2.008080              
Assambly         |      0.709600 ( -196%)   |      0.738348  ( -171%)      
Inlined Assembly |      0.714510 ( -194%)   |      0.734850  ( -173%)      
Cycle            |      0.625830 ( -236%)   |      0.650530  ( -208%)      
AVX strcmp       |      0.670350 ( -213%)   |      0.648290  ( -209%)      


Using prealigned words we have 209% improvement in time performance and don't have improvement in cache-misses if we taking the error into account.
Without prealigned words we have 213% boost in time performance and we don't have any changes in cache-misses if we taking into account the error.

With prealloced words time performance is 3% better.

Ded's performance coefficient:

$\xi = 1000 \cdot \frac{\text{perf boost}} {\text{asm lines}} = 310$
