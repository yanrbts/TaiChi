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

#ifndef _TCH_MODULE_H_INCLUDED_
#define _TCH_MODULE_H_INCLUDED_

#include <tch_config.h>
#include <tch_core.h>

struct  tch_module_s {
    const char                 *name;
    const char                 *cmdshell;
    const char                 *description;
    int                       (*execmd)(int argc, char **argv);   /* The current command executes the function, 
                                                                   the function pointer can be null */
    tch_command_t              *commands;
};

extern tch_module_t  *tch_modules[];

#endif