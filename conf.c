/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* Time-stamp: <2013-01-15 11:26:38 by Yu Yang>
 * =======================================================================
 *       Filename:  conf.c
 *        Created:  2013-04-14 18:07:47
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#include "util.h"
//#include "str.h"
#include "conf.h"
#define MAXLINE 1024 
#define MAXSIZE 4096 
#define SEP ' '

/* private functions */
static char *getKeyValue(FILE *fp, char *key);
static char *skipHeadChar(char *line, char c);

static bool isCommentLine(char *line);
static bool isBlankLine(char *line);

int getIntegerValue(FILE *fp, char *entry, int defaultVal);
char *getStringValue(FILE *fp, char *entry);
bool getBoolValue(FILE *fp, char *entry, bool defaultVal);
void getMultiStringValue(FILE *fp, char *entry, char **ret, int len);
static int conf_initUrlStructs(conf_t *confp);
static int conf_initReqBuffers(conf_t *confp);
/*********end****************/
conf_t *conf_new(const char *path) {
    conf_t *confp = calloc(1, sizeof(conf_t));
    if (confp == NULL) {
        return NULL;
    }
    int err = conf_init(confp, path);
    if (err < 0) {
        return NULL;
    } else {
        return confp;
    }
}

int conf_destroy(conf_t *confp) {
    conf_reset(confp);
    free(confp);
    return 0;
}

int conf_init(conf_t *confp, const char *path) {
    int err = 0;
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "can't open %s\n", path);
        return -1;
    }
    confp->proxyHost     = getStringValue(fp, "proxyHost");
    confp->proxyPort     = getIntegerValue(fp, "proxyPort", -1);
    confp->type          = getIntegerValue(fp, "type", BENCH_SEQ);
    confp->method        = getIntegerValue(fp, "method", METHOD_GET);
    confp->clients       = getIntegerValue(fp, "clients", 300);
    confp->benchtime     = getIntegerValue(fp, "benchtime", 10); 
    confp->force         = getBoolValue(fp, "force", false);
    confp->forceReload   = getBoolValue(fp, "forceReload", false);
    confp->userAgent     = getStringValue(fp, "userAgent");

    getMultiStringValue(fp, "urls", confp->urlStrings, MAX_URLS);
    /* get the number of urls */
    confp->numUrls = 0;
    for(char **pp=confp->urlStrings; *pp != NULL; pp++) {
        confp->numUrls += 1;
    }
    /* init url struct */
    err = conf_initUrlStructs(confp);
    if (err < 0) {
        conf_reset(confp);
        return -1;
    }
    /* init http request buffers */
    err = conf_initReqBuffers(confp);
    if (err < 0) {
        conf_reset(confp);
        return -1;
    }
    return 0;
}

void conf_reset(conf_t *confp) {
    Free(confp->proxyHost);
    Free(confp->userAgent);
    if (confp->urls) {
        for (int i = 0; i < confp->numUrls; i++) {
            url_reset(confp->urls+i);
        }
        Free(confp->urls);
    }
    for (int i = 0; i < confp->numUrls; i++) {
        Free(confp->urlStrings[i]);
        Free(confp->reqBuffers[i]);
    }
}

int conf_initUrlStructs(conf_t *confp) {
    confp->urls = malloc(sizeof(url_t) * confp->numUrls);
    for (int i = 0; i < confp->numUrls; i++) {
        url_t *url = confp->urls + i;
        char *urlString = confp->urlStrings[i];
        int err = url_init_with_str(url, urlString);
        if (err < 0) {
            fprintf(stderr, "can't init url struct with string %s", urlString);
            return -1;
        }
    }
    return 0;
}

int conf_initReqBuffers(conf_t *confp) {
    assert(confp && confp->urls);
    char tmpBuf[MAXSIZE];
    char *methodStr = NULL;
    switch (confp->method) {
        case METHOD_GET:
            methodStr = "GET";
            break;
        case METHOD_HEAD:
            methodStr = "HEAD";
            break;
        case METHOD_OPTIONS:
            methodStr = "OPTIONS";
            break;
        case METHOD_TRACE:
            methodStr = "TRACE";
            break;
        default:
            fprintf(stderr, "unkown http method type..");
    }
    for (int i = 0; i < confp->numUrls; i++) {
        url_t *url = confp->urls + i;
        char **reqBuf = confp->reqBuffers + i;
        char pathAndQueryStr[MAXSIZE];
        if (url->query_str) {
            snprintf(pathAndQueryStr, MAXSIZE, "%s?%s", url->path, url->query_str);
        } else {
            snprintf(pathAndQueryStr, MAXSIZE, "%s", url->path);
        }
        if (confp->forceReload) {
            snprintf(tmpBuf, MAXSIZE, "%s %s HTTP/1.1\r\nHOST:%s\r\nUser-Agent: %s\r\nCache-Control: no-cache\r\n\r\n",
                     methodStr, pathAndQueryStr, url->host, confp->userAgent);
        } else {
            snprintf(tmpBuf, MAXSIZE, "%s %s HTTP/1.1\r\nHOST:%s\r\nUser-Agent: %s\r\n\r\n",
                     methodStr, pathAndQueryStr, url->host, confp->userAgent);
        }
        *reqBuf = strdup(tmpBuf);
        if (*reqBuf == NULL) {
            fprintf(stderr, "strdup fails for  %s's http request buffer\n", url->full_url);
            return -1;
        }
    }
    return 0;
}

