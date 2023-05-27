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
#ifndef __TCH_FILE_H__
#define __TCH_FILE_H__

#include <tch_config.h>
#include <tch_core.h>

/* fmq client connection node */
struct tch_fmq_cs {
    tch_fmq_client_t    *client;        // fmq client
    tch_lannode_t       *node;          // fmq client owning node
    zsock_t             *msgpipe;
    uint32_t             timeout;       // set time out
    char                 clpath[256];   // client fmq recv path
    char                *tcp;           // tcp address
};

struct tch_fmq_s {
    char                 svpath[256];   // server fmq path
    int                  sfg;           // Whether the fmq service is established, 1 is established, otherwise 0
};

int tch_fmq_ishavenode(const char *name);
int tch_fmq_insertnode(tch_fmq_cs_t *node);
int tch_fmq_deletenode(const char *name);
int tch_fmq_freenode(tch_fmq_cs_t *node);
void tch_fmq_destroy();
extern tch_module_t tch_file_module;
#endif

