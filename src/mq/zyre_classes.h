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

//  Asserts check the invariants of methods. If they're not
//  fulfilled the program should fail fast. Therefore enforce them!
#ifdef NDEBUG
  #undef NDEBUG
  #include <assert.h>
  #define NDEBUG
#else
  #include <assert.h>
#endif

#include "zyre_platform.h"
#include "zyre.h"
#include "zre_msg.h"
#include "zyre_event.h"
#include "zyre_peer.h"
#include "zyre_group.h"
#include "zyre_election.h"
#include "zyre_node.h"

typedef struct _zre_msg_t           zre_msg_t;
typedef struct _zyre_t              zyre_t;
typedef struct _zyre_event_t        zyre_event_t;
typedef struct _zyre_peer_t         zyre_peer_t;
typedef struct _zyre_group_t        zyre_group_t;
typedef struct _zyre_election_t     zyre_election_t;
typedef struct _zyre_node_t         zyre_node_t;