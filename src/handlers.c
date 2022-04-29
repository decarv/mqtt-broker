/* handlers.c - Define as funções específicas de manejo das comunicações pelo broker.
 *
 * Autor - Henrique de Carvalho <henriquecarvalho@usp.br>
 */

#include <sys/prctl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include "rc.h"
#include "log.h"
#include "config.h"
#include "protocol.h"
#include "internal.h"
#include "wrappers.h"

rc_t handle_message_loop(struct session_data *session)
{
    rc_t rc;
    while (true) {
        if ((rc = packet_init(session)) != BROKER_OK) {
            return rc;
        }
        if ((rc = handle_message(session)) != BROKER_OK) {
            return rc;
        }
        packet_free(session->packet);
    }
    return BROKER_OK;
}

rc_t handle_message(struct session_data *session)
{
    switch(session->packet->type) {
        case CONNECT:
            return handle_connect(session);
        case DISCONNECT:
            return handle_disconnect(session);
        case PUBLISH:
            return handle_publish(session);
        case SUBSCRIBE:
            return handle_subscribe(session);
        case PINGREQ:
            return handle_pingrec(session);
        case UNSUBSCRIBE:
        case CONNACK:
        case PUBACK:
        case PUBREC:
        case PUBCOMP:
        case SUBACK:
        case PINGRESP:
        case PUBREL:
            LOG(ERROR, "Mensagem proibida ou não implementada.\n");
            return BROKER_ERROR;
    }
}

rc_t handle_connect(struct session_data *session)
{
    if (session->connected == true) {
        LOG(ERROR, "[%d] Tentativa de realizar duas conexões"
                   "[MQTT-3.1.0-2].\n", getpid());
        return BROKER_ERROR;
    }
    session->connected = true;
    return send_connack(session, CONNECTION_ACCEPTED);
}

rc_t handle_disconnect(struct session_data *session)
{
    return BROKER_DISCONNECT;
}

rc_t handle_publish(struct session_data *session)
{
    rc_t rc;
    if ((rc = verify_connection(session)) != BROKER_OK) {
        return rc;
    }

    /* Implementação ignorada ------------------------
     *  if (session->packet->retain == 0x01) {
     *      if (session->packet->qos == 0) {
     *          rc = discard_retained(session->packet);
     *      }
     *      rc = store_pub(session->packet);
     *  }
     *  rc = pub_to_sub(session->packet);
     * -----------------------------------------------
     */

    if ((session->topic = fopen(session->topicfp, "w+")) == NULL) {
        LOG(ERROR, "Não foi possível abrir o arquivo");
        return rc;
    }
    fprintf(session->topic, "%s", session->packet->payloadmsg);
    fclose(session->topic);
    return BROKER_DISCONNECT;
}

rc_t handle_subscribe(struct session_data *session)
{
    rc_t rc;
    if ((rc = verify_connection(session)) != BROKER_OK) {
        return rc;
    }
    if ((rc = send_suback(session, SUBACK_SUCCESS_QOS_0)) != BROKER_OK) {
        LOG(WARNING, "Erro em suback");
    }

    /* TODO: pode ser que isso fique melhor dentro de handle_message */
    pid_t parent_pid = getpid();
    pid_t pid;
    pid = fork();
    if (pid < 0) {
        return SYSTEM_ERROR;
    } else if (pid != 0) {
        return BROKER_OK; /* Pai retorna para handle_message_loop */
    } else {
        int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
        if (r < 0) {
            return SYSTEM_ERROR;
        } else if (getppid() != parent_pid) {
            return BROKER_DISCONNECT;
        }

        char c;
        char *buffer;
        ssize_t buflen = 0;
        struct stat statbuf;
        uint64_t last_time_read = time(0);
        uint64_t last_time_modified;
        while (true) {
            while (stat(session->topicfp, &statbuf) != 0) {
            }
            while ((session->topic = fopen(session->topicfp, "r")) == NULL) {
                LOG(ERROR, "Erro ao abrir arquivo.\n");
            }
            last_time_modified = statbuf.st_mtim.tv_sec;
            if (last_time_read < last_time_modified) {
                if ((buffer = malloc(PUBLISH_BUFFER_LENGTH * sizeof(char))) == NULL) {
                    return SYSTEM_ERROR;
                }
                while ((c = fgetc(session->topic)) != EOF) {
                    buffer[buflen++] = c;
                }
                buffer[buflen] = '\0';
                if ((rc = send_publish(session, buffer, buflen)) != BROKER_OK) {
                    return BROKER_ERROR;
                }
                buflen = 0;
                SAFE_FREE(buffer);
                last_time_read = time(0);
            }
            fclose(session->topic);
            sleep(0.1);
        }
        SAFE_FREE(buffer);
    }
    return BROKER_OK;
}

