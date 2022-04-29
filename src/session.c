/* session.c - Contém as definições das funções que operam sobre a sessão entre o cliente e o
 *             broker.
 *
 * Autor - Henrique de Carvalho <henriquecarvalho@usp.br>
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>

#include "rc.h"
#include "log.h"
#include "internal.h"
#include "wrappers.h"
#include "protocol.h"

extern char    *topics_filepath;
extern ssize_t topics_filepath_length;

rc_t session_init(int connfd)
{
    LOG(INFO, "[%d] Nova sessão.\n", getpid());

    rc_t rc;
    struct session_data *session;
    if ((rc = session_new(&session)) != BROKER_OK) {
        return rc;
    }
    session->connfd            = connfd;
    session->connected         = false;
    session->session_present   = false;
    session->packet            = NULL;

    rc = handle_message_loop(session);

    LOG(INFO, "[%d] Encerrando a sessão. \n", getpid());
    close(session->connfd);
    session_free(session);
    return rc;
}

rc_t session_new(struct session_data **session)
{
    if ((*session = malloc(sizeof(struct session_data))) == NULL) {
        return SYSTEM_ERROR;
    }
    return BROKER_OK;
}

void session_free(struct session_data *session)
{
    if (session != NULL) {
        packet_free(session->packet);
        SAFE_FREE(session);
    }
}

rc_t packet_init(struct session_data *session)
{
    rc_t rc;
    if ((rc = packet_new(&session->packet)) != BROKER_OK) {
        goto error;
    }
    if ((session->packet->fixed_header = malloc(FIXED_HEADER_MAX_LENGTH *
                                                sizeof(uint8_t))) == NULL) {
        goto error;
    }
    memset(session->packet->fixed_header, 0, FIXED_HEADER_MAX_LENGTH);
    if ((rc = Read(session->connfd,
                   session->packet->fixed_header,
                   1)) != BROKER_OK) {
        goto error;
    }
    if ((rc = deserialize_fixed_header(session)) != BROKER_OK) {
        goto error;
    }
    /* TODO: Isso pode melhorar. */
    if (session->packet->type == PINGREQ) {
        return BROKER_OK;
    }

    if ((rc = deserialize_variable_header(session)) != BROKER_OK) {
        goto error;
    }
    if ((rc = deserialize_payload(session)) != BROKER_OK) {
        goto error;
    }

    /* Implementação ignorada -----
        if (session->packet->clean_session == true) {
            packet_free(session->packet);
        }
        ---------------------------
     */
    return BROKER_OK;

error:
    LOG(ERROR, "[%d] Erro em leitura do pacote (cód. %d)\n", getpid(), rc);
    packet_free(session->packet);
    return rc;
}

rc_t packet_new(struct control_packet **packet)
{
    if ((*packet = malloc(sizeof(struct control_packet))) == NULL) {
        LOG(ERROR, "alocação do packet.\n");
        return SYSTEM_ERROR;
    }
    return BROKER_OK;
}

void packet_free(struct control_packet *packet)
{
    if (packet != NULL) {
        SAFE_FREE(packet->payload);
        SAFE_FREE(packet->fixed_header);
        SAFE_FREE(packet);
    }
}

rc_t deserialize_fixed_header(struct session_data *session)
{
    rc_t rc;
    session->packet->type = (session->packet->fixed_header[0] & 0xF0) >> 4;
    if ((rc = validate_type(session->packet->type)) != BROKER_OK) {
        return BROKER_ERROR;
    }
    session->packet->fixed_header_flags = session->packet->fixed_header[0] & 0x0F;
    if (session->packet->type == PUBLISH) {
        session->packet->dup    = session->packet->fixed_header_flags & 0x08;
        session->packet->qos    = session->packet->fixed_header_flags & 0x06;
        session->packet->retain = session->packet->fixed_header_flags & 0x01;
    }
    if ((rc = validate_fixed_header_flags(session)) != BROKER_OK) {
        return BROKER_ERROR;
    }

    session->packet->remaininglen = decode_remaining_length(session);
    session->packet->payloadlen = session->packet->remaininglen;
    if ((rc = validate_remaining_length(session->packet->remaininglen)) != BROKER_OK) {
        return BROKER_ERROR;
    }

    return BROKER_OK;
}

