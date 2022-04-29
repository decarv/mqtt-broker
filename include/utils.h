/* utils.h - Contém a declaração de funções e macros que são auxiliares.
 *
 * Autor - Henrique de Carvalho <henriquecarvalho@usp.br>
 */

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <ftw.h>

#include "rc.h"

rc_t create_topic_dir(char *pathname, char *pathloc, ssize_t maxpathlen);

#endif

