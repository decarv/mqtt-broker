/* config.c - Contém as definições de macros de configuração do broker.
 *
 * Autor - Henrique de Carvalho <henriquecarvalho@usp.br>
 */

#ifndef CONFIG_H
#define CONFIG_H

/* PRIVADO - essas configs não podem ser alteradas sem consequências para a execução do programa.
 */
#define MQTT_VERSION                "3.1.1"
#define LISTENQ                     32

#define FILEPATH_MAX_LENGTH         256

/* PUBLICO - essas configs podem ser alteradas pelo usuário sem comprometer a execução do
 * programa.
 */
#define LOGGING                     1
#define TERMCOLORS                  1
#define TMP_FILEPATH                "/tmp/mqtt-topics-\0" /* Caminho de tópicos */


#endif

