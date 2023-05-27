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
#ifndef __TCH_NODE_H__
#define __TCH_NODE_H__

#include <tch_config.h>
#include <tch_core.h>

struct tch_lannode_s {
    int     index;                 /* computer index */
    char    uname[65];             /* computer name */
    int     flag;                  /* Identifies whether the node is offline, 
                                      1 if offline otherwise 0 */
    int     isselect;              /* Whether to be selected as the sender 1 if otherwise 0 */
    int     isfmq;                 /* Whether to set the fmq service 1, otherwise it is 0 */
    char    ip[64];                /* ip address */
    zsock_t *sock;
};

void tch_insert_node(char *name, const char *ip, zsock_t *sock);
void tch_del_node(char *name);
void tch_setfmq_node(char *name);
tch_lannode_t *tch_search_node(char *name);
int tch_list_node(int argc, char **argv);
int tch_select_node(int argc, char **argv);
tch_lannode_t *tch_getselect_node();

#endif