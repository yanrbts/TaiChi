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
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 */
#ifndef __TCH_FMQ_SERVER__
#define __TCH_FMQ_SERVER__

#include <tch_fmqmsg.h>

void fmq_server (zsock_t *pipe, void *args);

#endif