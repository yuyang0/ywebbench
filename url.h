#ifndef _URL_H_
#define _URL_H_
#include <stdint.h>
typedef struct 
{
    char *full_url;
    
    char *protocol;
    char *host;
    uint16_t port;
    
    char *path;
    char *query_str; /*query string*/
    char *frag_id;   /*frament id*/
    uint8_t  depth;

}url_t;
url_t *url_new_with_str(char *str);
url_t *url_new_with_host(char *protocol, char *host, char *file);
int url_init_with_str(url_t *url, char *url_str);
int url_init_with_host(url_t *url, char *protocol, char *host, char *file);
void url_destroy (url_t *url);
void url_reset(url_t *url);

/* char *get_protocol(char *url_str); */
/* char *get_host(char *url_str); */
/* unsigned int get_port(char *url_str); */
/* char *get_file(char *url_str); */
#endif
