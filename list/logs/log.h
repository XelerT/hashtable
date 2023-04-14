#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <assert.h>
#include <sys/stat.h>

struct logs_t {
        char *log_file_path = nullptr;
        FILE *log_file      = nullptr;
        char *buf           = nullptr;
        size_t n_chars      =       0;
};

enum log_errors {
        NULL_OUTPUT_PTR = -1,
        NULL_INPUT_PTR  = -2,
        NULL_IMG_PTR    = -3,
        NULL_LOG_FILE   = -4
};

void paste_img (FILE *html_logs, const char *img);

#endif /*LOG_H*/