rc_t deserialize_variable_header(struct session_data *session)
{
    if (session->packet->remaininglen == 0) {
        return BROKER_OK;
    }
    switch (session->packet->type) {
        case CONNECT:
            return deserialize_variable_header_connect(session);
        case PUBLISH:
            return deserialize_variable_header_publish(session);
        case SUBSCRIBE:
            return deserialize_variable_header_subscribe(session);
    }
    return BROKER_OK;
}

rc_t deserialize_variable_header_connect(struct session_data *session)
{
    rc_t rc;
    ssize_t length = CONNECT_VARIABLE_HEADER_LENGTH;
    if ((session->packet->variable_header = malloc(length * sizeof(uint8_t))) == NULL) {
        return SYSTEM_ERROR;
    }
    if ((rc = variable_header_read(session, length, 0)) != BROKER_OK) {
        return rc;
    }
    /* Implementação ignorada ------------------------------------------------
     * uint8_t connect_flags = session->packet->variable_header[7];
     * uint8_t clean_session = connect_flags & 0x02;
     * uint8_t keep_alive[2] = {session->packet->variable_header[8],
     *                          session->packet->variable_header[9]};
     * -----------------------------------------------------------------------
     */
    if ((rc = validate_variable_header_connect(session)) != BROKER_OK) {
        return rc;
    }
    return BROKER_OK;
}

rc_t validate_variable_header_connect(struct session_data *session)
{
    /* Implementação ignorada ------------------------------------------------
     * const char PROTOCOL_NAME[7] = {0x00, 0x04, 'M', 'Q', 'T', 'T'};
     * if (!strncmp(session->packet->variable_header, PROTOCOL_NAME, 6)) {
     *     LOG(WARNING, "[%d] Especificação do pacote CONNECT em desacordo"
     *                  "[MQTT-3.1.2-1]\n", session->pid);
     *     return BROKER_ERROR;
     * }
     * uint8_t protocol_level = session->packet->variable_header[6];
     * if (protocol_level != PROTOCOL_LEVEL) {
     *     LOG(WARNING, "[%d] Versão de protocolo não suportada"
     *                  "[MQTT-3.1.2-2]\n", session->pid);
     *     (void) send_connack(session, CONNECTION_REFUSED_PROTOCOL);
     *     return BROKER_ERROR;
     * }
     * -----------------------------------------------------------------------
     */
    return BROKER_OK;
}

rc_t deserialize_variable_header_subscribe(struct session_data *session)
{
    rc_t rc;
    if ((rc = variable_header_read(session, SUBSCRIBE_VARIABLE_HEADER_LENGTH, 0)) !=
            BROKER_OK) {
        return rc;
    }
    if ((rc = validate_variable_header_subscribe(session)) != BROKER_OK) {
        return rc;
    }
    return BROKER_OK;
}

rc_t validate_variable_header_subscribe(struct session_data *session)
{
    return BROKER_OK;
}

rc_t deserialize_variable_header_publish(struct session_data *session)
{
    rc_t rc;
    const uint32_t LENGTH_BYTES_SIZE = 2;
    if ((session->packet->variable_header = malloc(LENGTH_BYTES_SIZE * sizeof(uint8_t))) == NULL) {
        return SYSTEM_ERROR;
    }
    if ((rc = variable_header_read(session, LENGTH_BYTES_SIZE, 0)) != BROKER_OK) {
        return rc;
    }

    session->topicnamelen = (((uint16_t) session->packet->variable_header[0]) << 8) |
                              (uint16_t) session->packet->variable_header[1];

    if (realloc(session->packet->variable_header, (LENGTH_BYTES_SIZE+session->topicnamelen) * sizeof(uint8_t)) == NULL) {
        return SYSTEM_ERROR;
    }
    if ((rc = variable_header_read(session, session->topicnamelen, LENGTH_BYTES_SIZE)) != BROKER_OK) {
        return rc;
    }

    /* Implementação ignorada -------
     * if (qos > 0) { }
     * ------------------------------
     */
    
    session->topicfp = malloc((session->topicnamelen + topics_filepath_length) * sizeof(char));
    if (session->topicfp == NULL) {
        return SYSTEM_ERROR;
    }
    session->topicfp[0] = '\0';
    strncat(session->topicfp, topics_filepath, topics_filepath_length);
    strncat(session->topicfp,
            session->packet->variable_header+LENGTH_BYTES_SIZE,
            session->topicnamelen);
    return BROKER_OK;
}

