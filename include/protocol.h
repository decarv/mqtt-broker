/* protocol.h
 *
 * Refs.
 *  Algumas escolhas de design foram baseadas no código do mosquitto.
 *
 * Autor - Henrique de Carvalho <my@email.com>
 */

/* Definições Gerais
 *  Definições de tamanhos para recebimento de mensagens e tamanhos máximos de
 *  buffers, definidos com base no padrão MQTT OASIS.
 */

#define PROTOCOL_LEVEL                         0x04

/* Tamanhos dos pacotes - servem para deserializar e serializar os pacotes de mensagens.
 */
#define FIXED_HEADER_MIN_LENGTH                   2
#define FIXED_HEADER_MAX_LENGTH                   6
#define VARIABLE_HEADER_MIN_LENGTH                0
#define VARIABLE_HEADER_MAX_LENGTH                6
#define REMAINING_LENGTH_MAX              268435456

#define CONNECT_VARIABLE_HEADER_LENGTH           10
#define SUBSCRIBE_VARIABLE_HEADER_LENGTH          2

#define PUBLISH_BUFFER_LENGTH                  1024
#define SUBACK_BUFFER_LENGTH                      5
#define CONNACK_BUFFER_LENGTH                     4
#define PINGRESP_BUFFER_LENGTH                    2
#define PINGREQ_BUFFER_LENGTH                     2



#define SUBACK_SUCCESS_QOS_0                   0x00
#define KEEP_ALIVE_SLEEP                          5


/* Tipos de Pacotes de Controle do MQTT
 */
#define RSRVD1                                 0x00
#define CONNECT                                0x01
#define CONNACK                                0x02
#define PUBLISH                                0x03
#define PUBACK                                 0x04
#define PUBREC                                 0x05
#define PUBREL                                 0x06
#define PUBCOMP                                0x07
#define SUBSCRIBE                              0x08
#define SUBACK                                 0x09
#define UNSUBSCRIBE                            0x0A
#define UNSUBACK                               0x0B
#define PINGREQ                                0x0C
#define PINGRESP                               0x0D
#define DISCONNECT                             0x0E
#define RSRVD2                                 0x0F

/* CONNECT Return Codes */
#define CONNECTION_ACCEPTED                    0x00
#define CONNECTION_REFUSED_PROTOCOL            0x01

