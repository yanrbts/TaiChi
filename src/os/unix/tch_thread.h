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

#ifndef _TCH_THREAD_H_INCLUDED_
#define _TCH_THREAD_H_INCLUDED_


#include <tch_config.h>
#include <tch_core.h>

#if (TCH_THREADS)
#else

#define tch_log_tid           0
#define TCH_TID_T_FMT         "%d"

#endif

#endif