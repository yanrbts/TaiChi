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

//  Structure of our class

#include "zyre_classes.h"

struct _zyre_t {
    zactor_t        *actor;            //  A Zyre instance wraps the actor instance
    zsock_t         *inbox;            //  Receives incoming cluster traffic
    char            *uuid;             //  Copy of node UUID string
    char            *name;             //  Copy of node name
    char            *endpoint;         //  Copy of last endpoint bound to
};

//  Constructor, creates a new Zyre node. Note that until you start the
//  node it is silent and invisible to other nodes on the network.
//  The node name is provided to other nodes during discovery. If you
//  specify NULL, Zyre generates a randomized node name from the UUID.
zyre_t *
zyre_new (const char *name)
{
    zyre_t  *self = (zyre_t *) zmalloc (sizeof (zyre_t));
    assert(self);

    //  Create front-to-back pipe pair for data traffic
    zsock_t *outbox;
    self->inbox = zsys_create_pipe (&outbox);

    //  Start node engine and wait for it to be ready
    self->actor = zactor_new(zyre_node_actor, outbox);
    assert(self->actor);

    //  Send name, if any, to node ending
    if (name)
        zstr_sendx(self->actor, "SET NAME", name, NULL);

    return self;
}

//  Destructor, destroys a Zyre node. When you destroy a node, any
//  messages it is sending or receiving will be discarded.

void
zyre_destroy (zyre_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        zyre_t *self = *self_p;
        zactor_destroy(&self->actor);
        zsock_destroy (&self->inbox);
        zstr_free (&self->uuid);
        zstr_free (&self->name);
        zstr_free (&self->endpoint);
        free (self);
        *self_p = NULL;
    }
}

//  Return our node UUID string, after successful initialization
const char *
zyre_uuid (zyre_t *self)
{
    assert(self);
    //  Hold uuid string in zyre object so caller gets a safe reference
    zstr_free(&self->uuid);
    zstr_sendx(self->actor,"UUID", NULL);
    self->uuid = zstr_recv (self->actor);
    return self->uuid;
}

//  Return our node name, after successful initialization. By default
//  is taken from the UUID and shortened.
const char *
zyre_name (zyre_t *self)
{
    assert(self);
    //  Hold name in zyre object so caller gets a safe reference
    zstr_free(&self->name);
    zstr_sendx(self->actor, "NAME", NULL);
    self->name = zstr_recv (self->actor);
    return self->name;
}

//  Set the public name of this node overriding the default. The name is
//  provide during discovery and come in each ENTER message.
void
zyre_set_name (zyre_t *self, const char *name)
{
    assert(self);
    assert(name);
    zstr_sendx(self->actor, "SET NAME", name, NULL);
}

