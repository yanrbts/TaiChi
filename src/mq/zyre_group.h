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
 * Zyre - Local Area Clustering for Peer-to-Peer 
 * Zyre provides reliable group messaging over local area networks. It has these key characteristics:

    Zyre needs no administration or configuration.
    Peers may join and leave the network at any time.
    Peers talk to each other without any central brokers or servers.
    Peers can talk directly to each other.
    Peers can join groups, and then talk to groups.
    Zyre is reliable, and loses no messages even when the network is heavily loaded.
    Zyre is fast and has low latency, requiring no consensus protocols.
    Zyre is designed for WiFi networks, yet also works well on Ethernet networks.
    Time for a new peer to join a network is about one second.

 *
 */

#ifndef __ZYRE_GROUP_H_INCLUDED__
#define __ZYRE_GROUP_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

#include "zyre_library.h"
#include "zyre_classes.h"

//  Constructor
ZYRE_PRIVATE zyre_group_t *
    zyre_group_new (const char *name, zhash_t *container);

//  Destructor
ZYRE_PRIVATE void
    zyre_group_destroy (zyre_group_t **self_p);

//  Add peer to group
ZYRE_PRIVATE void
    zyre_group_join (zyre_group_t *self, zyre_peer_t *peer);

//  Remove peer from group
ZYRE_PRIVATE void
    zyre_group_leave (zyre_group_t *self, zyre_peer_t *peer);

//  Send message to all peers in group
ZYRE_PRIVATE void
    zyre_group_send (zyre_group_t *self, zre_msg_t **msg_p);

//  Return zlist of peer ids currently in this group
//  Caller owns return value and must destroy it when done.
ZYRE_PRIVATE zlist_t *
   zyre_group_peers (zyre_group_t *self);

//  Find or create an election for a group
zyre_election_t *
   zyre_group_require_election (zyre_group_t *self);

//  Enables peer to actively contest for leadership in this group.
void
   zyre_group_set_contest (zyre_group_t *self);

bool
   zyre_group_contest (zyre_group_t *self);

//  Return the election handler for this group.
zyre_election_t *
    zyre_group_election (zyre_group_t *self);

//  Sets the election handler for this group.
void
    zyre_group_set_election (zyre_group_t *self, zyre_election_t *election);

//  Return the peer that has been elected leader of this group.
zyre_peer_t *
    zyre_group_leader (zyre_group_t *self);

//  Sets the peer that has been elected leader of this group.
void
    zyre_group_set_leader (zyre_group_t *self, zyre_peer_t *leader);


#ifdef __cplusplus
}
#endif

#endif