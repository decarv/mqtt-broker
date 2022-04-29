/* wrappers.h
 *
 * Autor - Henrique de Carvalho <henriquecarvalho@usp.br>
 */

#ifndef WRAPPERS_H
#define WRAPPERS_H

#define SAFE_FREE(x)                   \
            do {                       \
                if (x != NULL) {       \
                    free(x);           \
                    x = NULL;          \
                }                      \
            } while(0)

rc_t Malloc(void **el, size_t n, size_t s);
rc_t Read(int socket, void *buffer, ssize_t bytes_total);
rc_t Write(int fd, char *buffer, ssize_t bytes_total);

#endif