rc_t variable_header_read(struct session_data *session, ssize_t length, ssize_t
        offset)
{
    rc_t rc;
    if ((rc = Read(session->connfd, session->packet->variable_header + offset,
                   length)) != BROKER_OK) {
        return rc;
    }
    session->packet->payloadlen = session->packet->payloadlen - length;
    return BROKER_OK;
}

rc_t deserialize_payload(struct session_data *session)
{
    rc_t rc;
    if (session->packet->payloadlen == 0) {
        return BROKER_OK;
    }
    session->packet->payload = malloc(sizeof(uint8_t) * session->packet->payloadlen);
    if ((rc = Read(session->connfd, session->packet->payload,
                   session->packet->payloadlen)) != BROKER_OK) {
        return rc;
    }

    /* AVISO IMPLEMENTAÇÃO IGNORADA: Essa implementação ignora a possibilidade de serem enviados
     * vários tópicos por um único cliente ou enviados vários tópicos a um cliente.
     */
    switch (session->packet->type) {
        case PUBLISH:
            return deserialize_payload_publish(session);
        case SUBSCRIBE:
            return deserialize_payload_subscribe(session);
        default:
            return BROKER_OK;
    }
    return BROKER_OK;
}

rc_t deserialize_payload_publish(struct session_data *session)
{

    session->packet->payloadmsg = malloc(session->packet->payloadlen * sizeof(char));
    session->packet->payloadmsg = strndup(session->packet->payload,
            session->packet->payloadlen);
    session->packet->payloadmsg[session->packet->payloadlen] = '\0';
    /* Implementação ignorada -----------------------------------
     *   session->packet->qos = *(session->packet->payload + LENGTH_BYTES + topicnamelen);
     * ----------------------------------------------------------
     */
    return BROKER_OK;
}

rc_t deserialize_payload_subscribe(struct session_data *session)
{
    rc_t rc;
    session->topicnamelen = (((uint16_t) session->packet->payload[0]) << 8) |
                             (uint16_t) session->packet->payload[1];
    if ((session->topicfp = malloc((session->topicnamelen + topics_filepath_length + 1) *
                     sizeof(char))) == NULL) {
        return SYSTEM_ERROR;
    }

    const uint32_t LENGTH_BYTES_SIZE = 2;
    session->topicfp[0] = '\0';
    strncat(session->topicfp, topics_filepath, topics_filepath_length);
    strncat(session->topicfp,
            session->packet->payload+LENGTH_BYTES_SIZE,
            session->topicnamelen);

    if ((session->topicname = malloc((session->topicnamelen + 1) *
                     sizeof(char))) == NULL) {
        return SYSTEM_ERROR;
    }
    session->topicname[0] = '\0';
    strncat(session->topicname,
            session->packet->payload+LENGTH_BYTES_SIZE,
            session->topicnamelen);

    /* Implementação ignorada ------
     * if (qos > 0) { }
     * -----------------------------
     */
    return BROKER_OK;
}

