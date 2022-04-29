/* log.c - Contém as definições das funções usadas para logging.
 *
 * Autor - Henrique de Carvalho <henriquecarvalho@usp.br>
 */

#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "log.h"

int current_timestring(char *tbuf, size_t tbuf_size) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t t = (time_t) tv.tv_sec;
    struct tm *tm = localtime(&t);
    int rv = strftime(tbuf, tbuf_size - 1, "%H:%M:%S", tm);
    rv = snprintf(tbuf+rv, tbuf_size - 1, ".%06ld", (long) tv.tv_usec);
    return rv;
}

void _log(uint8_t verbosity, const char *fmt, ...) 
{
    const char *color;
    char prefix[64];
    switch(verbosity) {
        case INFO:
            strcpy(prefix, " ");
            color = DEFAULT_COLOR;
            break;
        case WARNING:
            strcpy(prefix, " Aviso: ");
            color = YELLOW;
            break;
        case ERROR:
            strcpy(prefix, " Erro: ");
            color = RED;
            break;
        default:
            strcpy(prefix, "");
            color = DEFAULT_COLOR;
            break;
    }

    va_list vl;
    va_start(vl, fmt);
    char buffer[LOGSIZE];
    size_t buffer_size = 0;
    char tbuf[64];
    if (current_timestring(tbuf, sizeof(tbuf)) <= 0) {
        tbuf[0] = '\0';
    }
    buffer_size = sprintf(buffer, "%s[%s]%s%s",
                          GREEN,
                          tbuf,
                          color,
                          prefix);

    vsprintf(buffer+buffer_size, fmt, vl);

    fprintf(stdout, "%s", buffer);
    va_end(vl);
}

void _void_func() {}
