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
#include <tch_config.h>
#include <tch_core.h>

static int tch_file_ls(int argc, char **argv);
static int tch_file_send(int argc, char **argv);
static int tch_file_recv(int argc, char **argv);
static int tch_cattcp(char **tcp, const char *ip);

tch_command_t filecmds[] = {
    {"ls",        "List files in the current directory.",     tch_file_ls},
    {"send",      "Send file (Select the file first)",        tch_file_send},
    {"recv",      "Client accepts file",                      tch_file_recv},
    {NULL,        NULL,                                       NULL}
};

tch_module_t tch_file_module = {
    "file",
    "taichi/file>",
    "file upload command..",
    NULL,
    filecmds
};

static int
tch_file_ls(int argc, char **argv)
{
    TCH_UNUSED(argc);
    TCH_UNUSED(argv);

    system("ls");

    return 0;
}

static int
tch_file_send(int argc, char **argv)
{
    // check Whether the service established
    if (server.fmq.sfg == 1) {
        TCHLOGI("fmq service is established");
        return -1;
    }

    if (argc < 2) {
        if (tch_strcmp(server.fmq.svpath, "") == 0 || server.fmq.svpath[0] == '\0')
            strncpy(server.fmq.svpath, TCH_FMQ_SVPATH, tch_strlen(TCH_FMQ_SVPATH)+1);
    } else {
        strncpy(server.fmq.svpath, argv[1], tch_strlen(argv[1])+1);
    }

    // make Directory
    tch_mkdir(server.fmq.svpath);

    zstr_sendx(server.file_actor, "PUBLISH", server.fmq.svpath, "/", NULL);
    zstr_sendx(server.file_actor, "BIND", TCH_FMQ_TCP, NULL);

    /*Notify other nodes that the file transfer service node has been established*/
    zstr_sendx(server.actor, "SHOUT", TCH_FMQ_SERVER, NULL);
    server.fmq.sfg = 1;
    return 0;
}

static int
tch_file_recv(int argc, char **argv)
{
    tch_fmq_cs_t    *new_node;

    tch_lannode_t *node = tch_getselect_node();
    if (node == NULL) {
        TCHLOGE("please select node host by command 'select'.");
        return TCH_ERROR;
    }

    /*
     * First, determine whether this node has been selected as the file receiver. 
     * If it has been selected before, this operation will be abandoned.
     */
    if (tch_fmq_ishavenode(node->uname) == TCH_OK) {
        TCHLOGI("Node has been selected recv file.");
        return TCH_ERROR;
    } else if (tch_strcmp(node->uname, "") != 0) {
        new_node = tch_malloc(sizeof(tch_fmq_cs_t));
        tch_memzero(new_node, sizeof(tch_fmq_cs_t));
        tch_memzero(new_node->clpath, sizeof(new_node->clpath));
        new_node->node = node;

        if (argc < 2) {
            if (tch_strcmp(new_node->clpath, "") == 0)
                strncpy(new_node->clpath, TCH_FMQ_CLPATH, tch_strlen(TCH_FMQ_CLPATH)+1);
        } else {
            strncpy(new_node->clpath, argv[1], tch_strlen(argv[1])+1);
        }

        // make Directory
        if (tch_mkdir(new_node->clpath) == TCH_ERROR) {
            goto tcherror;
        }

        // link tcp address : tcp://ip:5670
        if (tch_cattcp(&new_node->tcp, node->ip) == -1) {
            TCHLOGE("strcat ip error %s\n", node->ip);
            goto tcherror;
        }

        new_node->timeout = 1000;
        new_node->client = fmq_client_new();
        assert(new_node->client);
    } else {
        TCHLOGE("create node failed.");
        return TCH_ERROR;
    }

    if (fmq_client_connect(new_node->client, new_node->tcp, new_node->timeout) != 0) {
        if (new_node->client)
            fmq_client_destroy(&new_node->client);
        TCHLOGE("fmq client connect error");
        goto tcherror;
    }
    //  Set the clients storage location
    if (fmq_client_set_inbox(new_node->client, new_node->clpath) != 0) {
        TCHLOGE("fmq client set inbox error");
        goto tcherror;
    }
    //  Subscribe to the server's root
    if (fmq_client_subscribe(new_node->client, "/") != 0) {
        TCHLOGE("fmq client subscribe error");
        goto tcherror;
    }
    //  Get a reference to the msgpipe
    new_node->msgpipe = fmq_client_msgpipe(new_node->client);
    assert(new_node->msgpipe);

    tch_fmq_insertnode(new_node);

    return TCH_OK;

tcherror:
    tch_fmq_freenode(new_node);
    return TCH_ERROR;
}

static int
tch_cattcp(char **tcp, const char *ip)
{
    if (ip == NULL)
        return -1;
    /*
     * free old tcp if tcp have data Instructions to initiate a new file download
     */
    if (tcp != NULL && *tcp != NULL) {
        free(*tcp);
        *tcp = NULL;
    }

    char *tp = tch_strrchr(ip, ':');
    int  len = strlen(tp);

    /* tcp://localhost + ':5670' + '\0'*/
    *tcp = tch_malloc(len + 6);
    tch_memzero(*tcp, len + 6);
    tch_memcpy(*tcp, tp, len);
    *tcp = tch_strncat(*tcp, TCH_FMQ_PORT, len + 6);

    /* free strdup() function malloc memory */
    free(tp);
    return 0;
}

int
tch_fmq_freenode(tch_fmq_cs_t *node)
{
    if (node != NULL) {
        if (node->tcp != NULL) {
            free(node->tcp);
            node->tcp = NULL;
        }
        if (node->client)
            fmq_client_destroy(&node->client);  
        free(node);
    }

    return TCH_OK;
}

/* Look for non-existent node */
int 
tch_fmq_ishavenode(const char *name)
{
    if (name == NULL) {
        TCHLOGE("fmq tch_fmq_ishavenode() parameter NULL");
        return TCH_ERROR;
    }

    tch_fmq_cs_t *fs = (tch_fmq_cs_t*)zhash_lookup(server.fmqnodes, name);
    if (fs == NULL)
        return TCH_ERROR;

    return TCH_OK;
}

int 
tch_fmq_insertnode(tch_fmq_cs_t *node)
{
    if (node == NULL) {
        TCHLOGE("fmq tch_fmq_insertnode() node NULL");
        return TCH_ERROR;
    }

    if (zhash_insert(server.fmqnodes, node->node->uname, node) != 0) {
        TCHLOGE("fmq zhash_insert() error");
        return TCH_ERROR;
    }

    return TCH_OK;
}

int 
tch_fmq_deletenode(const char *name)
{
    if (name == NULL) {
        TCHLOGE("fmq tch_fmq_deletenode() parameter NULL");
        return TCH_ERROR;
    }

    zhash_delete(server.fmqnodes, name);

    return TCH_OK;
}

void 
tch_fmq_destroy()
{
    tch_fmq_cs_t *tp = (tch_fmq_cs_t*)zhash_first(server.fmqnodes);
    while (tp) {
        if (tp->tcp)
            free(tp->tcp);
        tp = (tch_fmq_cs_t*)zhash_next(server.fmqnodes);
    }

    if (zhash_size(server.fmqnodes) != 0)
        zhash_destroy(&server.fmqnodes);
}