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
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <assert.h>
#include <stdio.h>

ssize_t decode_remaining_length(uint8_t *decoded_bytes, char **buffer)
{
    uint32_t multiplier = 1;
    uint8_t i = 0;
    uint32_t value = 0;
    do {
        value += (decoded_bytes[i] & 0x7F) * multiplier;
        if (multiplier > 128 * 128 * 128) {
            return -1;
        }
        multiplier *= 128;
    } while ((decoded_bytes[i++] & 0x80) != 0);
    decoded_bytes[0] -= 2;
    strncat(*buffer, decoded_bytes, i);

    return value;
}

void encode_remaining_length(uint8_t *buffer, ssize_t *buflen, uint32_t size)
{
    uint8_t encoded_byte;
    do {
        encoded_byte = size % 0x80;
        size = size / 0x80;
        if (size > 0) {
            encoded_byte = encoded_byte | 0x80;
        }
        buffer[*buflen] = encoded_byte;
        *buflen += 1;
    } while (size > 0);
}

void printbuf(uint8_t *buffer, ssize_t buflen)
{
    for (int i = 0; i < buflen; i++) {
        printf("%d %x\n", i, buffer[i]);
    }
}

int main(void) {
    char *buffer;

    buffer = malloc(sizeof(char) * 1024);
    buffer[0] = '\0';
    uint8_t A[1] = {0xB0};
    ssize_t a = decode_remaining_length(A, &buffer);
    free(buffer);

    buffer = malloc(sizeof(char) * 1024);
    uint8_t B[2] = {0xFF, 0x7F};
    ssize_t b = decode_remaining_length(B, &buffer);
    assert(b == 16383);
    free(buffer);

    buffer = malloc(sizeof(char) * 1024);
    uint8_t C[3] = {0xFF, 0xFF, 0x7F};
    ssize_t c = decode_remaining_length(C, &buffer);
    assert(c == 2097151);
    free(buffer);
    
    buffer = malloc(sizeof(char) * 1024);
    ssize_t buflen = 0;
    encode_remaining_length(buffer, &buflen, 17);
    printbuf(buffer, buflen);
    free(buffer);

}