//  Set node header; these are provided to other nodes during discovery
//  and come in each ENTER message.
void
zyre_set_header (zyre_t *self, const char *name, const char *format, ...)
{
    assert(self);
    assert(name);
    assert(format);

    va_list argptr;
    va_start(argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end(argptr);

    zstr_sendx(self->actor, "SET HEADER", name, string, NULL);
    zstr_free(&string);
}

//  Set verbose mode; this tells the node to log all traffic as well as
//  all major events.
void
zyre_set_verbose (zyre_t *self)
{
    assert(self);
    zstr_sendx(self->actor, "SET VERBOSE", "1", NULL);
}

//  Set UDP beacon discovery port; defaults to 5670, this call overrides
//  that so you can create independent clusters on the same network, for
//  e.g. development vs. production. Has no effect after zyre_start().
void
zyre_set_port (zyre_t *self, int port_nbr)
{
    assert(self);
    zstr_sendm(self->actor, "SET PORT");
    zstr_sendf(self->actor, "%d", port_nbr);
}

//  Set the node evasiveness timeout, in milliseconds. Default is 5000.
//  This can be tuned in order to deal with expected network conditions
//  and the response time expected by the application. This is tied to
//  the beacon interval and rate of messages received.
void
zyre_set_evasive_timeout (zyre_t *self, int interval)
{
    assert(self);
    zstr_sendm(self->actor, "SET EVASIVE TIMEOUT");
    zstr_sendf(self->actor, "%d", interval);
}

//  Set the node silence timeout, in milliseconds. Default is 5000.
//  Silence means that a peer does not send messages and does not
//  answer to ping. SILENT event is triggered one second after the
//  configured silence timeout, and every second after that until
//  the expired timeout is reached.
//  This can be tuned in order to deal with expected network conditions
//  and the response time expected by the application. This is tied to
//  the beacon interval and rate of messages received.
//  NB: in current implementation, zyre_set_silent_timeout is redundant
//  with zyre_set_evasive_timeout and calls the same code underneath.
void
zyre_set_silent_timeout (zyre_t *self, int interval)
{
    assert(self);
    zstr_sendm(self->actor, "SET SILENT TIMEOUT");
    zstr_sendf(self->actor, "%d", interval);
}

//  Set the node expiration timeout, in milliseconds. Default is 30000.
//  This can be tuned in order to deal with expected network conditions
//  and the response time expected by the application. This is tied to
//  the beacon interval and rate of messages received.
void
zyre_set_expired_timeout (zyre_t *self, int interval)
{
    assert(self);
    zstr_sendm(self->actor, "SET EXPIRED TIMEOUT");
    zstr_sendf(self->actor, "%d", interval); 
}

//  Set UDP beacon discovery interval, in milliseconds. Default is instant
//  beacon exploration followed by pinging every 1,000 msecs.
void
zyre_set_interval (zyre_t *self, size_t interval)
{
    assert(self);
    zstr_sendm(self->actor, "SET INTERVAL");
    zstr_sendf(self->actor, "%zd", interval);
}

//  Set network interface for UDP beacons. If you do not set this, CZMQ will
//  choose an interface for you. On boxes with several interfaces you should
//  specify which one you want to use, or strange things can happen.
void
zyre_set_interface (zyre_t *self, const char *value)
{
    assert (self);
    assert (value);

    //  Implemented by zsys global for now
    zsys_set_interface(value);
}

//  By default, Zyre binds to an ephemeral TCP port and broadcasts the local
//  host name using UDP beaconing. When you call this method, Zyre will use
//  gossip discovery instead of UDP beaconing. You MUST set-up the gossip
//  service separately using zyre_gossip_bind() and _connect(). Note that the
//  endpoint MUST be valid for both bind and connect operations. You can use
//  inproc://, ipc://, or tcp:// transports (for tcp://, use an IP address
//  that is meaningful to remote as well as local nodes). Returns 0 if
//  the bind was successful, else -1.
int
zyre_set_endpoint (zyre_t *self, const char *format, ...)
{
    assert(self);
    assert(format);

    va_list argptr;
    va_start(argptr, format);
    char *string = zsys_vprintf(format, argptr);
    va_end(argptr);

    zstr_sendx(self->actor, "SET ENDPOINT", string, NULL);
    free(string);

    return zsock_wait(self->actor) == 0? 0: -1;
}

//  Set-up gossip discovery of other nodes. At least one node in the cluster
//  must bind to a well-known gossip endpoint, so other nodes can connect to
//  it. Note that gossip endpoints are completely distinct from Zyre node
//  endpoints, and should not overlap (they can use the same transport).

void
zyre_gossip_bind (zyre_t *self, const char *format, ...)
{
    assert(self);
    assert(format);

    va_list argptr;
    va_start(argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end(argptr);

    zstr_sendx(self->actor, "GOSSIP BIND", string, NULL);
    free(string);
}

//  Set-up gossip discovery of other nodes. A node may connect to multiple
//  other nodes, for redundancy paths. For details of the gossip network
//  design, see the CZMQ zgossip class.

void
zyre_gossip_connect (zyre_t *self, const char *format, ...)
{
    assert(self);
    assert(format);

    va_list argptr;
    va_start(argptr,format);
    char *string = zsys_vprintf (format, argptr);
    va_end(argptr);

    zstr_sendx(self->actor, "GOSSIP CONNECT", string, NULL);
    free(string);
}

//  Start node, after setting header values. When you start a node it
//  begins discovery and connection. Returns 0 if OK, -1 if it wasn't
//  possible to start the node. If you want to use gossip discovery, set
//  the endpoint (optionally), then bind/connect the gossip network, and
//  only then start the node.

int
zyre_start (zyre_t *self)
{
    assert(self);

    zstr_sendx(self->actor, "START", NULL);
    return zsock_wait(self->actor) == 0? 0: -1;
}

//  Stop node; this signals to other peers that this node will go away.
//  This is polite; however you can also just destroy the node without
//  stopping it.

void
zyre_stop (zyre_t *self)
{
    assert (self);

    zstr_sendx (self->actor, "STOP", NULL);
    zsock_wait (self->actor);
}

//  Join a named group; after joining a group you can send messages to
//  the group and all Zyre nodes in that group will receive them.

int
zyre_join (zyre_t *self, const char *group)
{
    assert(self);
    assert(group);

    zstr_sendx(self->actor, "JOIN", group, NULL);
    return 0;
}

//  Leave a group

int
zyre_leave (zyre_t *self, const char *group)
{
    assert(self);
    assert(group);

    zstr_sendx (self->actor, "LEAVE", group, NULL);
    return 0;
}

//  Receive next message from network; the message may be a control
//  message (ENTER, EXIT, JOIN, LEAVE) or data (WHISPER, SHOUT).
//  Returns zmsg_t object, or NULL if interrupted

zmsg_t *
zyre_recv (zyre_t *self)
{
    assert (self);
    return zmsg_recv (self->inbox);
}

//  Send message to single peer, specified as a UUID string
//  Destroys message after sending

int
zyre_whisper (zyre_t *self, const char *peer, zmsg_t **msg_p)
{
    assert(self);
    assert(peer);
    assert(msg_p);

    zstr_sendm(self->actor, "WHISPER");
    zstr_sendm (self->actor, peer);
    zmsg_send (msg_p, self->actor);
    return 0;
}

//  Send message to a named group
//  Destroys message after sending

int
zyre_shout (zyre_t *self, const char *group, zmsg_t **msg_p)
{
    assert(self);
    assert(group);
    assert(msg_p);

    if (zstr_sendm(self->actor, "SHOUT") == -1) {
        return -1;
    }

    if (zstr_sendm(self->actor, group) == -1) {
        return -1;
    }

    return zmsg_send (msg_p, self->actor);
}

//  Send formatted string to a single peer specified as UUID string

int
zyre_whispers (zyre_t *self, const char *peer, const char *format, ...)
{
    assert(self);
    assert(peer);
    assert(format);

    va_list argptr;
    va_start(argptr,format);
    char *string = zsys_vprintf (format, argptr);
    va_end(argptr);

    zstr_sendm (self->actor, "WHISPER");
    zstr_sendm (self->actor, peer);
    zstr_send (self->actor, string);
    free (string);
    return 0;
}

//  Send formatted string to a named group

int
zyre_shouts (zyre_t *self, const char *group, const char *format, ...)
{
    assert(self);
    assert(group);
    assert(format);

    va_list argptr;
    va_start(argptr,format);
    char *string = zsys_vprintf (format, argptr);
    va_end(argptr);

    zstr_sendm(self->actor, "SHOUT");
    zstr_sendm(self->actor, group);
    zstr_send  (self->actor, string);
    free (string);
    return 0;
}

//  Return zlist of current peers. The caller owns this list and should
//  destroy it when finished with it.

zlist_t *
zyre_peers (zyre_t *self)
{
    assert(self);

    zlist_t *peers;
    zstr_send(self->actor, "PEERS");
    zsock_recv(self->actor, "p", &peers);
    return peers;
}

//  Return zlist of current peers of this group. The caller owns this list and
//  should destroy it when finished with it.

zlist_t *
zyre_peers_by_group (zyre_t *self, const char *group)
{
    assert(self);
    assert(group);

    zlist_t *peers;
    zstr_sendm(self->actor, "GROUP PEERS");
    zstr_send (self->actor, group);
    zsock_recv (self->actor, "p", &peers);
    return peers;
}

//  Return zlist of currently joined groups. The caller owns this list and
//  should destroy it when finished with it.

zlist_t *
zyre_own_groups (zyre_t *self)
{
    assert(self);

    zlist_t *groups;
    zstr_send (self->actor, "OWN GROUPS");
    zsock_recv(self->actor, "p", &groups);
    return groups;
}

//  Return zlist of groups known through connected peers. The caller owns this
//  list and should destroy it when finished with it.

zlist_t *
zyre_peer_groups (zyre_t *self)
{
    assert(self);

    zlist_t *groups;
    zstr_send(self->actor, "PEER GROUPS");
    zsock_recv(self->actor, "p", &groups);
    return groups;
}

//  Return the endpoint of a connected peer. Returns empty string if
//  the peer does not exist. Caller owns the string.

char *
zyre_peer_address (zyre_t *self, const char *peer)
{
    assert(self);
    assert(peer);

    char *address;
    zstr_sendm(self->actor, "PEER ENDPOINT");
    zstr_send(self->actor, peer);
    zsock_recv (self->actor, "s", &address);
    return address;
}

//  Return the value of a header of a conected peer.  Returns null if peer
//  or key doesn't exits. Caller owns the string.

char *
zyre_peer_header_value (zyre_t *self, const char *peer, const char *name)
{
    assert(self);
    assert(peer);
    assert(name);

    zstr_sendm(self->actor, "PEER HEADER");
    zstr_sendm(self->actor, peer);
    zstr_send(self->actor, name);
    return zstr_recv (self->actor);
}

//  Return node zsock_t socket, for direct polling of socket

zsock_t *
zyre_socket (zyre_t *self)
{
    assert (self);

    return self->inbox;
}

//  Prints zyre node information

void
zyre_print (zyre_t *self)
{
    assert (self);

    zstr_send (self->actor, "DUMP");
}

//  Return the Zyre version for run-time API detection; returns
//  major * 10000 + minor * 100 + patch, as a single integer.

uint64_t
zyre_version (void)
{
    return ZYRE_VERSION;
}

//  Set TCP ephemeral port for beacon; defaults to 0, and the port is random.
//  This call overrides this to bypass some firewall issues with random ports.
//  Has no effect after zyre_start().

void
zyre_set_beacon_peer_port (zyre_t *self, int port)
{
    assert(self);

    zstr_sendm (self->actor, "SET EPHEMERAL PORT");
    zstr_sendf (self->actor, "%d", port);
}

//  This options enables a peer to actively contest for leadership in the
//  given group. If this option is not set the peer will still participate in
//  elections but never gets elected. This ensures that a consent for a leader
//  is reached within a group even though not every peer is contesting for
//  leadership.

void
zyre_set_contest_in_group (zyre_t *self, const char *group) 
{
    assert (self);
    assert (group);
    zstr_sendx (self->actor, "SET CONTEST" , group, NULL);
}

void
zyre_set_advertised_endpoint (zyre_t *self, const char *endpoint)
{
    assert (self);
    assert (endpoint);

    zstr_sendx (self->actor, "SET ADVERTISED ENDPOINT", endpoint, NULL);
}

//  *** Draft method, for development use, may change without warning ***
//  Apply a azcert to a Zyre node.

void 
zyre_set_zcert(zyre_t *self, zcert_t *zcert)
{
    assert(self);
    assert(zcert);

    // actor will assert check the keys
    zstr_sendx (self->actor, "SET PUBLICKEY", zcert_public_txt(zcert), NULL);
    zstr_sendx (self->actor, "SET SECRETKEY", zcert_secret_txt(zcert), NULL);
}

//  *** Draft method, for development use, may change without warning ***
//  Specify the ZAP domain (for use with CURVE).

void 
zyre_set_zap_domain(zyre_t *self, const char *domain)
{
    assert(self);
    assert(domain);

    zstr_sendx (self->actor, "ZAP DOMAIN", domain, NULL);
}

//  *** Draft method, for development use, may change without warning ***
//  Set-up gossip discovery with CURVE enabled.

void
zyre_gossip_connect_curve (zyre_t *self, const char *public_key, const char *format, ...)
{
    assert(self);
    assert(public_key);
    assert(format);

    va_list argptr;
    va_start(argptr, format);
    char *string = zsys_vprintf(format, argptr);
    va_end (argptr);

    zstr_sendx (self->actor, "GOSSIP CONNECT", string, public_key, NULL);
    free (string);
}

//  Inform gossip to remove a node from it's master (tuples) list
//  Useful when tracking nodes activity across the mesh

void
zyre_gossip_unpublish (zyre_t *self, const char *node)
{
    assert(self);
    assert(node);

    zstr_sendx (self->actor, "GOSSIP UNPUBLISH", node, NULL);
}

//  Explicitly connect to a peer

int
zyre_require_peer (zyre_t *self, const char *uuid, const char *endpoint, const char *public_key)
{
    assert (self);
    assert (uuid);
    assert (endpoint);
    assert (public_key);

    return zstr_sendx (self->actor, "REQUIRE PEER", uuid, endpoint, public_key, NULL);
}

void *zyre_socket_zmq (zyre_t *self)
{
    assert (self);

    return zsock_resolve (self->inbox);
}