/* wrappers.c - Funções que camuflam operações mais complexas no código.
 *
 * Refs.
 *  https://sandialabs.github.io/Zoltan/ug_html/ug_util_mem.html
 *  https://stackoverflow.com/questions/666601/what-is-the-correct-way-of-reading-from-a-tcp-socket-in-c-c
 *
 * Autor - Henrique de Carvalho <henriquecarvalho@usp.br>
 */

#include <unistd.h>

#include "log.h"
#include "error.h"
#include "internal.h"

rc_t Malloc(void **el, size_t n, size_t s)
{
    *el = malloc(n * s);
    if (*el == NULL) {
        LOG(ERROR, "calloc error.\n");
        return SYSTEM_ERROR;
    }
    return BROKER_OK;
}

rc_t Read(int fd, void *buffer, ssize_t bytes_total)
{
    ssize_t bytes_read = 0;
    ssize_t nread;
    while (bytes_read < bytes_total) {
        nread = read(fd, buffer+bytes_read, bytes_total - bytes_read);
        if (nread < 0) {
            LOG(ERROR, "Erro de leitura do socket (cód. %ld)", nread);
            return SYSTEM_ERROR;
        } else if (nread == 0) {
            sleep(READ_SLEEP);
        }
        bytes_read += nread;
    }
    return BROKER_OK;
}

rc_t Write(int fd, char *buffer, ssize_t bytes_total)
{
    ssize_t bytes_written = 0;
    ssize_t nwrite;
    while (bytes_written < bytes_total) {
        nwrite = write(fd, buffer+bytes_written, bytes_total - bytes_written);
        if (nwrite < 0) {
            LOG(ERROR, "Erro de escrita no socket.\n");
            return SYSTEM_ERROR;
        } else if (nwrite == 0) {
            LOG(ERROR, "Erro de burrice mesmo.\n");
            return BROKER_ERROR;
        }
        bytes_written += nwrite;
    }
    return BROKER_OK;
}




