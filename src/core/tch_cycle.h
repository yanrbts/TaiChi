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

#ifndef _TCH_CYCLE_INCLUDE_
#define _TCH_CYCLE_INCLUDE_

#include <tch_config.h>
#include <tch_core.h>

struct tch_cycle_s {

    char                     *cmdline;
    
    tch_module_t             *current_module;
    tch_module_t            **modules;
    tch_uint_t                modules_n;
};

#endif