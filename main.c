/* -*- Mode: C; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
 * =======================================================================
 *       Filename:  main.c
 *        Created:  2015-02-11 15:15:48
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang
 *			Email:  yy2012cn@NOSPAM.gmail.com
 * =======================================================================
 */
#include <unistd.h>
#include <sys/param.h>
#include <rpc/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <getopt.h>
#include <signal.h>
#include <time.h>

#include "util.h"
#include "conf.h"
#include "url.h"

#define RANDOM(x) (rand() % x)
#define MAXSIZE  4096

typedef struct statis_s {
    int bytes;
    int failed;
    int succeed;
} statis_t;

static statis_t statis;
static conf_t settings;
static int mypipe[2];

volatile int timerexpired=0;
static void alarmHandler(int signal) {
   timerexpired=1;
}

/*
 *  send a http request, return -1 if get a error, otherwith return the number
 *  of bytes read from Socket(Notice: it is 0 if the force setting isd true.)
 */
static int sendHttpRequest(const char *host,const int port,const char *reqBuf, int len) {
    int sock = Socket(host, port);
    if (sock < 0) {
        return -1;
    }
    if (len != write(sock, reqBuf, len)) {
        close(sock);
        return -1;
    }
    int i = 0;
    int totalBytes = 0;
    char buf[MAXSIZE];
    if(settings.force) {
        if(close(sock)) {
            return -1;
        } else {
            return 0;
        }
    } else {
        /* read all available data from socket */
        while(1) {
            if(timerexpired) break;
            i = read(sock,buf,MAXSIZE);
            /* fprintf(stderr,"%d\n",i); */
            if(i<0) {
                close(sock);
                return -1;
            } else if(i==0) {
                close(sock);
                return totalBytes;
            } else {
                totalBytes += i;
            }
        }
    }
    return 0;
}

static int benchSeq() {
    while(1) {
        if(timerexpired) {
            if(statis.failed>0) {
                /* fprintf(stderr,"Correcting failed by signal\n"); */
                statis.failed--;
            }
            return 0;
        }
        bool isFailed = false;
        for (int i = 0; i < settings.numUrls; i++) {
            url_t *url = settings.urls + i;
            char *reqBuf = settings.reqBuffers[i];
            int numBytes = sendHttpRequest(url->host, url->port, reqBuf, strlen(reqBuf));
            if (numBytes < 0) {
                isFailed = true;
                break;
            } else {
                statis.bytes += numBytes;
            }
        }
        if (isFailed) {
            statis.failed++;
        } else {
            statis.succeed++;
        }
    }
    return 0;
}

static int benchRandom() {
    while(1) {
        if(timerexpired) {
            if(statis.failed>0) {
                /* fprintf(stderr,"Correcting failed by signal\n"); */
                statis.failed--;
            }
            return 0;
        }
        int i = RANDOM(settings.numUrls);
        url_t *url = settings.urls + i;
        char *reqBuf = settings.reqBuffers[i];
        int reqBufLen = strlen(reqBuf);
        int numBytes = sendHttpRequest(url->host, url->port, reqBuf, reqBufLen);
        if (numBytes < 0) {
            statis.failed++;
            continue;
        } else {
            statis.bytes += numBytes;
        }
        statis.succeed++;
    }
    return 0;
}

static  int benchFirst() {
    url_t *firstUrl = settings.urls;
    char *reqBuf = settings.reqBuffers[0];
    int reqBufLen = strlen(reqBuf);

    while(1) {
        if(timerexpired) {
            if(statis.failed>0) {
                /* fprintf(stderr,"Correcting failed by signal\n"); */
                statis.failed--;
            }
            return 0;
        }
        int numBytes = sendHttpRequest(firstUrl->host, firstUrl->port, reqBuf, reqBufLen);
        if (numBytes < 0) {
            statis.failed++;
            break;
        } else {
            statis.bytes += numBytes;
        }
        statis.succeed++;
    }
    return 0;
}

static int bench() {
    struct sigaction sa;

    /* setup alarm signal handler */
    sa.sa_handler=alarmHandler;
    sa.sa_flags=0;
    if(sigaction(SIGALRM, &sa, NULL)) {
        exit(3);
    }
    alarm(settings.benchtime);

    switch (settings.type) {
        case BENCH_SEQ:
            benchSeq();
            break;
        case BENCH_RANDOM:
            benchRandom();
            break;
        case BENCH_FIRST:
            benchFirst();
            break;
        default:
            fprintf(stderr, "unkown bech type..");
            return -1;
    }
    return 0;
}

static int createChildren() {
    pid_t pid;
    if(pipe(mypipe)) {
        perror("pipe failed.");
        return 3;
    }

    for (int i=0; i < settings.clients; i++) {
        pid = fork();
        if (pid < (pid_t)0) {  /* error fork, exit the parent process. */
            fprintf(stderr,"problems forking worker no. %d\n",i);
            perror("fork failed.");
            return 3;
        } else if (pid == (pid_t)0) {  /* childen process,break out of the loop */
            sleep(1);
            break;
        } else {
            continue;
        }
    }

    if (pid == (pid_t)0) {  /* children process */
        bench();

        FILE *f=fdopen(mypipe[1],"w");
        if(f==NULL) {
            perror("open pipe for writing failed.");
            return 3;
        }
        /* fprintf(stderr,"Child - %d %d\n",speed,failed); */
        fprintf(f,"%d %d %d\n", statis.succeed, statis.failed, statis.bytes);
        fclose(f);
        return 0;

    } else {  /* parent process */
        FILE *f=fdopen(mypipe[0],"r");
        if(f==NULL) {
            perror("open pipe for reading failed.");
            return 3;
        }
        setvbuf(f,NULL,_IONBF,0);
        int succeed=0;
        int failed=0;
        int bytes=0;
        /*
         * read statistics data from all the child process. and store the sum
         * to the global statis struct
         */
        for(int clients = settings.clients; clients > 0; clients--) {
            pid=fscanf(f,"%d %d %d",&succeed, &failed, &bytes);
            if(pid<2) {
                fprintf(stderr,"Some of our childrens died.\n");
                break;
            }
            statis.succeed += succeed;
            statis.failed += failed;
            statis.bytes += bytes;
            /* fprintf(stderr,"*Knock* %d %d read=%d\n",speed,failed,pid); */
        }
        fclose(f);

        printf("\n");
        printf("Speed=%d requests/min \n", (int)((statis.succeed+statis.failed)/(settings.benchtime/60.0)));
        printf("%d bytes/sec  \n", statis.bytes/settings.benchtime);
        printf("succeed requests: %d \n", statis.succeed);
        printf("failed requests : %d \n", statis.failed); 
    }
    return 0;
}

int main(int argc, char *argv[]) {
    int opt;
    char *confPath = NULL;
    while ((opt = getopt(argc, argv, "c:h?")) != -1) {
        switch(opt) {
            case 'c':
                confPath = optarg;
                break;
            case 'h':
            case '?':
                printf ("-c: configure file path\n-h -?: show help\n");
                break;
            default:
                fprintf(stderr, "unknown option");
                break;
        }
    }
    if (confPath == NULL) {
        confPath = "./bench.conf";
    }
    int err = conf_init(&settings, confPath);
    if (err < 0) {
        fprintf(stderr, "can't parse the configure file \n");
        return -1;
    }
    createChildren();

    /* release the resources. */
    conf_reset(&settings);
    return 0;
}
