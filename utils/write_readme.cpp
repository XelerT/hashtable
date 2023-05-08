#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <assert.h>
#include "../include/config.h"
#include "../include/text.h"

int paste_perf_in_readme ();
float get_perf_timing (const char *buf);

static const int N_FILES2PASTE = 11;

int main ()
{
        return paste_perf_in_readme();
}

const char *TABLE = "Optimisation     | In time allocation, s (reduced by) | Preallocated, s (reduced by)  \n"
                    "-----------------|------------------|-------------         \n"
                    "No               |      %f          |      %f              \n"
                    "Assambly crc32         |      %f (%0.2f)   |      %f  (%0.2f)      \n"
                    "+Inlined Assembly crc32 |      %f (%0.2f)   |      %f  (%0.2f)      \n"
                    "+Cycle instead recursion            |      %f (%0.2f)   |      %f  (%0.2f)      \n"
                    "+AVX strcmp       |      %f (%0.2f)   |      %f  (%0.2f)      \n%n";

int paste_perf_in_readme ()
{
        log_init("hashtable_logs.html");

        char perf_file_name[64] = {};

        FILE *readme_file = fopen("utils/RAW_README.md", "r");
        text_t readme = {};
        get_text(readme_file, &readme, "utils/RAW_README.md");
        fclose(readme_file);

        size_t readme_chars_left = readme.n_chars;
        char *readme_symb = readme.buf;

        size_t new_readme_n_chars = 0;
        char *new_readme = (char*) calloc(readme.n_chars + N_FILES2PASTE * 256 * 4, sizeof(char));
        if (!new_readme)
                return NULL_CALLOC;
        char *symb_ptr   = new_readme;

        float timing[N_FILES2PASTE] = {};

        for (int i = 1; i < N_FILES2PASTE; i++) {
                sprintf(perf_file_name, "perf_stats/perf_stat%d.txt", i);
                FILE *input_file = fopen(perf_file_name, "r");
                text_t text = {};
                get_text(input_file, &text, perf_file_name);
                fclose(input_file);
                timing[i] = get_perf_timing(text.buf);

                while (readme_chars_left) {
                        if (!strncmp(readme_symb, "!*!*!*", 6)) {
                                memcpy(symb_ptr, text.buf, text.n_chars);
                                readme_symb       += 12 + 2;
                                readme_chars_left -= 12 + 2;
                                symb_ptr += text.n_chars;
                                new_readme_n_chars += text.n_chars;
                                break;
                        } else {
                                *symb_ptr = *readme_symb;
                                symb_ptr++;
                                readme_symb++;
                                new_readme_n_chars++;
                                readme_chars_left--;
                        }
                }
                text_dtor(&text);
        }

        int chars_from_table = 0;
        while (readme_chars_left) {
                if (!strncmp(readme_symb, "!*!*!*TABLE", 11)) {
                        sprintf(symb_ptr, TABLE, timing[1], timing[6], timing[2], ((timing[1] / timing[2])), timing[7], ((timing[6] / timing[7])),
                                                                       timing[3], ((timing[1] / timing[3])), timing[8], ((timing[6] / timing[8])),
                                                                       timing[4], ((timing[1] / timing[4])), timing[9], ((timing[6] / timing[9])),
                                                                       timing[5], ((timing[1] / timing[5])), timing[10], ((timing[6] / timing[10])),
                                                                        &chars_from_table);
                        readme_symb       += 17;
                        readme_chars_left -= 17;
                        symb_ptr += chars_from_table;
                        new_readme_n_chars += chars_from_table;
                        break;
                } else {
                        *symb_ptr = *readme_symb;
                        symb_ptr++;
                        readme_symb++;
                        new_readme_n_chars++;
                        readme_chars_left--;
                }
        }

        while (readme_chars_left) {
                *symb_ptr = *readme_symb;
                symb_ptr++;
                readme_symb++;
                new_readme_n_chars++;
                readme_chars_left--;
        }

        FILE *output = fopen("README.md", "w");
        fwrite(new_readme, sizeof(char), new_readme_n_chars, output);
        fclose(output);

        free(new_readme);

        text_dtor(&readme);

        log_dtor();

        return 0;
}

float get_perf_timing (const char *buf)
{
        assert(buf);

        float timing = 0.f;
        const char *pattern = "seconds";

        const char *str_timing = strstr(buf, pattern);
        while (*str_timing != '\n')
                str_timing--;
        sscanf(str_timing, "%f", &timing);

        return timing;
}
