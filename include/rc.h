/* rc.h - esse arquivo contém os Códigos de Retorno (rc) ou Valores de Retorno (rv)
 *
 * Autor: Henrique de Carvalho <henriquecarvalho@usp.br>
 */

#ifndef RC_H
#define RC_H

enum rc_t {
    SYSTEM_ERROR      = -5,
    BROKER_DISCONNECT = -2,
    BROKER_ERROR      = -1,
    BROKER_OK         =  0
};
typedef enum rc_t rc_t;

#endif
