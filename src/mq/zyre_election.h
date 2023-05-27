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

#ifndef ZYRE_ELECTION_H_INCLUDED
#define ZYRE_ELECTION_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "zyre_classes.h"

//  Create a new zyre_election
ZYRE_PRIVATE zyre_election_t *
    zyre_election_new ();

//  Destroy the zyre_election
ZYRE_PRIVATE void
    zyre_election_destroy (zyre_election_t **self_p);

ZYRE_PRIVATE bool
    zyre_election_challenger_superior (zyre_election_t *self, const char *r);

ZYRE_PRIVATE void
    zyre_election_reset (zyre_election_t *self);

ZYRE_PRIVATE const char *
    zyre_election_caw (zyre_election_t *self);

ZYRE_PRIVATE void
    zyre_election_set_caw (zyre_election_t *self, char *caw);

ZYRE_PRIVATE zyre_peer_t *
    zyre_election_father (zyre_election_t *self);

ZYRE_PRIVATE void
    zyre_election_set_father (zyre_election_t *self, zyre_peer_t *father);

ZYRE_PRIVATE zre_msg_t *
    zyre_election_build_elect_msg (zyre_election_t *self);

ZYRE_PRIVATE zre_msg_t *
    zyre_election_build_leader_msg (zyre_election_t *self);

ZYRE_PRIVATE bool
    zyre_election_supporting_challenger (zyre_election_t *self, const char *r);

ZYRE_PRIVATE void
    zyre_election_increment_erec (zyre_election_t *self);

ZYRE_PRIVATE void
    zyre_election_increment_lrec (zyre_election_t *self);

ZYRE_PRIVATE bool
    zyre_election_erec_complete (zyre_election_t *self, zyre_group_t *group);

ZYRE_PRIVATE bool
    zyre_election_lrec_complete (zyre_election_t *self, zyre_group_t *group);

ZYRE_PRIVATE bool
    zyre_election_lrec_started (zyre_election_t *self);

//  Handle received election and leader messages. Return 1 if election is
//  still in progress, 0 if election is concluded and -1 is an error occurred.
ZYRE_PRIVATE int
    zyre_election_recv (zyre_election_t *self, zre_msg_t *msg, zyre_peer_t *sender);

//  Returns the leader if an election is finished, otherwise NULL.
ZYRE_PRIVATE const char *
    zyre_election_leader (zyre_election_t *self);

//  Sets the leader if an election is finished, otherwise NULL.
ZYRE_PRIVATE void
    zyre_election_set_leader (zyre_election_t *self, char *leader);

//  Returns true if an election is finished and won.
ZYRE_PRIVATE bool
    zyre_election_finished (zyre_election_t *self);

//  Returns true if an election is won, otherwise false.
ZYRE_PRIVATE bool
    zyre_election_won (zyre_election_t *self);

//  Enable/disable verbose logging.
ZYRE_PRIVATE void
    zyre_election_set_verbose (zyre_election_t *self, bool verbose);

//  Print election status to command line
ZYRE_PRIVATE void
    zyre_election_print (zyre_election_t *self);

#ifdef __cplusplus
}
#endif

#endif