rc_t validate_payload(struct session_data *session)
{
    /* Implementação ignorada -----------------------------
     * switch(session->packet->type) {
     *     case SUBSCRIBE:
     *         return validate_payload_subscribe(session);
     *     case CONNECT:
     *     case DISCONNECT:
     *     case PUBLISH:
     *     case UNSUBSCRIBE:
     *     case CONNACK:
     *     case PUBACK:
     *     case PUBREC:
     *     case PUBCOMP:
     *     case SUBACK:
     *     case PINGREQ:
     *     case PINGRESP:
     *     case PUBREL:
     *         LOG(ERROR, "Mensagem proibida ou não aceita.\n");
     *         return BROKER_ERROR;
     * }
     * ------------------------------------------------------
     */
    return BROKER_OK;
}

rc_t validate_payload_subscribe(struct session_data *session)
{
    /* Implementação ignorada -------------------------------
     * rc_t rc;
     * if (session->packet->payloadlen <= 0) {
     *     return BROKER_ERROR;
     * }
     * ------------------------------------------------------
     */
    return BROKER_OK;
}

ssize_t decode_remaining_length(struct session_data *session)
{
    uint32_t multiplier = 1;
    uint8_t i = 1;
    size_t value = 0;
    do {
        if (Read(session->connfd, &(session->packet->fixed_header[i]), 1) != BROKER_OK) {
            return -1;
        }
        value += (session->packet->fixed_header[i] & 0x7F) * multiplier;
        if (multiplier > 128 * 128 * 128) {
            return -1;
        }
        multiplier *= 128;
    } while ((session->packet->fixed_header[i++] & 0x80) != 0);
    return value;
}

rc_t encode_remaining_length(uint8_t *buffer, ssize_t *buflen, uint32_t size)
{
    if (size < 0 || size > REMAINING_LENGTH_MAX) {
        return BROKER_ERROR;
    }
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
    return BROKER_OK;
}

rc_t validate_type(uint8_t type)
{
    if (type < CONNECT || type > DISCONNECT) {
        LOG(ERROR, "Tipo de pacote reservado.\n"); /* TODO: 4.8 */
        return BROKER_ERROR;
    }
    return BROKER_OK;
}

rc_t validate_fixed_header_flags(struct session_data *session)
{
    /* Implementação ignorada ------------------------------------------------------
     *switch(session->packet->type) {
     *    case CONNECT:
     *    case CONNACK:
     *    case PUBACK:
     *    case PUBREC:
     *    case PUBCOMP:
     *    case SUBACK:
     *    case PINGREQ:
     *    case PINGRESP:
     *    case DISCONNECT:
     *        if (session->packet->fixed_header_flags != 0x00) {
     *            LOG(ERROR, "Flags do cabeçalho fixo inconsistentes para"
     *                       "tipo %d [MQTT-2.2.2-1].\n", session->packet->type);
     *            return BROKER_ERROR;
     *        }
     *        break;

     *    case PUBREL:
     *    case SUBSCRIBE:
     *    case UNSUBSCRIBE:
     *        if (session->packet->fixed_header_flags != 0x02) {
     *            LOG(ERROR, "Flags do cabeçalho fixo inconsistentes para"
     *                       "tipo %d [MQTT-2.2.2-1] [MQTT-3.8.1-1].\n", session->packet->type);
     *            return BROKER_ERROR;
     *        }
     *        break;
     *    case PUBLISH:
     *        if (session->packet->qos == 0x3) {
     *            LOG(ERROR, "PUBLISH PACKET não PODE ter QoS 3 [MQTT-3.3.1-4].\n");
     *            return BROKER_ERROR;
     *        }
     *        if (session->packet->qos != 0 && session->packet->dup == 0) {
     *            LOG(ERROR, "DUP DEVE ser 0 para todo QoS 0 [MQTT-3.3.1-2].\n");
     *            return BROKER_ERROR;
     *        }
     *}
     * ----------------------------------------------------------------------------
     */
    return BROKER_OK;
}

rc_t validate_remaining_length(uint32_t remaininglen)
{
    if (0 > remaininglen || remaininglen > REMAINING_LENGTH_MAX) {
        return BROKER_ERROR;
    }
    return BROKER_OK;
}
