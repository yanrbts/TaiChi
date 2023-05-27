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
#ifndef __TCH_FMQ_CLIENT__
#define __TCH_FMQ_CLIENT__

#include <tch_fmqmsg.h>

typedef struct tch_fmq_client_s     tch_fmq_client_t;

void fmq_client(zsock_t *cmdpipe, void *msgpipe);
tch_fmq_client_t *fmq_client_new(void);
void fmq_client_destroy(tch_fmq_client_t **self_p);
uint8_t fmq_client_destructor(tch_fmq_client_t *self);
uint8_t fmq_client_connect(tch_fmq_client_t *self, const char *endpoint, uint32_t timeout);
uint8_t fmq_client_subscribe(tch_fmq_client_t *self, const char *path);
uint8_t fmq_client_set_inbox(tch_fmq_client_t *self, const char *path);
uint8_t fmq_client_status(tch_fmq_client_t *self);
const char *fmq_client_reason(tch_fmq_client_t *self);
bool fmq_client_connected(tch_fmq_client_t *self);
zactor_t *fmq_client_actor(tch_fmq_client_t *self);
zsock_t *fmq_client_msgpipe(tch_fmq_client_t *self);

#endif