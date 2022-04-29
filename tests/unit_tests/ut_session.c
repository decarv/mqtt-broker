/* ut_session.c
 *
 * Autor
 *   Henrique de Carvalho <henriquecarvalho@usp.br>
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

#include "handlers.h"
#include "session.h"
#include "error.h"
#include "log.h"
#include "wrappers.h"
#include "topictable.h"


int main(void) {
    struct session_data *session;
    session_new(&session);
    assert(session != NULL);
    session->pid               = 0;
    session->connfd            = 0;
    session->connected         = false;
    session->session_present   = false;
    assert(session->packet == NULL);
    packet_new(&session->packet);
    assert(session->packet != NULL);
    session->packet->type      = 1;
    session->packet->payloadlen        = 5;
    assert(session->packet->fixed_header == NULL);
    if ((session->packet->fixed_header = malloc((FIXED_HEADER_MAX_LENGTH +
                                                 VARIABLE_HEADER_MAX_LENGTH + 1)
         * sizeof(uint8_t))) == NULL);
    assert(session->packet->fixed_header != NULL);

    session_free(session);
}
