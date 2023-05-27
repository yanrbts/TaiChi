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

#ifndef __ZYRE_API_H_INCLUDED__
#define __ZYRE_API_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

#include "zyre_library.h"
#include "zyre_classes.h"

//  This is a stable class, and may not change except for emergencies. It
//  is provided in stable builds.
//  This class has draft methods, which may change over time. They are not
//  in stable releases, by default. Use --enable-drafts to enable.
//  Constructor, creates a new Zyre node. Note that until you start the
//  node it is silent and invisible to other nodes on the network.
//  The node name is provided to other nodes during discovery. If you
//  specify NULL, Zyre generates a randomized node name from the UUID.
ZYRE_EXPORT zyre_t *
    zyre_new (const char *name);

//  Destructor, destroys a Zyre node. When you destroy a node, any
//  messages it is sending or receiving will be discarded.
ZYRE_EXPORT void
    zyre_destroy (zyre_t **self_p);

//  Return our node UUID string, after successful initialization
ZYRE_EXPORT const char *
    zyre_uuid (zyre_t *self);

//  Return our node name, after successful initialization. First 6
//  characters of UUID by default.
ZYRE_EXPORT const char *
    zyre_name (zyre_t *self);

//  Set the public name of this node overriding the default. The name is
//  provide during discovery and come in each ENTER message.
ZYRE_EXPORT void
    zyre_set_name (zyre_t *self, const char *name);

//  Set node header; these are provided to other nodes during discovery
//  and come in each ENTER message.
ZYRE_EXPORT void
    zyre_set_header (zyre_t *self, const char *name, const char *format, ...) CHECK_PRINTF (3);

//  Set verbose mode; this tells the node to log all traffic as well as
//  all major events.
ZYRE_EXPORT void
    zyre_set_verbose (zyre_t *self);

//  Set UDP beacon discovery port; defaults to 5670, this call overrides
//  that so you can create independent clusters on the same network, for
//  e.g. development vs. production. Has no effect after zyre_start().
ZYRE_EXPORT void
    zyre_set_port (zyre_t *self, int port_nbr);

//  Set the peer evasiveness timeout, in milliseconds. Default is 5000.
//  This can be tuned in order to deal with expected network conditions
//  and the response time expected by the application. This is tied to
//  the beacon interval and rate of messages received.
ZYRE_EXPORT void
    zyre_set_evasive_timeout (zyre_t *self, int interval);

//  Set the peer silence timeout, in milliseconds. Default is 5000.
//  This can be tuned in order to deal with expected network conditions
//  and the response time expected by the application. This is tied to
//  the beacon interval and rate of messages received.
//  Silence is triggered one second after the timeout if peer has not
//  answered ping and has not sent any message.
//  NB: this is currently redundant with the evasiveness timeout. Both
//  affect the same timeout value.
ZYRE_EXPORT void
    zyre_set_silent_timeout (zyre_t *self, int interval);

//  Set the peer expiration timeout, in milliseconds. Default is 30000.
//  This can be tuned in order to deal with expected network conditions
//  and the response time expected by the application. This is tied to
//  the beacon interval and rate of messages received.
ZYRE_EXPORT void
    zyre_set_expired_timeout (zyre_t *self, int interval);

//  Set UDP beacon discovery interval, in milliseconds. Default is instant
//  beacon exploration followed by pinging every 1,000 msecs.
ZYRE_EXPORT void
    zyre_set_interval (zyre_t *self, size_t interval);

//  Set network interface for UDP beacons. If you do not set this, CZMQ will
//  choose an interface for you. On boxes with several interfaces you should
//  specify which one you want to use, or strange things can happen.
//  The interface may by specified by either the interface name e.g. "eth0" or
//  an IP address associalted with the interface e.g. "192.168.0.1"
ZYRE_EXPORT void
    zyre_set_interface (zyre_t *self, const char *value);

//  By default, Zyre binds to an ephemeral TCP port and broadcasts the local
//  host name using UDP beaconing. When you call this method, Zyre will use
//  gossip discovery instead of UDP beaconing. You MUST set-up the gossip
//  service separately using zyre_gossip_bind() and _connect(). Note that the
//  endpoint MUST be valid for both bind and connect operations. You can use
//  inproc://, ipc://, or tcp:// transports (for tcp://, use an IP address
//  that is meaningful to remote as well as local nodes). Returns 0 if
//  the bind was successful, else -1.
ZYRE_EXPORT int
    zyre_set_endpoint (zyre_t *self, const char *format, ...) CHECK_PRINTF (2);

//  Set-up gossip discovery of other nodes. At least one node in the cluster
//  must bind to a well-known gossip endpoint, so other nodes can connect to
//  it. Note that gossip endpoints are completely distinct from Zyre node
//  endpoints, and should not overlap (they can use the same transport).
ZYRE_EXPORT void
    zyre_gossip_bind (zyre_t *self, const char *format, ...) CHECK_PRINTF (2);

//  Set-up gossip discovery of other nodes. A node may connect to multiple
//  other nodes, for redundancy paths. For details of the gossip network
//  design, see the CZMQ zgossip class.
ZYRE_EXPORT void
    zyre_gossip_connect (zyre_t *self, const char *format, ...) CHECK_PRINTF (2);

//  Start node, after setting header values. When you start a node it
//  begins discovery and connection. Returns 0 if OK, -1 if it wasn't
//  possible to start the node.
ZYRE_EXPORT int
    zyre_start (zyre_t *self);

//  Stop node; this signals to other peers that this node will go away.
//  This is polite; however you can also just destroy the node without
//  stopping it.
ZYRE_EXPORT void
    zyre_stop (zyre_t *self);

