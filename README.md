-# Hashtables
---
### What is it?

A hashtable or hash map is a data structure that implements an associative array. A hashtable uses a hash function to compute an index to put object inside. Simlpy, hashtable is an array of lists. 

![Alt hashtable]( "Hashtable")

### What's happening
1. Load text and parsing it.
2. If word was already added, skip it.
2. Calculate hashcodes for every word.
3. This hashcodes is the indexes of lists in hashtable.
4. If words has the same hash they are pasted on next position in list.

## Hash-funcs overview
	We have 6 hash-functions for comparison:
	
Func       | Description
------------- | -------------
one        | hash = 1
ascii      | hash = word[0]
ascii_sum  | hash = word[0] + ... + word[n]
length     | hash = strlen(word)
rol        | for i in strlen(word) hash = rol(hash) ^ word[i]
crc32      | ...


![one][image1]
![Картинка][image2]
![Картинка][image3]
![Картинка][image4]
![rol][image5]
![crc32][image6]

[image1]: //https://github.com/XelerT/hashtable/blob/main/graphics/one.png/200x200
[image2]: //https://github.com/XelerT/hashtable/blob/main/graphics/ascii.png/200x200
[image3]: //https://github.com/XelerT/hashtable/blob/main/graphics/ascii_sum.png/200x200
[image4]: //https://github.com/XelerT/hashtable/blob/main/graphics/length.png/200x200
[image5]: //https://github.com/XelerT/hashtable/blob/main/graphics/rol.png/200x200
[image6]: //https://github.com/XelerT/hashtable/blob/main/graphics/crc32.png/200x200

After analizing graphs, we can say that ascii_sum, rol, crc32 can be used as hash-functions.

>**Note:** Tests were done using hashtable size equals 32.


## Optimisation
---
<pre>
		In this part of project <b>crc32</b> was chosen as hash function.
        
    	As data base was chosen <b>Atlas Shrugged</b>(~17 000 unique words), as search data used file with words from text and words that do not meet in it, total number of words to search is 570391(one full text and a little bit more) and 67 isn't in text.

</pre>


### General performance

<pre> 
          1,437.49 msec task-clock:u                     #    0.999 CPUs utilized             
                 0      context-switches:u               #    0.000 /sec                      
                 0      cpu-migrations:u                 #    0.000 /sec                      
             4,857      page-faults:u                    #    3.379 K/sec                     
     4,875,969,738      cycles:u                         #    3.392 GHz                       
     5,638,967,597      instructions:u                   #    1.16  insn per cycle            
       941,165,771      branches:u                       #  654.730 M/sec                     
        41,925,551      branch-misses:u                  #    4.45% of all branches           

       1.438353766 seconds time elapsed

       1.426456000 seconds user
       0.009982000 seconds sys

</pre>

As main performance argument we will choose executing time of program. Therefore we will test performance 5 times and then will get average execution time.

t1 | t2 | t3 | t4 | t5 | \<t\>
---|----|----|----|----|------
1.446 | 1.455 | 1.437 | 1.439 | 1.426 | 1.441

Main time-consuming functions:

<pre>
+   68.73%    31.14%  hash-tables.out  hash-tables.out       [.] find_word_in_list
+   22.71%    22.71%  hash-tables.out  hash-tables.out       [.] get_crc32_hash                                
</pre>

## Recursion and strcmp

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

Firstly we change recursion using cycle. Stats:

<pre>
          1,152.29 msec task-clock:u                     #    0.999 CPUs utilized             
                 0      context-switches:u               #    0.000 /sec                      
                 0      cpu-migrations:u                 #    0.000 /sec                      
             4,849      page-faults:u                    #    4.208 K/sec                     
     3,889,472,819      cycles:u                         #    3.375 GHz                       
     4,289,687,992      instructions:u                   #    1.10  insn per cycle            
       656,879,636      branches:u                       #  570.066 M/sec                     
        44,691,703      branch-misses:u                  #    6.80% of all branches           

       1.152952114 seconds time elapsed

       1.141215000 seconds user
       0.010007000 seconds sys

</pre>

t1 | t2 | t3 | t4 | t5 | \<t\>
---|----|----|----|----|------
1.159 | 1.139 | 1.131 | 1.146 | 1.141 | 1.143

We get 26% improvement in time but branch-misprediction percentage increased by 2.35%.

Now we changed strcmp. We will compare words which contain their length and pointer on string. For comparing strings we will use avx intrinsics. We didn't alligne strings before searching tests because of huge memory consumptions (If we had, we would have better performance boost in these optimisation [Next part](#Prealigned words)).

