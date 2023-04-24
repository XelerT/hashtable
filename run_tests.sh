#! /usr/bin/bash

read -p "Enter mode of hashtable tests: " run_mode
optimisations=("NO_OPTIMISATION" "CYCLE_OPTIMISATION" "AVX_OPTIMISATION" "ASM_CRC32_OPTIMISATION" "INL_ASM_CRC32_OPTIMISATION" "PRE_ALIGNED_WORDS" "NOPRE_ALIGNEED_WORDS")

set -e

if [[ "$run_mode" == "graphs" ]]; then

        make -f Makefile graph_hashes DFLAG1=${optimisations[0]}
        make -f Makefile pygraph
elif [[ "$run_mode" == "tests" ]]; then
        for opt in {0..4}; do
                echo DFLAG1=${optimisations[$opt]} DFLAG2=${optimisations[6]}
                make DFLAG1=${optimisations[$opt]} DFLAG2=${optimisations[6]}
                perf stat -r 5 -e cycles,instructions,cache-references,cache-misses,bus-cycles ./hash-tables.out
                echo "================================================================================================"
        done
        echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
        for opt in {0..4}; do
                echo DFLAG1=${optimisations[$opt]} DFLAG2=${optimisations[5]}
                make DFLAG1=${optimisations[$opt]} DFLAG2=${optimisations[5]}
                perf stat -r 5 -e cycles,instructions,cache-references,cache-misses,bus-cycles ./hash-tables.out
                echo "================================================================================================"
        done
else
        echo "No such command."
fi
        echo "Runned script."

:
