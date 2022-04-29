/*
 * test_log.c
 *
 * Autor
 *  Henrique de Carvalho <henriquecarvalho@usp.br>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#include "../include/config.h"
#include "../include/log.h"

int main(void)
{
    char buffer[50] = "henrique\0";
    LOG(INFO, "This is an info message. This is 4 -> %d , and this is henrique"
              "-> %s\n", 4, buffer);
    LOG(WARNING, "This is an warning message. This is a 5 -> %d... and a 3 -> %d\n", 5, 3);
    LOG(ERROR, "This is an warning message. This is a 99 -> %d\n", 99);
    return 0;
}