```C
          1,122.24 msec task-clock:u                     #    0.999 CPUs utilized             
                 0      context-switches:u               #    0.000 /sec                      
                 0      cpu-migrations:u                 #    0.000 /sec                      
             4,849      page-faults:u                    #    4.321 K/sec                     
     3,802,485,316      cycles:u                         #    3.388 GHz                       
     4,289,910,134      instructions:u                   #    1.13  insn per cycle            
       656,916,654      branches:u                       #  585.364 M/sec                     
        42,154,230      branch-misses:u                  #    6.42% of all branches           

       1.122860639 seconds time elapsed

       1.111512000 seconds user
       0.009945000 seconds sys

```
t1 | t2 | t3 | t4 | t5 | \<t\>
---|----|----|----|----|------
1.112 | 1.100 | 1.114 | 1.116 | 1.114 | 1.111

Inlining of this function don't have any countable effect. We have performance boost in 3%. Therefore we will optimise next function.

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

For optimising these function we can use assembly function.

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

Result of this optimisation is pretty good. Time performance boost is 32% and mispredicted branches reduced by 4.15%.

<pre>
            860.96 msec task-clock:u                     #    0.993 CPUs utilized             
                 0      context-switches:u               #    0.000 /sec                      
                 0      cpu-migrations:u                 #    0.000 /sec                      
             4,854      page-faults:u                    #    5.638 K/sec                     
     2,890,718,505      cycles:u                         #    3.358 GHz                       
     4,750,701,831      instructions:u                   #    1.64  insn per cycle            
       723,546,512      branches:u                       #  840.395 M/sec                     
        16,406,949      branch-misses:u                  #    2.27% of all branches           

       0.867078287 seconds time elapsed

       0.832934000 seconds user
       0.026337000 seconds sys

</pre>


t1 | t2 | t3 | t4 | t5 | \<t\>
---|----|----|----|----|------
0.833 | 0.845 | 0.843 | 0.850 | 0.850 | 0.844

### Inlined asm

Now we can try to get more from assembler optimisation. We will inline assembler in find element function.

```C

int find_elem_inlined_asm (hashtable_t *hashtable, word_t *word)
{
        assert(hashtable);
        assert(word);

        size_t hash = 0;

        asm (
                "mov %0, 0xFFFFFFFF\n\t"
                "mov rcx, qword [%2]\n\t"
                "mov rsi, qword [%2 - 0x8]\n\t"
                ".hash:\n"
                "crc32 %0, byte ptr [rsi]\n\t"
                "inc rsi\n\t"
                "loop .hash\n"
                : "=r" (hash)
                : "r"  (hashtable), "r" (word)
                : "rcx", "rsi", "memory"
        );
        // printf("%ld %ld\n", hash, get_word_hash(word));

        if (hash >= hashtable->capacity)
                hash = hash % hashtable->capacity;

        if (!find_word_in_list(hashtable->lists + hash, word, 1))
                return NO_ELEMENT;

        return 0;
}

```

<pre>
            846.58 msec task-clock:u                     #    0.992 CPUs utilized             
                 0      context-switches:u               #    0.000 /sec                      
                 0      cpu-migrations:u                 #    0.000 /sec                      
             4,859      page-faults:u                    #    5.740 K/sec                     
     2,843,655,572      cycles:u                         #    3.359 GHz                       
     4,748,420,225      instructions:u                   #    1.67  insn per cycle            
       722,405,686      branches:u                       #  853.326 M/sec                     
        16,385,370      branch-misses:u                  #    2.27% of all branches 
</pre>

t1 | t2 | t3 | t4 | t5 | \<t\>
---|----|----|----|----|------
0.835 | 0.830 | 0.847 | 0.830 | 0.838 | 0.836


Time performance boost is 1%, that can be just error, therefore we stopped optimising this function.

# Prealigned words

If we want to have maximum in searching functions we need to spend more memory to aligne words and compare them using avx registers. 

This part breifly repeats stats from previous part of work but with aligned words before searching them in hashtable.

## Gereral performance

<pre>
          1,592.77 msec task-clock:u                     #    0.989 CPUs utilized             
                 0      context-switches:u               #    0.000 /sec                      
                 0      cpu-migrations:u                 #    0.000 /sec                      
            13,769      page-faults:u                    #    8.645 K/sec                     
     5,261,223,084      cycles:u                         #    3.303 GHz                       
     5,700,922,097      instructions:u                   #    1.08  insn per cycle            
       951,646,153      branches:u                       #  597.478 M/sec                     
        44,349,616      branch-misses:u                  #    4.66% of all branches           

       1.610622468 seconds time elapsed

       1.560536000 seconds user
       0.029934000 seconds sys

