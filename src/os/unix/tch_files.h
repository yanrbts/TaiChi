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

#ifndef _TCH_FILES_INCLUDE_
#define _TCH_FILES_INCLUDE_

#include <tch_config.h>
#include <tch_core.h>


typedef int                      tch_fd_t;

#define tch_stdout               STDOUT_FILENO
#define tch_stderr               STDERR_FILENO
#define TCH_LINEFEED             "\x0a"

/*
 * we use inlined function instead of simple #define
 * because glibc 2.3 sets warn_unused_result attribute for write()
 * and in this case gcc 4.3 ignores (void) cast
 */
static tch_inline ssize_t
tch_write_fd(tch_fd_t fd, void *buf, size_t n)
{
    return write(fd, buf, n);
}

#define tch_write_console        tch_write_fd

#define tch_linefeed(p)          *p++ = LF;
#define TCH_LINEFEED_SIZE        1


#endif