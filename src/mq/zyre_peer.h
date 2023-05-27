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

#ifndef __ZYRE_PEER_H_INCLUDED__
#define __ZYRE_PEER_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

#include "zyre_library.h"
#include "zyre_classes.h"

//  Constructor
ZYRE_PRIVATE zyre_peer_t *
    zyre_peer_new (zhash_t *container, zuuid_t *uuid);

//  Destructor
ZYRE_PRIVATE void
    zyre_peer_destroy (zyre_peer_t **self_p);

//  Connect peer mailbox
ZYRE_PRIVATE int
    zyre_peer_connect (zyre_peer_t *self, zuuid_t *from, const char *endpoint, uint64_t expired_timeout);

//  Connect peer mailbox
ZYRE_PRIVATE void
    zyre_peer_disconnect (zyre_peer_t *self);

//  Return peer connected status
ZYRE_PRIVATE bool
    zyre_peer_connected (zyre_peer_t *self);

//  Return peer connection endpoint
ZYRE_PRIVATE const char *
    zyre_peer_endpoint (zyre_peer_t *self);

//  Send message to peer
ZYRE_PRIVATE int
    zyre_peer_send (zyre_peer_t *self, zre_msg_t **msg_p);

//  Return peer identity string
ZYRE_PRIVATE const char *
    zyre_peer_identity (zyre_peer_t *self);

//  Register activity at peer
ZYRE_PRIVATE void
    zyre_peer_refresh (zyre_peer_t *self, uint64_t evasive_timeout, uint64_t expired_timeout);

//  Return peer future evasive time
ZYRE_PRIVATE int64_t
    zyre_peer_evasive_at (zyre_peer_t *self);

//  Return peer future expired time
ZYRE_PRIVATE int64_t
    zyre_peer_expired_at (zyre_peer_t *self);

//  Return peer name
ZYRE_PRIVATE const char *
    zyre_peer_name (zyre_peer_t *self);

//  Set peer name
ZYRE_PRIVATE void
    zyre_peer_set_name (zyre_peer_t *self, const char *name);

//  Set current node name, for logging
ZYRE_PRIVATE void
    zyre_peer_set_origin (zyre_peer_t *self, const char *origin);

//  Return peer status
ZYRE_PRIVATE byte
    zyre_peer_status (zyre_peer_t *self);

//  Set peer status
ZYRE_PRIVATE void
    zyre_peer_set_status (zyre_peer_t *self, byte status);

//  Return peer ready state
ZYRE_PRIVATE byte
    zyre_peer_ready (zyre_peer_t *self);

//  Set peer ready
ZYRE_PRIVATE void
    zyre_peer_set_ready (zyre_peer_t *self, bool ready);

//  Get peer header value
ZYRE_PRIVATE const char *
    zyre_peer_header (zyre_peer_t *self, char *key, char *default_value);

//  Get peer headers table
ZYRE_PRIVATE zhash_t *
    zyre_peer_headers (zyre_peer_t *self);

//  Set peer headers from provided dictionary
ZYRE_PRIVATE void
    zyre_peer_set_headers (zyre_peer_t *self, zhash_t *headers);

//  Check if messages were lost from peer, returns true if they were.
ZYRE_PRIVATE bool
    zyre_peer_messages_lost (zyre_peer_t *self, zre_msg_t *msg);

//  Ask peer to log all traffic via zsys
ZYRE_PRIVATE void
    zyre_peer_set_verbose (zyre_peer_t *self, bool verbose);

//  Return want_sequence
ZYRE_PRIVATE uint16_t
    zyre_peer_want_sequence (zyre_peer_t *self);

//  Return sent_sequence
ZYRE_PRIVATE uint16_t
    zyre_peer_sent_sequence (zyre_peer_t *self);

// curve support
ZYRE_PRIVATE void
    zyre_peer_set_server_key (zyre_peer_t *self, const char *key);

ZYRE_PRIVATE void
    zyre_peer_set_public_key (zyre_peer_t *self, const char *key);

ZYRE_PRIVATE void
    zyre_peer_set_secret_key (zyre_peer_t *self, const char *key);

#ifdef __cplusplus
}
#endif

#endif