</pre>

t1 | t2 | t3 | t4 | t5 | \<t\>
---|----|----|----|----|------
1.560 | 1.526 | 1.560 | 1.492 | 1.498 | 1.527

## Recursion and strcmp

### Recoursion

Stats after recursion deletation:


<pre>

          1,250.74 msec task-clock:u                     #    1.000 CPUs utilized             
                 0      context-switches:u               #    0.000 /sec                      
                 0      cpu-migrations:u                 #    0.000 /sec                      
            13,759      page-faults:u                    #   11.001 K/sec                     
     4,191,783,655      cycles:u                         #    3.351 GHz                       
     4,352,014,349      instructions:u                   #    1.04  insn per cycle            
       667,392,685      branches:u                       #  533.599 M/sec                     
        47,571,859      branch-misses:u                  #    7.13% of all branches           

       1.251244188 seconds time elapsed

       1.226168000 seconds user
       0.023268000 seconds sys


</pre>


t1 | t2 | t3 | t4 | t5 | \<t\>
---|----|----|----|----|------
1.226 | 1.225 | 1.212 | 1.229 | 1.241 | 1.227

Boost in time is 24.5%, branch-missprediction increased in 2.47%.


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

          1,153.14 msec task-clock:u                     #    0.992 CPUs utilized             
                 0      context-switches:u               #    0.000 /sec                      
                 0      cpu-migrations:u                 #    0.000 /sec                      
            13,767      page-faults:u                    #   11.939 K/sec                     
     3,770,674,800      cycles:u                         #    3.270 GHz                       
     4,682,064,324      instructions:u                   #    1.24  insn per cycle            
       670,906,500      branches:u                       #  581.807 M/sec                     
        34,344,398      branch-misses:u                  #    5.12% of all branches           

       1.162184955 seconds time elapsed

       1.106378000 seconds user
       0.043130000 seconds sys

</pre>

t1 | t2 | t3 | t4 | t5 | \<t\>
---|----|----|----|----|------
1.106 | 1.094 | 1.103 | 1.092 | 1.102 | 1.099

Time performance boost is 11.6% and percentage of mispredicted branches reduced by 2.01%.


## Assembler optimisation

Stats after using crc32 in assembler:

<pre>
            815.28 msec task-clock:u                     #    0.999 CPUs utilized             
                 0      context-switches:u               #    0.000 /sec                      
                 0      cpu-migrations:u                 #    0.000 /sec                      
            13,766      page-faults:u                    #   16.885 K/sec                     
     2,686,216,621      cycles:u                         #    3.295 GHz                       
     4,019,561,542      instructions:u                   #    1.50  insn per cycle            
       563,193,526      branches:u                       #  690.797 M/sec                     
        15,696,744      branch-misses:u                  #    2.79% of all branches           

       0.815870937 seconds time elapsed

       0.784871000 seconds user
       0.029916000 seconds sys
</pre>



t1 | t2 | t3 | t4 | t5 | \<t\>
---|----|----|----|----|------
0.785 | 0.795 | 0.791 | 0.788 | 0.783 | 0.788

Execution time reduced by 39.4% and  branch-misses reduced by 2.33%.

### Inlining

Stats after inlining:

<pre>

            808.14 msec task-clock:u                     #    0.999 CPUs utilized             
                 0      context-switches:u               #    0.000 /sec                      
                 0      cpu-migrations:u                 #    0.000 /sec                      
            13,766      page-faults:u                    #   17.034 K/sec                     
     2,660,979,925      cycles:u                         #    3.293 GHz                       
     4,017,279,976      instructions:u                   #    1.51  insn per cycle            
       562,052,742      branches:u                       #  695.493 M/sec                     
        15,738,757      branch-misses:u                  #    2.80% of all branches           

       0.808581397 seconds time elapsed

       0.774355000 seconds user
       0.033211000 seconds sys

</pre>


t1 | t2 | t3 | t4 | t5 | \<t\>
---|----|----|----|----|------
0.774 | 0.778 | 0.785 | 0.776 | 0.785 | 0.780


There we also have time performance boost in 1% and no changes in branch-missprediction.

## Conclusion

Using prealigned words we have 95.8% improvement in time performance and 1.86% in branch-misprediction.
Without prealigned words we have 72.4% boost in time performance and 2.18% in branch-misprediction.

Ded's performance coefficient:
$$ \xi = 1000 \cdot \frac{perf_boost} {asm_lines} = 200 $$