int getIntegerValue(FILE *fp, char *entry, int defaultVal) {
    char *val = getKeyValue(fp, entry);
    if (val) {
        return atoi(val);
    } else {
        return defaultVal;
    }
}

char *getStringValue(FILE *fp, char *entry) {
    char *val = getKeyValue(fp, entry);
    if (val) {
        return strdup(val);
    } else {
        return NULL;
    }
}

bool getBoolValue(FILE *fp, char *entry, bool defaultVal) {
    char *val = getKeyValue(fp, entry);
    if (val) {
        if (!strcasecmp(val, "on")) {
            return true;
        } else if (!strcasecmp(val, "true")) {
            return true;
        } else if (!strcasecmp(val, "off")) {
            return false;
        } else if (!strcasecmp(val, "false")) {
            return false;
        } else {
            fprintf(stderr, "only 'on' 'true' 'off' and 'false' are valid for the key %s\n", entry);
            return false;
        }
    } else {
        fprintf(stderr, "not found %s\n", entry);
        return defaultVal;
    }
}

/* the code of this function is dirty*/
void getMultiStringValue (FILE *fp, char *entry, char **ret, int len)
{
    assert(fp && entry && ret);
    fseek(fp, 0, SEEK_SET);
    char line[MAXLINE];
    bool in_val = false;

    while (fgets(line, MAXLINE, fp) != NULL)
    {
        if (isCommentLine(line) || isBlankLine(line))
        {
            continue;
        }
        char *end = line + strlen(line);
        if (*(end - 1) == '\n')
        {
            *(end - 1) = '\0';
        }
        char *start = strip(line, " ");
        if (in_val)
        {
            if (strchr (start, '}'))
            {
                in_val = false;
                return;
            }
            else
            {
                char *val_start = strtok(start, " ");
                for (; ; )
                {
                    /* the end element of the array should be NULL*/
                    if (len > 1)
                    {
                        *ret = strdup(val_start);
                        ret++;
                        len--;
                        val_start = strtok(NULL, " ");
                        if (!val_start)
                        {
                            break;
                        }
                    }
                    else
                    {
                        fprintf(stderr, "entry %s has too many values\n", entry);
                        return;
                    }
                }
            }
        }
        char *middle = strchr(start, SEP);
        if (!middle)
        {
            continue;
        }
        *middle = '\0';
        middle++;
        char *name = strip(start, " ");
        if (strcasecmp(name, entry))
        {
            continue;
        }
        else
        {
            char *value = strip(middle, " ");
            if (!strchr (value, '{'))
            {
                fprintf(stderr, "error: not found '{' for entry: %s\n", entry);
                return;
            }
            else
            {
                in_val = true;
            }
        }
    }
    return;
}

static char *getKeyValue(FILE *fp, char *key) {
    assert(fp && key);
    fseek(fp, 0, SEEK_SET);
    static char line[MAXLINE];
    while (fgets(line, MAXLINE, fp) != NULL) {
        if (isCommentLine(line) || isBlankLine(line)) {
            continue;
        }
        char *start = skipHeadChar(line, ' ');
        char *end = line + strlen(line);
        if (*(end - 1) == '\n') {
            *(end - 1) = '\0';
        }
        char *middle = strchr(start, SEP);
        if (!middle) {
            continue;
        }
        *middle = '\0';
        middle++;
        char *name = strip(start, " ");
        if (strcasecmp(name, key)) {
            continue;
        } else {
            char *value = strip(middle, " ");
            return value;
        }
    }
    return NULL;
}

static char *skipHeadChar(char *line, char c) {
    while(*line == c) {
        line++;
    }
    return line;
}

static bool isCommentLine(char *line) {
    while(*line == ' ') {
        line++;
    }
    return ((*line == '#')? true : false);
}

static bool isBlankLine(char *line) {
    while(*line == ' ') {
        line++;
    }
    return (*line == '\n')? true : false;
}

/* debug  */
#if 0
int main(int argc, char *argv[])
{
    if (conf_init("../conf/settings.conf") < 0)
    {
        fprintf(stderr, "conf init error\n");
        return -1;
    }

    return 0;
}
#endif
