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
#ifndef __TCH_TAICHI_H__
#define __TCH_TAICHI_H__

#include <tch_config.h>
#include <tch_core.h>

struct tch_data_s {
    zactor_t        *actor;        /* message actor is implemented as a thread plus a PAIR-PAIR pipe. */
    zactor_t        *file_actor;   /* actor of files sended */
    tch_lannode_t   *nodes;        /* computer nodes max 254 */
    tch_lannode_t    local_node;   /* local machine */
    tch_fmq_t        fmq;          /* fmq client */
    zhash_t         *fmqnodes;     /* fmq client node hash table*/
};

int tch_quit(int argc, char **argv);

extern tch_data_t server;

#endif