/* ut_session_init.c
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


const uint8_t CONNECT_SAMPLE_FIXED_HEADER = {0x01, 0x04, 'M', 'Q', 'T', 'T'};
const uint8_t CONNECT_SAMPLE_VAR_HEADER = {0x00, 0x04, 'M', 'Q', 'T', 'T'};

int main(void) {
    struct session_data *session;
    session_new(&session);

    


    session_free(session);
}
