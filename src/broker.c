/* broker.c - Contém o loop principal do broker
 *
 * Autor - Henrique de Carvalho <henriquecarvalho@usp.br>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/prctl.h>
#include <signal.h>
#include <errno.h>

#include "rc.h"
#include "log.h"
#include "utils.h"
#include "config.h"
#include "internal.h"

static volatile int listenfd;
char    *topics_filepath;
ssize_t topics_filepath_length;

void sig_handler(int signo) {
    if (signo == SIGINT) {
        close(listenfd);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr,"Uso: %s <Porta>\n", argv[0]);
        return EXIT_FAILURE;
    }

    LOG(INFO, "Iniciando MQTT Broker v.%s. Pressione Ctrl-C para terminar.\n", MQTT_VERSION);

    topics_filepath = malloc(FILEPATH_MAX_LENGTH * sizeof(char));
    if (topics_filepath == NULL) {
        LOG(ERROR, "topics_filepath alloc.\n");
        return EXIT_FAILURE;
    }

    rc_t rc;
    if ((rc = create_topic_dir(topics_filepath, TMP_FILEPATH, FILEPATH_MAX_LENGTH)) != BROKER_OK) {
        return EXIT_FAILURE;
    }
    topics_filepath_length = strlen(topics_filepath);
    LOG(INFO, "Criado diretório de tópicos em : %s.\n", topics_filepath);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        LOG(ERROR, "socket.\n");
    }
    const uint32_t port = atoi(argv[1]);
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        LOG(ERROR, "bind.\n");
        return EXIT_FAILURE;
    }
    if (listen(listenfd, LISTENQ) < 0) { /* LISTENQ: int backlog -> queue */
        LOG(ERROR, "listen.\n");
        return EXIT_FAILURE;
    }
    LOG(INFO, "Escutando na porta %d.\n", port);

    signal(SIGINT, sig_handler);
    int connfd;
    pid_t pid, parent_pid = getpid();
    while (true) {
        connfd = accept(listenfd, (struct sockaddr *) NULL, NULL);
        if (connfd < 0) {
            if (errno == EBADF) {
                break;
            }
            LOG(ERROR, "accept.\n");
        } else {
            pid = fork();
            if (pid < 0) {
                LOG(ERROR, "fork.\n");
            } else if (pid == 0) {
                int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
                if (r < 0) {
                    return SYSTEM_ERROR;
                } else if (getppid() != parent_pid) {
                    return BROKER_DISCONNECT;
                }
                close(listenfd);
                if ((rc = session_init(connfd)) != BROKER_DISCONNECT) {
                    return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
            } else {
                close(connfd);
            }
        }
    }
    LOG(INFO, "Encerrando MQTT broker.\n");
    return EXIT_SUCCESS;
}
