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

#ifndef _TCH_PROCESS_H_INCLUDE_
#define _TCH_PROCESS_H_INCLUDE_

#include <tch_config.h>
#include <tch_core.h>

#define tch_getpid   getpid
#define tch_getppid  getppid

#ifndef tch_log_pid
#define tch_log_pid  tch_getpid
#endif

#endif