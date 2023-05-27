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

static int  tch_initserver();
static void tch_delserver();
static void *thread_start(void *arg);

/*
 * Each machine node has a storage data structure, this structure stores 
 * all LAN nodes connected to file transfer, this structure is the main 
 * data maintenance structure, and all operations are based on this structure
 */
tch_data_t server;

//  This actor will listen and publish anything received
//  on the CHAT group

static void 
msg_actor(zsock_t *pipe, void *args)
{
    zyre_t *node = zyre_new((char *) args);
    if (!node)
        return;                 //  Could not create new node
    zyre_start(node);
    zyre_join(node, "CHAT");
    zsock_signal(pipe, 0);     //  Signal "ready" to caller

    bool terminated = false;
    zpoller_t *poller = zpoller_new(pipe, zyre_socket(node),NULL);
    while (!terminated) {
        void *which = zpoller_wait(poller, -1);
        if (which == pipe) {
            zmsg_t *msg = zmsg_recv(which);
            if (!msg)
                break;              //  Interrupted

            char *command = zmsg_popstr(msg);
            if (streq (command, "$TERM")) {
                terminated = true;
            } else if (streq(command, "SHOUT")) {
                char *string = zmsg_popstr(msg);
                zyre_shouts(node, "CHAT", "%s", string);
            } else {
                puts ("E: invalid message to actor");
                assert (false);
            }
            free(command);
            zmsg_destroy(&msg);
        } else if (which == zyre_socket(node)) {
            zmsg_t *msg = zmsg_recv(which);
            char *event = zmsg_popstr(msg);
            char *peer = zmsg_popstr(msg);
            char *name = zmsg_popstr(msg);
            char *group = zmsg_popstr(msg);
            char *message = zmsg_popstr(msg);
            
            if (streq(event, "ENTER")) {
                tch_insert_node(name, zyre_peer_address(node, peer), (zsock_t*)which/*zsock_endpoint(which)*/);
            } else if (streq(event, "EXIT")) {
                tch_del_node(name);
            } else if (streq(event, "SHOUT")) {
                //printf("event : %s msg : %s\n", event, message);
                if (tch_strcmp(TCH_FMQ_SERVER, message) == 0)
                    tch_setfmq_node(name);
            }
                
            /*else if (streq (event, "EVASIVE"))
                printf ("%s is being evasive\n", name);
            else if (streq (event, "SILENT"))
                printf ("%s is being silent\n", name);*/

            free (event);
            free (peer);
            free (name);
            free (group);
            free (message);
            zmsg_destroy (&msg);
        }
    }
    zpoller_destroy(&poller);
    zyre_stop(node);
    zclock_sleep(100);
    zyre_destroy(&node);
}

static void 
tch_sigint(int signo) {
    tch_fmq_destroy();
    tch_delserver();
    abort();
}
//#define DIR_PERMS (S_IRWXU | S_IRWXG | S_IRWXO)
//#define UMASK_SETTING (S_IWGRP | S_IXGRP | S_IWOTH | S_IXOTH)
int 
main(int argc, char **argv)
{
    pthread_t tid1;

    // signal
    if (signal(SIGINT, tch_sigint) == SIG_ERR) {
        TCHLOGE("SIGINT error %d, %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (tch_initserver() == -1) {
        TCHLOGE("Init server struct fail.");
        return 0;
    }
    tch_show_logo(server.local_node.ip, server.local_node.uname);

    pthread_create(&tid1, NULL, thread_start, NULL);
    //zloop_start(server.loop);
    tch_console_loop();
    tch_fmq_destroy();
    tch_delserver();
    
    return 0;
}

static int 
tch_initserver()
{
    /* init local host infomation*/
    tch_memzero(&server, sizeof(server));
    if (tch_gethost(server.local_node.uname,
                    sizeof(server.local_node.uname)/sizeof(char)) == -1) {
        tch_error("tch_gethost()");
        return -1;
    }

    tch_getip(server.local_node.ip, sizeof(server.local_node.ip)/sizeof(char));
    
    server.actor = zactor_new(msg_actor, server.local_node.uname);
    assert(server.actor);

    server.file_actor = zactor_new(fmq_server, server.local_node.uname);
    assert(server.file_actor);

    /* If it is the same number segment, there are 254 */
    server.nodes = tch_malloc(TCH_MACNAME * sizeof(tch_lannode_t));
    tch_memzero(server.nodes, TCH_MACNAME * sizeof(tch_lannode_t));

    /*fmq init*/
    server.fmq.sfg = 0;
    tch_memzero(server.fmq.svpath, sizeof(server.fmq.svpath));
    
    server.fmqnodes = zhash_new();
    assert(server.fmqnodes);
    return 0;
}

static void
tch_delserver()
{
    if (server.actor != NULL) {
        zactor_destroy(&server.actor);
        server.actor = NULL;
    }
    /* delete node data*/
    tch_lannode_t *node = NULL;
    for (node = server.nodes; node != NULL; node++) {
        free(node);
        node = NULL;
    }
}

int
tch_quit(int argc, char **argv)
{
    TCH_UNUSED(argc);
    TCH_UNUSED(argv);

	tch_fmq_destroy();
    tch_delserver();
	return 0;
}

static void *
thread_start(void *arg)
{
    while (!zsys_interrupted) {
        //char message [1024];
        //if (!fgets(message, 1024, stdin))
         //   break;
        //strcpy(message, "hello world");
        //message[strlen(message) - 1] = 0;     // Drop the trailing linefeed
        //zstr_sendx(server.actor, "SHOUT", message, NULL);
    }
    return((void *)0);
}