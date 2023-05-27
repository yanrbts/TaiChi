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

#include <tch_config.h>
#include <tch_core.h>

int use_syslog = 0;
int use_tty = 1;

void 
tch_error(const char *s)
{
    char *msg = strerror(errno);
    TCHLOGE("%s: %s", s, msg); 
}