/*Time-stamp: <2013-01-15 11:28:12 by Yu Yang>
 * =======================================================================
 *       Filename:  settings.h
 *        Created:  2015-02-02 21:25:14
 *       Compiler:  gcc
 *
 *         Author:  Yu Yang
 *			Email:  yy1990cn@gmail.com
 * =======================================================================
 */

#ifndef _SETTINGS_H_
#define _SETTINGS_H_ 1

#include <stdbool.h>
#define BENCH_SEQ    0
#define BENCH_RANDOM 2
#define BENCH_FIRST  4

typedef struct settings_s {
    int type;
    int method;
    int clients;
    int benchtime;
    bool force;
    bool forceReload;
    char **urls;
}settings_t;

#endif /* _SETTINGS_H_ */