rc_t handle_pingrec(struct session_data *session)
{
    return send_pingresp(session);
}

rc_t send_pingresp(struct session_data *session)
{
    rc_t rc;
    char *buffer;
    if ((buffer = malloc(PINGRESP_BUFFER_LENGTH * sizeof(char))) == NULL) {
        return SYSTEM_ERROR;
    }
    buffer[0] = 0xD0;
    buffer[1] = 0x00;
    if ((rc = Write(session->connfd, buffer, PINGRESP_BUFFER_LENGTH)) != BROKER_OK) {
        SAFE_FREE(buffer);
        return rc;
    }
    SAFE_FREE(buffer);
    return BROKER_OK;
}

rc_t send_publish(struct session_data *session, char *message, ssize_t msglen)
{
    rc_t rc;
    uint8_t *buffer;
    if ((buffer = malloc(PUBLISH_BUFFER_LENGTH * sizeof(char))) == NULL) {
        return SYSTEM_ERROR;
    }
    ssize_t buflen = 0;
    uint32_t remaininglen = session->topicnamelen + 2 + msglen;

    /* Fixed Header */
    buffer[buflen++] = 0x30;
    encode_remaining_length(buffer, &buflen, remaininglen);

    /* Variable Header */
    buffer[buflen++] = (uint8_t) ((session->topicnamelen & 0xFF00) >> 4);
    buffer[buflen++] = (uint8_t)  (session->topicnamelen & 0x00FF);

    for (int i = buflen; i < (buflen+session->topicnamelen); i++) {
        buffer[i] = session->topicname[i-buflen];
    }
    buflen += session->topicnamelen;
    for (int i = buflen; i < (buflen+msglen); i++) {
        buffer[i] = message[i-buflen];
    }
    buflen += msglen;
    if ((rc = Write(session->connfd, buffer, buflen)) != BROKER_OK) {
        SAFE_FREE(buffer);
        return rc;
    }
    SAFE_FREE(buffer);
    return BROKER_OK;
}

rc_t send_connack(struct session_data *session, uint8_t connack_rc)
{
    rc_t rc;
    uint8_t ack_flags;
    if (session->packet->clean_session) {
        ack_flags = 0x00;
    } else if (session->session_present) {
        ack_flags = 0x01;
    } else {
        ack_flags = 0x00;
    }
    uint8_t *buffer;
    if ((buffer = malloc(CONNACK_BUFFER_LENGTH * sizeof(uint8_t))) == NULL) {
        return SYSTEM_ERROR;
    }
    buffer[0] = 0x20;
    buffer[1] = 0x02;
    buffer[2] = ack_flags;
    buffer[3] = connack_rc;
    if ((rc = Write(session->connfd, buffer, CONNACK_BUFFER_LENGTH)) != BROKER_OK) {
        SAFE_FREE(buffer);
        return rc;
    }
    SAFE_FREE(buffer);
    return BROKER_OK;
}

rc_t send_suback(struct session_data *session, uint8_t payload)
{
    rc_t rc;
    uint8_t *buffer;
    if ((buffer = malloc((SUBACK_BUFFER_LENGTH) * sizeof(uint8_t))) == NULL) {
        return SYSTEM_ERROR;
    }

    buffer[0] = 0x90;
    buffer[1] = 0x03;
    buffer[2] = session->packet->variable_header[0];
    buffer[3] = session->packet->variable_header[1];
    buffer[4] = payload;

    if ((rc = Write(session->connfd, buffer, SUBACK_BUFFER_LENGTH)) != BROKER_OK) {
        goto error;
    }
    SAFE_FREE(buffer);
    return BROKER_OK;

error:
    SAFE_FREE(buffer);
    return rc;
}

rc_t verify_connection(struct session_data *session)
{
    if (session->connected == false) {
        LOG(ERROR,
            "[%d] Tentativa de desconexão sem estar conectado", getpid());
        return BROKER_ERROR;
    }
    return BROKER_OK;
}

rc_t serialize_message(uint32_t n, uint8_t *buffer, ...)
{
    size_t bufsize = 128;
    buffer = calloc(bufsize, sizeof(uint8_t));
    va_list vl;
    ssize_t buflen = 0;
    ssize_t len;
    va_start(vl, buffer);
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