//  Join a named group; after joining a group you can send messages to
//  the group and all Zyre nodes in that group will receive them.
ZYRE_EXPORT int
    zyre_join (zyre_t *self, const char *group);

//  Leave a group
ZYRE_EXPORT int
    zyre_leave (zyre_t *self, const char *group);

//  Receive next message from network; the message may be a control
//  message (ENTER, EXIT, JOIN, LEAVE) or data (WHISPER, SHOUT).
//  Returns zmsg_t object, or NULL if interrupted
//  Caller owns return value and must destroy it when done.
ZYRE_EXPORT zmsg_t *
    zyre_recv (zyre_t *self);

//  Send message to single peer, specified as a UUID string
//  Destroys message after sending
ZYRE_EXPORT int
    zyre_whisper (zyre_t *self, const char *peer, zmsg_t **msg_p);

//  Send message to a named group
//  Destroys message after sending
ZYRE_EXPORT int
    zyre_shout (zyre_t *self, const char *group, zmsg_t **msg_p);

//  Send formatted string to a single peer specified as UUID string
ZYRE_EXPORT int
    zyre_whispers (zyre_t *self, const char *peer, const char *format, ...) CHECK_PRINTF (3);

//  Send formatted string to a named group
ZYRE_EXPORT int
    zyre_shouts (zyre_t *self, const char *group, const char *format, ...) CHECK_PRINTF (3);

//  Return zlist of current peer ids.
//  Caller owns return value and must destroy it when done.
ZYRE_EXPORT zlist_t *
    zyre_peers (zyre_t *self);

//  Return zlist of current peers of this group.
//  Caller owns return value and must destroy it when done.
ZYRE_EXPORT zlist_t *
    zyre_peers_by_group (zyre_t *self, const char *name);

//  Return zlist of currently joined groups.
//  Caller owns return value and must destroy it when done.
ZYRE_EXPORT zlist_t *
    zyre_own_groups (zyre_t *self);

//  Return zlist of groups known through connected peers.
//  Caller owns return value and must destroy it when done.
ZYRE_EXPORT zlist_t *
    zyre_peer_groups (zyre_t *self);

//  Return the endpoint of a connected peer.
//  Returns empty string if peer does not exist.
//  Caller owns return value and must destroy it when done.
ZYRE_EXPORT char *
    zyre_peer_address (zyre_t *self, const char *peer);

//  Return the value of a header of a conected peer.
//  Returns null if peer or key doesn't exits.
//  Caller owns return value and must destroy it when done.
ZYRE_EXPORT char *
    zyre_peer_header_value (zyre_t *self, const char *peer, const char *name);

//  Return socket for talking to the Zyre node, for polling
ZYRE_EXPORT zsock_t *
    zyre_socket (zyre_t *self);

//  Print zyre node information to stdout
ZYRE_EXPORT void
    zyre_print (zyre_t *self);

//  Return the Zyre version for run-time API detection; returns
//  major * 10000 + minor * 100 + patch, as a single integer.
ZYRE_EXPORT uint64_t
    zyre_version (void);

//  *** Draft method, for development use, may change without warning ***
//  Set the TCP port bound by the ROUTER peer-to-peer socket (beacon mode).
//  Defaults to * (the port is randomly assigned by the system).
//  This call overrides this, to bypass some firewall issues when ports are
//  random. Has no effect after zyre_start().
ZYRE_EXPORT void
    zyre_set_beacon_peer_port (zyre_t *self, int port_nbr);

//  *** Draft method, for development use, may change without warning ***
//  This options enables a peer to actively contest for leadership in the
//  given group. If this option is not set the peer will still participate in
//  elections but never gets elected. This ensures that a consent for a leader
//  is reached within a group even though not every peer is contesting for
//  leadership.
ZYRE_EXPORT void
    zyre_set_contest_in_group (zyre_t *self, const char *group);

//  *** Draft method, for development use, may change without warning ***
//  Set an alternative endpoint value when using GOSSIP ONLY. This is useful
//  if you're advertising an endpoint behind a NAT.
ZYRE_EXPORT void
    zyre_set_advertised_endpoint (zyre_t *self, const char *value);

//  *** Draft method, for development use, may change without warning ***
//  Apply a azcert to a Zyre node.
ZYRE_EXPORT void
    zyre_set_zcert (zyre_t *self, zcert_t *zcert);

//  *** Draft method, for development use, may change without warning ***
//  Specify the ZAP domain (for use with CURVE).
ZYRE_EXPORT void
    zyre_set_zap_domain (zyre_t *self, const char *domain);

//  *** Draft method, for development use, may change without warning ***
//  Set-up gossip discovery with CURVE enabled.
ZYRE_EXPORT void
    zyre_gossip_connect_curve (zyre_t *self, const char *public_key, const char *format, ...) CHECK_PRINTF (3);

//  *** Draft method, for development use, may change without warning ***
//  Unpublish a GOSSIP node from local list, useful in removing nodes from list when they EXIT
ZYRE_EXPORT void
    zyre_gossip_unpublish (zyre_t *self, const char *node);

//  *** Draft method, for development use, may change without warning ***
//  Explicitly connect to a peer
ZYRE_EXPORT int
    zyre_require_peer (zyre_t *self, const char *uuid, const char *endpoint, const char *public_key);

//  *** Draft method, for development use, may change without warning ***
//  Return underlying ZMQ socket for talking to the Zyre node,
//  for polling with libzmq (base ZMQ library)
ZYRE_EXPORT void *
    zyre_socket_zmq (zyre_t *self);

#define zyre_dump(z) zyre_print((z))


#ifdef __cplusplus
}
#endif
#endif