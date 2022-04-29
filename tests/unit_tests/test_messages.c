/* __template.c
 *   This file does exactly what it is supposed to, nothing more, nothing
 *   less.
 *
 * Refs.
 *   https://example.com
 *
 * Autor / Author
 *   Henrique de Carvalho <henriquecarvalho@usp.br>
 *   11819104
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

enum rc_t {
    BROKER_ERROR = -1,
    BROKER_OK    =  0
};
typedef enum rc_t rc_t;

rc_t serialize_message(uint32_t n, uint8_t *buffer, ...)
{
    size_t bufsize = 128;
    buffer = calloc(bufsize, sizeof(uint8_t));
    va_list vl;
    ssize_t buflen = 0;
    ssize_t len;
    va_start(vl, n);
    while (n--) {
        uint8_t *next = va_arg(vl, uint8_t*);
        len = strlen(next);
        if (len+buflen > bufsize) {
            bufsize *= 2;
            buffer = realloc(buffer, bufsize*2);
        }
        for (int i = 0; i < len; i++) {
            buffer[buflen++] = next[i];
        }
    }
    va_end(vl);
    return BROKER_OK;
}

int main(int argc, char **argv)
{
    uint8_t *buffer;
    uint8_t fh[3] = {0x64, 0x65, 0x00};
    uint8_t vh[3] = {0x66, 0x67, 0x00};

    serialize_message(2, buffer, fh, vh);
    return 0;
}

