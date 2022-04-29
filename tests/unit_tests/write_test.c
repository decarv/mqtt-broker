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

#include <stdio.h>
#include <stdlib.h>

int main() {
    char sentence[1000];

    // creating file pointer to work with files
    FILE *fptr;

    // opening file in writing mode
    fptr = fopen("./topics/test", "w");

    // exiting program 
    if (fptr == NULL) {
        printf("Error!");
        exit(1);
    }
    printf("Enter a sentence:\n");
    fgets(sentence, sizeof(sentence), stdin);
    fprintf(fptr, "%s", sentence);
    fclose(fptr);
    return 0;
}
