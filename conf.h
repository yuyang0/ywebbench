/*Time-stamp: <2013-01-15 11:28:12 by Yu Yang>
 * =======================================================================
 *       Filename:  conf.h
 *        Created:  2013-04-14 18:13:20
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang 
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */

#ifndef _CONF_H_
#define _CONF_H_ 1

#include <stdbool.h>
#include "url.h"

#define MAX_URLS 10
/* type */
#define BENCH_SEQ    0
#define BENCH_RANDOM 1
#define BENCH_FIRST  2
/* http method */
#define METHOD_GET 0
#define METHOD_HEAD 1
#define METHOD_OPTIONS 2
#define METHOD_TRACE 3

typedef struct _conf {
    char *proxyHost;
    int proxyPort;
    int type;
    int method;
    int clients;
    int benchtime;
    bool force;
    bool forceReload;
    char *urlStrings[MAX_URLS];
    char *userAgent;
    char *reqBuffers[MAX_URLS];
    int numUrls;
    url_t *urls;
}conf_t;

int conf_init(conf_t *confp, const char *path);
void conf_reset(conf_t *confp);

conf_t *conf_new(const char *path);
int conf_destroy(conf_t *confp);

#endif /* _CONF_H_ */

