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
#include <tch_config.h>
#include <tch_core.h>

void 
tch_insert_node(char *name, const char *ip, zsock_t *sock)
{
    tch_lannode_t *node = NULL;
    node = tch_search_node(name);

    // find node data
    if (node != NULL) {
        // set connected flag
        node->flag = 0;
        return;
    }

    for (int i = 0; i < TCH_MACNAME; i++) {
        node = server.nodes + i;
        if (node->uname[0] == '\0') {
            strncpy(node->uname, name, sizeof(node->uname)/sizeof(char));
            strncpy(node->ip, ip, sizeof(node->ip)/sizeof(char));
            node->sock = sock;
            node->flag = 0;
            node->index = i;
            break;
        }
    }
}

void 
tch_del_node(char *name)
{
    tch_lannode_t *node = NULL;
    node = tch_search_node(name);

    // find node data
    // set offline flag
    if (node != NULL)
        node->flag = 1;
}

void 
tch_setfmq_node(char *name)
{
    tch_lannode_t *node = NULL;
    node = tch_search_node(name);
    // find node data
    // set fmq flag
    if (node != NULL)
        node->isfmq = 1;
}

/* Returns node pointer if found, otherwise returns -1 */
tch_lannode_t *
tch_search_node(char *name)
{
    tch_lannode_t *node = NULL;
    for (int i = 0; i < TCH_MACNAME; i++) {
        node = server.nodes + i;
        if (tch_strcmp(node->uname, name) != 0)
            continue;
        // data found
        return node ;
    }
    return NULL;
}

int 
tch_list_node(int argc, char **argv)
{
    tch_lannode_t *node = NULL;
    for (int i = 0; i < TCH_MACNAME; i++) {
        node = server.nodes + i;
        if (node->uname[0] != '\0' && node->flag == 0) {
            if (node->isfmq == 0)
                printf("[%d] %s [%s]\n", node->index, node->uname, node->ip);
            else
                printf("[%s%d%s] %s [%s]\n", TCH_COLOR_RED, node->index, TCH_COLOR_END, node->uname, node->ip);
        }
    }
    return 0;
}

int
tch_select_node(int argc, char **argv)
{
    /*
     * select command have two parameters
     * example : select 0
     */
    if (argc != 2)
        return 0;

    int n = atoi(argv[1]);
    tch_lannode_t *node = NULL;
    for (int i = 0; i < TCH_MACNAME; i++) {
        node = server.nodes + i;
        if (node->index == n) {
            node->isselect = 1;
        }
    }
    return 0;
}

tch_lannode_t *
tch_getselect_node()
{
    tch_lannode_t *node = NULL;
    for (int i = 0; i < TCH_MACNAME; i++) {
        node = server.nodes + i;
        if (node->isselect)
            break;
        node = NULL;
    }
    return node;
}