#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <assert.h>
#include <ctype.h>

#include "include/text.h"


int get_text (FILE *input, text_t *text, const char *file_name)
{
        if (!input) {
                log(1, "File pointer is null.");
                return NULL_FILE_PTR;
        }
        if (!text) {
                log(1, "Text pointer is null.");
                return NULL_TEXT_PTR;
        }

        struct stat file = {};
        if (stat(file_name, &file) < 0)
                return FILE_ERR;

        size_t n_chars = 0;
        char *buf = (char*) calloc(file.st_size + 1, sizeof(char));
        if (!buf) {
                log(1, "Calloc returned NULL.");
                return NULL_CALLOC;
        }

        n_chars = fread(buf, sizeof(char), file.st_size, input);
        text->n_chars = n_chars;

        text->buf = buf;
        text->n_lines = file.st_size - n_chars;
        if (!n_chars) {
                log(1, "Input file is empty.");
                return EMPTY_FILE;
        }
        return 0;
}

void replace_n (text_t *text)
{
        if (!text) {
                log(1, "Text pointer is null.");
                return;
        }

        char *buf = text->buf;

        for (size_t i = 0; i < text->n_chars; i++)
                if (text->buf[i] == '\n')
                        buf[i] = '\0';
}

void delete_punctuations (text_t *text)
{
        assert(text);

        char *buf = text->buf;

        for (size_t i = 0; i < text->n_chars; i++)
                if (buf[i] != ' ' && !isalpha(buf[i]) && buf[i] != '\'') {
                        buf[i] = '\0';
                } else if (buf[i] == ' ') {
                        buf[i] = '\0';
                        if (isalpha(buf[i - 1]))
                                text->n_words++;
                }
}

void divide_text_on_lines (text_t *text)
{
        if (!text) {
                log(1, "Text pointer is null.");
                return;
        }

        char *buf = text->buf;
        buf[text->n_chars + 1] = '\0';

        line_t *lines = (line_t*) calloc(text->n_lines + 1, sizeof(line_t));
        if (!lines) {
                log(1, "Calloc returned NULL.");
                return;
        }
        text->lines = lines;
        for (size_t i = 0; i < text->n_lines; i++) {
                lines[i].ptr = buf;
                while (*buf != '\n' && *buf != '\0')
                        buf++;
                buf++;
                lines[i].length = buf - lines[i].ptr;
        }
}

void divide_text_on_words (text_t *text)
{
        if (!text) {
                log(1, "Text pointer is null.");
                assert(text);
        }

        char *buf = text->buf;

        word_t *words = (word_t*) calloc(text->n_words, sizeof(word_t));
        if (!words) {
                log(1, "Calloc returned NULL.");
                assert(words);
        }
        text->words = words;
        for (size_t i = 0, n = 0; i < text->n_chars; i++)      {
                if (*buf != '\0') {
                        words[n].ptr = buf;
                        while (*buf != '\0') {
                                buf++;
                                i++;
                        }
                        buf++;
                        words[n].length = buf - words[n].ptr;
                        $d(n)
                        n++;
                } else {
                        buf++;
                }
        }
}

void text_dtor (text_t *text)
{
        assert(text);

        free(text->words);
        text->words = nullptr;

        free(text->lines);
        text->lines = nullptr;

        free(text->buf);
        text->buf  = nullptr;
}