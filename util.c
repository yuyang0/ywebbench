/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * =======================================================================
 *       Filename:  wrapper.c
 *        Created:  2015-02-02 21:17:05
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang
 *			Email:  yy2012cn@NOSPAM.gmail.com
 * =======================================================================
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

int Socket(const char *host, int clientPort) {
    int sock;
    unsigned long inaddr;
    struct sockaddr_in ad;
    struct hostent *hp;

    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;

    inaddr = inet_addr(host);
    if (inaddr != INADDR_NONE) {
        memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
    } else {
        hp = gethostbyname(host);
        if (hp == NULL)
            return -1;
        memcpy(&ad.sin_addr, hp->h_addr, hp->h_length);
    }
    ad.sin_port = htons(clientPort);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return sock;
    }
    if (connect(sock, (struct sockaddr *)&ad, sizeof(ad)) < 0) {
        return -1;
    }
    return sock;
}

void *Malloc(size_t size) {
	char *ptr = calloc(1, size);
	if (ptr == NULL) {
		perror("malloc error:");
		exit(1);
	}
	return ptr;
}

void *Realloc(void *ptr, size_t size) {
	assert(ptr);
	char *ret = realloc(ptr, size);
	if (!ret) {
		perror("realloc error");
		exit(1);
	}
	return ret;
}


/* return true if the string contain the character
   otherwith return false
 */
static bool contain_char(char *s, char c)
{
    for (; *s; ++s)
    {
        if (*s == c)
        {
            return true;
        }
    }
    return false;
}
/*strip the characters contained in d_chars
  at the beginning of the string
*/
char *lstrip(char *str, char *d_chars)
{
    for (; *str; ++str)
    {
        char c = *str;
        if (!contain_char(d_chars, c))
        {
            break;
        }
    }
    return str;
}
char *rstrip(char *str, char *d_chars)
{
    char *end = str + strlen(str) - 1;
    for (; end >= str; --end)
    {
        char c = *end;
        if (!contain_char(d_chars, c))
        {
            break;
        }
    }
    *(++end) = '\0';
    return str;
}
char *strip(char *str, char *d_chars)
{
    char *start = lstrip(str, d_chars);
    return rstrip(start, d_chars);
}

bool startswith(const char *str, const char *prefix)
{
    if (strncmp(str, prefix, strlen(prefix)))
    {
        return false;
    }
    else
    {
        return true;
    }
}
bool endswith(const char *str, const char *suffix)
{
    if(strlen(str) < strlen(suffix))
        return false;
    const char *p = str + (strlen(str) - strlen(suffix));
    return startswith(p, suffix);
}
