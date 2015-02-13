/*Time-stamp: <2013-01-15 11:28:12 by Yu Yang>
 * =======================================================================
 *       Filename:  wrapper.h
 *        Created:  2015-02-02 21:16:57
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */

#ifndef _WRAPPER_H_
#define _WRAPPER_H_ 1

#include <stdbool.h>

#define Free(ptr)                               \
    do{                                         \
	if (ptr != NULL)                        \
            free(ptr);                          \
	ptr = NULL;                             \
    }while(0)

void *Malloc(size_t size);
void *Realloc(void *ptr, size_t size);

int Socket(const char *host, int clientPort);

char *lstrip(char *str, char *d_chars);
char *rstrip(char *str, char *d_chars);
char *strip(char *str, char *d_chars);

bool startswith(const char *str, const char *prefix);
bool endswith(const char *str, const char *suffix);
#endif /* _WRAPPER_H_ */

