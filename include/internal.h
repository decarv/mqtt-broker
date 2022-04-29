/* internal.h - Declara as estruturas e funções responsáveis por uma sessão entre cliente e
 *              servidor, pela criação e manutenção das estruturas de dados internas.
 *
 * Autor - Henrique de Carvalho <my@email.com>
 */

#ifndef SESSION_H
#define SESSION_H

#include <stdbool.h>
#include <stdlib.h>

#include "rc.h"

#define BUFFER_READ_LENGTH                     4096
#define TOPIC_MAX_LENGTH                        256

#define READ_SLEEP                                1

/* Structs internas usadas para passar dados entre as funções.
 */
struct session_data {
    /* Dados da conexão */
    int                connfd;
    bool               connected;  /* se um MQTT CONNECT foi estabelecido */
    bool               session_present;

    /* Dados do tópico em que se publica ou em que se inscreve */
    FILE               *topic;
    char               *topicfp;
    char               *topicname;
    uint16_t           topicnamelen;

    struct             control_packet *packet;
};

struct control_packet {
    uint8_t type;
    uint8_t fixed_header_flags;
    ssize_t remaininglen;
    ssize_t payloadlen;

    /* Flags de connect */
    uint8_t dup;
    uint8_t qos;
    uint8_t retain;
    uint8_t clean_session;

    int8_t    *fixed_header;
    int8_t    *variable_header;
    int8_t    *payload;
    char      *payloadmsg;
};

/* Função: session_init - inicia e administra uma sessão entre o cliente e o broker.
 *
 * Parâmetros:
 *  int connfd - descritor do arquivo da conexão entre o cliente e o processo de
 *               pid_t pid.
 *  pid_t pid  - id do processo responsável pela sessão.
 *
 * Retorna:
 *  rc_t rc - código de retorno para controle do fluxo.
 */
rc_t session_init(int connfd);

/* Função: packet_init - deserializa uma mensagem e inicia um pacote.
 *
 * Parâmetros:
 *  struct session_data *session (mod.): o pacote será colocado na estrutura da sessão.
 *
 * Retorna:
 *  rc_t rc - código de retorno para controle do fluxo.
 */
rc_t packet_init(struct session_data *session);

/* Função: {struct}_new, {struct}_free
 *  {aloca,libera} recursos de memória de uma sessão
 *
 * Parâmetros:
 *  struct * : ponteiro que será {alocado,liberado}
 *
 * Retorna:
 *  rc_t rc - código de retorno para controle do fluxo.
 */
rc_t session_new(struct session_data **session);
void session_free(struct session_data *session);
rc_t packet_new(struct control_packet **packet);
void packet_free(struct control_packet *packet);

/* Função: read_packet - chama as funções de deserializar uma mensagem recebida de um cliente e cria
 *                       um pacote de controle
 *
 * Parâmetros:
 *  struct session_data *session: ponteiro para uma sessão
 *
 * Retorna:
 *  rc_t rc - código de retorno para controle do fluxo.
 */
rc_t read_packet(struct session_data *session);

/* Funções: deserialize_{something} - deserializa uma parte específica de uma mensagem
 *
 * Parâmetros:
 *  struct session_data *session: ponteiro para uma sessão
 *
 * Retorna:
 *  rc_t rc - código de retorno para controle do fluxo.
 */
rc_t deserialize_fixed_header(struct session_data *session);
rc_t deserialize_variable_header(struct session_data *session);
rc_t deserialize_variable_header_connect(struct session_data *session);
rc_t deserialize_variable_header_subscribe(struct session_data *session);
rc_t deserialize_variable_header_publish(struct session_data *session);
rc_t deserialize_payload(struct session_data *session);
rc_t deserialize_payload_subscribe(struct session_data *session);
rc_t deserialize_payload_publish(struct session_data *session);

/* Funções: validate_{something} - valida dados deserializados
 *
 * Parâmetros:
 *  {dados a serem validados}
 *
 * Retorna:
 */
rc_t validate_type(uint8_t type);
rc_t validate_fixed_header_flags(struct session_data *session);
rc_t validate_remaining_length(uint32_t remaininglen);
rc_t validate_variable_header_connect(struct session_data *session);
rc_t validate_variable_header_subscribe(struct session_data *session);
rc_t variable_header_read(struct session_data *session, ssize_t length, ssize_t offset);

/* Funções: {encode,decode}_remaining_length
 *  Faz o {encode,decode} do campo remaininglen do cabeçalho fixo pacote
 *  de controle.
 *
 * Parâmetros:
 *  struct session_data *session (mod.): remaininglen é armazenado no pacote
 *  uint8_t *buffer: valor a ser codificado
 *
 * Retorna:
 *  ssize_t size: valor decodificado
 */
ssize_t decode_remaining_length(struct session_data *session);
rc_t encode_remaining_length(uint8_t *buffer, ssize_t *buflen, uint32_t size);

/* Função: handle_session - lida com a sessão chamando as funções do broker
 *  sequencialmente para ler pacote de CONNECT, responder pacote de CONNECT, ler
 *  nova mensagem e responder mensagem.
 *
 * Parâmetros:
 *  struct session_data *session: ponteiro para uma sessão
 *
 * Retorna:
 *  rc_t rc: código de retorno de erro
 */
rc_t handle_message_loop(struct session_data *session);

/* Função: handle_{something} - realiza o manejo de alguma mensagem enviada pelo cliente ao broker.
 *                              handle_message é responsável por avaliar o tipo de pacote.
 *
 * Parâmetros:
 *  struct session_data *session (mod.)
 *
 * Retorna:
 *  rc_t rc - código de retorno para controle do fluxo.
 */
rc_t handle_message(struct session_data *session);
rc_t handle_connect(struct session_data *session);
rc_t handle_disconnect(struct session_data *session);
rc_t handle_publish(struct session_data *session);
rc_t handle_subscribe(struct session_data *session);
rc_t handle_unsubscribe(struct session_data *session);
rc_t handle_pingrec(struct session_data *session);

/* Funções: send_{something} - responsável por serializar e enviar uma mensagem ao cliente.
 *
 * Parâmetros:
 *  struct session_data *session (mod.)
 *  {valores que serão serializados na mensagem}
 *
 * Retorna:
 *  rc_t rc - código de retorno para controle do fluxo.
 */
rc_t send_connack(struct session_data *session, uint8_t connack_rc);
rc_t send_suback(struct session_data *session, uint8_t payload);
rc_t send_publish(struct session_data *session, char *buffer, ssize_t buflen);
rc_t send_pingresp(struct session_data *session);

/* Função: serialize_message - serializa uma mensagem [TODO: Essa função não é utilizada atualmente,
 *                             ou deprecar ou usar].
 *
 * Parâmetros:
 *  n - quantidade de argumentos passados
 *  uint8_t *buffer - primeiro parâmetro para serialização
 *
 * Retorna:
 *  rc_t rc - código de retorno para controle do fluxo.
 */
rc_t serialize_message(uint32_t n, uint8_t *buffer, ...);

/* Funções: verify_connection - verifica o status de uma conexão
 *
 * Parâmetros:
 *  struct session_data *session
 *
 * Retorna:
 *  rc_t rc - código de retorno para controle do fluxo. BROKER_OK se estiver conectado.
 */
rc_t verify_connection(struct session_data *session);

#endif
