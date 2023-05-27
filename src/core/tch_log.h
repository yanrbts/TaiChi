/*
 * Project:
 *  ___________      .___________ .__    .__ 
 * \__    ___/____  |__\_   ___ \|  |__ |__|
 *   |    |  \__  \ |  /    \  \/|  |  \|  |
 *   |    |   / __ \|  \     \___|   Y  \  |
 *   |____|  (____  /__|\______  /___|  /__|
 *                \/           \/     \/   
 *
 * Copyright (C) 2021 - 2022, Yan RuiBing, <772166784@qq.com>, et al.
 *
 */

#ifndef _TCH_LOG_INCLUDE_
#define _TCH_LOG_INCLUDE_

#include <tch_config.h>
#include <tch_core.h>

extern int use_syslog;
extern int use_tty;

#define USE_TTY() do {                                                          \
        use_tty = isatty(STDERR_FILENO);                                        \
    } while(0)

#define TIME_FORMAT "%F %T"
#define USE_SYSLOG(ident) do {                                                  \
        use_syslog = 1;                                                         \
        openlog((ident), LOG_CONS | LOG_PID, 0);                                \
    } while(0)

#define TCHLOGI(format, ...) do {                                                \
        if (use_syslog) {                                                        \
            syslog(LOG_INFO, format, ## __VA_ARGS__);                            \
        } else {                                                                 \
            time_t now = time(NULL);                                             \
            char timestr[20];                                                    \
            strftime(timestr, 20, TIME_FORMAT, localtime(&now));                 \
            if (use_tty) {                                                       \
                fprintf(stderr, "\e[01;32m %s INFO: \e[0m" format "\n", timestr, \
                        ## __VA_ARGS__);                                         \
            } else {                                                             \
                fprintf(stderr, " %s INFO: " format "\n", timestr,               \
                        ## __VA_ARGS__);                                         \
            }                                                                    \
        }                                                                        \
    } while(0)

#define TCHLOGE(format, ...) do {                                                 \
        if (use_syslog) {                                                         \
            syslog(LOG_ERR, format, ## __VA_ARGS__);                              \
        } else {                                                                  \
            time_t now = time(NULL);                                              \
            char timestr[20];                                                     \
            strftime(timestr, 20, TIME_FORMAT, localtime(&now));                  \
            if (use_tty) {                                                        \
                fprintf(stderr, "\e[01;35m %s ERROR: \e[0m" format "\n", timestr, \
                        ## __VA_ARGS__);                                          \
            } else {                                                              \
                fprintf(stderr, " %s ERROR: " format "\n", timestr,               \
                        ## __VA_ARGS__);                                          \
            }                                                                     \
        }                                                                         \
    } while (0)

static tch_inline void
tch_write_stderr(char *text)
{
    (void) tch_write_fd(tch_stderr, text, tch_strlen(text));
}


static tch_inline void
tch_write_stdout(char *text)
{
    (void) tch_write_fd(tch_stdout, text, tch_strlen(text));
}

void tch_error(const char *s);

#endif