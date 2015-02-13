#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "util.h"
#include "str.h"
#include "url.h"

#define URL_LENGTH 4096

url_t *url_new_with_str(char *url_str)
{
    url_t *u = Malloc(sizeof(*u));
    url_init_with_str(u, url_str);
    return u;
}

url_t *url_new_with_host(char *protocol, char *host, char *file)
{
    url_t *u = Malloc(sizeof(*u));
    url_init_with_host(u, protocol, host, file);
    return u;
}

int url_init_with_str(url_t *u, char *url_str)
{
    assert(url_str);
    u->full_url = strdup(url_str);
    
    size_t url_len = strlen(url_str);
    char buf[URL_LENGTH];
    if (url_len >= URL_LENGTH)
    {
        return -1;
    }
    strncpy(buf, url_str, URL_LENGTH);
    /* get fragment id*/
    char *frag_id = strchr(buf, '#');
    if (!frag_id)
    {
        u->frag_id = NULL;
    }
    else
    {
        *frag_id = '\0';
        frag_id++;
        u->frag_id = strdup(frag_id);
    }
    /* get query string*/
    char *query_str = strchr(buf, '?');
    if (!query_str)
    {
        u->query_str = NULL;
    }
    else
    {
        *query_str = '\0';
        query_str++;
        u->query_str = strdup(query_str);
    }
    /* get protocol*/
    char *p_start = buf;
    char *p_end = strstr(buf, "://");
    char *h_start;
    if (!p_end)
    {
        u->protocol = strdup("http");
        h_start = buf;
    }
    else
    {
        *p_end = '\0';
        u->protocol = strdup(p_start);
        h_start = p_end + 3;
    }
    /*get path*/
    char *path = strchr(h_start, '/');
    if (!path)
    {
        u->path = strdup("/");
    }
    else
    {
        u->path = strdup(path);
        *path = '\0';
    }
    /*get port*/
    char *port = strchr(h_start, ':');
    if (!port)
    {
        if (!strcasecmp(u->protocol, "http"))
        {
            u->port = 80;
        }
        else if (!strcasecmp(u->protocol, "https"))
        {
            u->port = 443;
        }
        else
        {
            u->port = 0;
        }
    }
    else
    {
        *port = '\0';
        port++; /* ignore ':' */
        u->port = atoi(port);
    }
    /* get host*/
    u->host = strdup(h_start);
    return 0;
}


int url_init_with_host(url_t *u, char *protocol, char *host, char *file)
{
    if (!startswith(file, "/"))
    {
        fprintf (stderr, "error:host:%s  file:%s\n", host, file);
        return -1;
    }
    char buf[URL_LENGTH];
    strncpy(buf, file, URL_LENGTH);
    /* get fragment id*/
    char *frag_id = strchr(buf, '#');
    if (!frag_id)
    {
        u->frag_id = NULL;
    }
    else
    {
        *frag_id = '\0';
        frag_id++;
        u->frag_id = strdup(frag_id);
    }
    /* get query string*/
    char *query_str = strchr(buf, '?');
    if (!query_str)
    {
        u->query_str = NULL;
    }
    else
    {
        *query_str = '\0';
        query_str++;
        u->query_str = strdup(query_str);
    }
    /* get path */
    u->path = strdup(buf);
    /*get protocol*/
    if (protocol)
    {
        u->protocol = strdup(protocol);
    }
    else
    {
        /* default protocol is http*/
        u->protocol = strdup("http");
    }
    u->host = strdup(host);

    /* get full url */
    u->port = 80;
    int p_len = strlen(u->protocol);
    int host_len = strlen(u->host);
    int file_len = strlen(file);
    /* must contain :// */
    u->full_url = Malloc(p_len + host_len + file_len + 4);
    memcpy(u->full_url, u->protocol, p_len);
    strcpy(u->full_url + strlen(u->full_url), "://");
    memcpy(u->full_url + strlen(u->full_url), host, host_len);
    memcpy(u->full_url + strlen(u->full_url), file, file_len);
    return 0;
}

void url_destroy (url_t *u)
{
    assert(u);
    
    url_reset(u);
    free(u);
}
void url_reset(url_t *u)
{
    assert(u);
    
    Free(u->full_url);
    Free(u->protocol);
    Free(u->host);
    Free(u->path);
    Free(u->query_str);
    Free(u->frag_id);
}

/*** for debug ***************/
#if 0

char *urls[] = {
    "www.163.com:1123/hello?123#print",
    "www.baidu.com/search",
    "https://www.google.com.hk/search?newwindow=1&safe=strict&client=aff-360daohang&hl=zh-CN&gbv=2&q=url+length&oq=url+length&gs_l=serp.3..0l2j0i7i30l2j0i30l2j0i7i30j0i30j0i7i30j0i30.819698.823173.0.824438.15.11.0.0.0.0.342.1509.2-2j3.5.0...0.0...1c.1.11.serp.OgVs9TDwJNc",
    "http://stackoverflow.com/questions/417142/what-is-the-maximum-length-of-a-url-in-different-browsers",
    "http://baike.baidu.com/view/1758.htm#1",
    "http://www.baidu.com/s?wd=%E5%88%98%E5%BE%B7%E5%8D%8E&rsv_bp=0&ch=&tn=baidu&bar=&rsv_spt=3&ie=utf-8&inputT=1",
    NULL
};
void print_url(url_t *u)
{
    printf ("full_url:%s\n", u->full_url);
    printf ("protocol:%s\n", u->protocol);
    printf ("host    :%s\n", u->host);
    
    printf ("port    :%d\n", u->port);

    printf ("path:   :%s\n", u->path);
    printf ("query_st:%s\n", u->query_str);
    printf ("fragment:%s\n\n", u->frag_id);
}
int main(int argc, char *argv[])
{
    char **u = urls;
    while(*u)
    {
        url_t *uu = url_new_with_str(*u);
        print_url(uu);
        url_destroy(uu);
        u++;
    }
    url_t *ul = url_new_with_host(NULL, "www.baidu.com", "/hello");
    print_url(ul);
    url_destroy(ul);
    return 0;
}


#endif
