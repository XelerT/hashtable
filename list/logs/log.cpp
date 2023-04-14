#include "log.h"

void paste_img (FILE *html_logs, const char *img)
{
        assert(html_logs);
        assert(img);

        fprintf(html_logs, "<pre>");
        fprintf(html_logs, "<img src=\"%s\" alt=\"graphics\" width = \"800wh\">", img);
        fprintf(html_logs, "</pre>");
}
