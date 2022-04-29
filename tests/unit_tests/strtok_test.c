/* __template.c
 *   This file does exactly what it is supposed to, nothing more, nothing
 *   less.
 *
 * Refs.
 *   https://example.com
 *
 * Autor / Author
 *   Henrique de Carvalho <henriquecarvalho@usp.br>
 *   11819104
 */

#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

int main () {
   char str[128] = "topics/house/temperature";
   const char s[2] = "/";
   char *token = strtok(str, s);
   printf("%d\n", mkdir("a", 0666));
   printf("%d\n", mkdir("e/b/", 0666));
   return(0);
}
