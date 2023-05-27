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

#ifndef _TCH_CMD_INCLUDE_
#define _TCH_CMD_INCLUDE_

#include <tch_config.h>
#include <tch_core.h>

struct tch_command_s{
    const char          *name;
    const char          *description;
    int                (*func)(int argc, char **argv);
};

extern tch_command_t tch_cmds[];
#endif