/* log.h - Contém definições de macros e declarações de funções usadas para logging.
 *
 * Autor - Henrique de Carvalho <henriquecarvlho@usp.br>
 */

#ifndef LOGGING_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "config.h"

/* Definições de cores para log no terminal.
 *
 * Ver também: TERMCOLORS @config.h
 */
#define DEFAULT_COLOR "\e[m"
#if TERMCOLORS
#define RED    "\033[0;31m"
#define GREEN  "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE   "\033[0;34m"
#else
#define RED    DEFAULT_COLOR
#define GREEN  DEFAULT_COLOR
#define YELLOW DEFAULT_COLOR
#define BLUE   DEFAULT_COLOR
#endif

/* Definições de log.
 */
#define DEFAULT_COLOR "\e[m"
#define LOGSIZE 256 /* Tamanho máximo da string de log. */

/* Verbosity para funções LOG */
#define INFO    0
#define WARNING 1
#define ERROR   2

/* Wrapper para _log
 *
 * Ver também: LOGGING e DEBUGGING @config.h
 */
#if LOGGING
#define LOG(verbosity, ...)                       \
    _log(verbosity, __VA_ARGS__)
#else
#define LOG(verbosity, ...)                       \
    _void_func()
#endif
#if DEBUGGING
#define DEBUG(...)                                  \
    _log(...)
#else
#define DEBUG(...)                                  \
    void
#endif


/* Função: current_timestring
 *  Armazena uma string com o horário atual em tbuf. Usado para logging.
 *
 * Parâmetros:
 *  char *tbuf : valor a ser modificado
 *  size_t tbuf_size : tamanho máximo de tbuf
 *
 * Retorna:
 *  int rv : a quantidade de bytes armazenados em buf
 */
int current_timestring(char *tbuf, size_t tbuf_size);

/* Funções: _log , _debug_log, _void_func
 *  Funções usadas para imprimir um log na saída.
 *
 * Parâmetros:
 *  uint8_t verbosity : o tipo de log
 *  const char *fmt : string formatada que será impressa.
 */
void _log(uint8_t verbosity, const char *fmt, ...);
void _debug_log(const char *func, const char *fmt, ...);
void _void_func();

#endif
