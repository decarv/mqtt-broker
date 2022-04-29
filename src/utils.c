/* utils.h - Contém a definição de funções e macros que são auxiliares e utilizadas pelo código do 
 *           broker.
 *
 * Autor - Henrique de Carvalho <henriquecarvalho@usp.br>
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "config.h"
#include "rc.h"
#include "log.h"

rc_t create_topic_dir(char *pathname, char *pathloc, ssize_t maxpathlen)
{
    //struct timeval tv;
    //gettimeofday(&tv, NULL);
    //time_t t = (time_t) tv.tv_sec;
    //struct tm *tm = localtime(&t);
    time_t t = time(0);
    int n = snprintf(pathname, maxpathlen, "%s%ld/", pathloc, (long) t);

    if (mkdir(pathname, 0777) == -1) {
        LOG(ERROR, "Não foi possível criar diretório '%s'.", pathname);
        return SYSTEM_ERROR;
    }
    return BROKER_OK;
}

#endif
