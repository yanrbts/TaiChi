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

#include "zyre_classes.h"


struct _zyre_peer_t {
    zsock_t         *mailbox;        //  Socket through to peer
    zuuid_t         *uuid;           //  Identity object
    char            *endpoint;       //  Endpoint connected to
    char            *name;           //  Peer's public name
    char            *origin;         //  Origin node's public name
    uint64_t         evasive_at;     //  Peer is being evasive
    uint64_t         expired_at;     //  Peer has expired by now
    bool             connected;      //  Peer will send messages
    bool             ready;          //  Peer has said Hello to us
    byte             status;         //  Our status counter
    uint16_t         sent_sequence;  //  Outgoing message sequence
    uint16_t         want_sequence;  //  Incoming message sequence
    zhash_t         *headers;        //  Peer headers
    bool             verbose;        //  Do we log traffic & failures?
    char            *public_key;     // curve public key
    char            *secret_key;     // curve secret key
    char            *server_key;     // curve server [remote endpoint] key
};


//  Callback when we remove peer from container

static void
s_delete_peer (void *argument)
{
    zyre_peer_t *peer = (zyre_peer_t *) argument;
    zyre_peer_destroy (&peer);
}

//  Construct new peer object

zyre_peer_t *
zyre_peer_new (zhash_t *container, zuuid_t *uuid)
{
    zyre_peer_t *self = (zyre_peer_t *) zmalloc (sizeof(zyre_peer_t));
    self->uuid = zuuid_dup(uuid);
    self->ready = false;
    self->connected = false;
    self->sent_sequence = 0;
    self->want_sequence = 0;

    //  Insert into container if requested
    if (container) {
        int rc = zhash_insert(container, zuuid_str(self->uuid), self);
        assert(rc == 0);
        zhash_freefn (container, zuuid_str (self->uuid), s_delete_peer);
    }
    return self;
}

//  Destroy peer object

void
zyre_peer_destroy (zyre_peer_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        zyre_peer_t *self = *self_p;
        zyre_peer_disconnect (self);
        zhash_destroy(&self->headers);
        zuuid_destroy(&self->uuid);
        free (self->name);
        free (self->origin);
        free (self->server_key);
        free (self->public_key);
        free (self->secret_key);
        free (self);
        *self_p = NULL;
    }
}

//  Connect peer mailbox
//  Configures mailbox and connects to peer's router endpoint

int
zyre_peer_connect (zyre_peer_t *self, zuuid_t *from, const char *endpoint, uint64_t expired_timeout)
{
    assert(self);
    assert(!self->connected);

    //  Create new outgoing socket (drop any messages in transit)
    self->mailbox = zsock_new(ZMQ_DEALER);
    if (!self->mailbox) 
        return -1;      //  Null when we're shutting down

    //  Set our own identity on the socket so that receiving node
    //  knows who each message came from. Note that we cannot use
    //  the UUID directly as the identity since it may contain a
    //  zero byte at the start, which libzmq does not like for
    //  historical and arguably bogus reasons that it nonetheless
    //  enforces.
    byte routing_id [ZUUID_LEN + 1] = { 1 };
    memcpy(routing_id + 1, zuuid_data(from), ZUUID_LEN);
    int rc = zmq_setsockopt(zsock_resolve(self->mailbox), ZMQ_IDENTITY, routing_id, ZUUID_LEN + 1);
    assert(rc == 0);

     //  Set a high-water mark that allows for reasonable activity
    zsock_set_sndhwm (self->mailbox, expired_timeout * 100);

    //  Send messages immediately or return EAGAIN
    zsock_set_sndtimeo (self->mailbox, 0);

    //  If the peer is a link-local IPv6 address but the interface is not set,
    //  use ZSYS_INTERFACE_ADDRESS if provided
    zrex_t *rex = zrex_new(NULL);
    char endpoint_iface [NI_MAXHOST] = {0};
    if (zsys_ipv6 () 
        && zsys_interface () 
        && strlen(zsys_interface ()) 
        && !streq (zsys_interface (), "*") 
        && zrex_eq (rex, endpoint, "^tcp://(fe80[^%]+)(:\\d+)$")) {
        
        const char *hostname, *port;
        zrex_fetch(rex, &hostname, &port, NULL);
        strcat (endpoint_iface, "tcp://");
        strcat (endpoint_iface, hostname);
        strcat (endpoint_iface, "%");
        strcat (endpoint_iface, zsys_interface ());
        strcat (endpoint_iface, port);
    } else {
        strcat (endpoint_iface, endpoint);
    }
    zrex_destroy (&rex);

    if (self->server_key) {
        assert(self->public_key);
        assert(self->server_key);

        // zarmour use for string conversion
        zarmour_t *armour = zarmour_new ();
        zarmour_set_mode(armour, ZARMOUR_MODE_Z85);
        zarmour_set_pad(armour, false);
        zarmour_set_line_breaks(armour,false);

        // convert keys from Z85 strings (40 bytes) to raw byte arrays (32 bytes)
        zchunk_t *decoded_public_key = zarmour_decode (armour, self->public_key);
        zchunk_t *decoded_secret_key = zarmour_decode (armour, self->secret_key);

        zcert_t *cert = zcert_new_from (zchunk_data (decoded_public_key), zchunk_data (decoded_secret_key));

        zcert_apply(cert, self->mailbox);
        zsock_set_curve_serverkey (self->mailbox, self->server_key);

        zcert_destroy(&cert);
        zchunk_destroy (&decoded_secret_key);
        zchunk_destroy (&decoded_public_key);
        zarmour_destroy (&armour);

#ifndef ZMQ_CURVE
        // legacy ZMQ support
        // inline incase the underlying assert is removed
        bool ZMQ_CURVE = false;
#endif
        assert(zsock_mechanism (self->mailbox) == ZMQ_CURVE);
    }
    //  Connect through to peer node
    rc = zsock_connect(self->mailbox, "%s", endpoint_iface);
    if (rc != 0) {
        zsys_debug ("(%s) cannot connect to endpoint=%s", self->origin, endpoint_iface);
        zsock_destroy(&self->mailbox);
        return -1;
    }
    if (self->verbose)
        zsys_info ("(%s) connect to peer: endpoint=%s", self->origin, endpoint_iface);

    self->endpoint = strdup(endpoint_iface);
    self->connected = true;
    self->ready = false;

    return 0;
}

//  Disconnect peer mailbox
//  No more messages will be sent to peer until connected again

void
zyre_peer_disconnect (zyre_peer_t *self)
{
    //  If connected, destroy socket and drop all pending messages
    assert(self);
    if (self->connected) {
        zsock_destroy(&self->mailbox);
        free(self->endpoint);
        self->mailbox = NULL;
        self->endpoint = NULL;
        self->connected = false;
        self->ready = false;
    }
}

//  Return peer connected status

bool
zyre_peer_connected (zyre_peer_t *self)
{
    assert(self);
    return self->connected;
}

//  Return peer connection endpoint

const char *
zyre_peer_endpoint (zyre_peer_t *self)
{
    assert(self);
    if (self->connected)
        return self->endpoint;
    else 
        return "";
}

//  Send message to peer

int
zyre_peer_send (zyre_peer_t *self, zre_msg_t **msg_p)
{
    assert(self);
    zre_msg_t *msg = *msg_p;
    assert(msg);
    if (self->connected) {
        self->sent_sequence += 1;
        zre_msg_set_sequence(msg, self->sent_sequence);
        if (self->verbose)
            zsys_info ("(%s) send %s to peer=%s sequence=%d",
                self->origin,
                zre_msg_command (msg),
                self->name? self->name: "-",
                zre_msg_sequence (msg));

        if (zre_msg_send (msg, self->mailbox)) {
            if (errno == EAGAIN) {
                if (self->verbose)
                    zsys_info("(%s) disconnect from peer (EAGAIN): name=%s", self->origin, self->name);
                zyre_peer_disconnect(self);
                return -1;
            }
            //  Can't get any other error here
            assert (false);
        }
    }

    zre_msg_destroy (msg_p);

    return 0;
}

//  Return peer identity string

const char *
zyre_peer_identity (zyre_peer_t *self)
{
    assert(self);
    return zuuid_str (self->uuid);
}

//  Register activity at peer

void
zyre_peer_refresh (zyre_peer_t *self, uint64_t evasive_timeout, uint64_t expired_timeout)
{
    assert(self);
    self->evasive_at = zclock_mono () + evasive_timeout;
    self->expired_at = zclock_mono () + expired_timeout;
}

//  Return peer future evasive time

int64_t
zyre_peer_evasive_at (zyre_peer_t *self)
{
    assert (self);
    return self->evasive_at;
}

//  Return peer future expired time

int64_t
zyre_peer_expired_at (zyre_peer_t *self)
{
    assert (self);
    return self->expired_at;
}

//  Return peer name

const char *
zyre_peer_name (zyre_peer_t *self)
{
    assert (self);
    return self->name? self->name: "";
}

//  Set peer name

void
zyre_peer_set_name (zyre_peer_t *self, const char *name)
{
    assert (self);
    free (self->name);
    self->name = strdup (name);
}

//  Set current node name, for logging

void
zyre_peer_set_origin (zyre_peer_t *self, const char *origin)
{
    assert (self);
    free (self->origin);
    self->origin = strdup (origin);
}

//  Return peer cycle
//  This gives us a state change count for the peer, which we can
//  check against its claimed status, to detect message loss.

byte
zyre_peer_status (zyre_peer_t *self)
{
    assert (self);
    return self->status;
}

//  Set peer status

void
zyre_peer_set_status (zyre_peer_t *self, byte status)
{
    assert (self);
    self->status = status;
}

//  Return peer ready state

byte
zyre_peer_ready (zyre_peer_t *self)
{
    assert (self);
    return self->ready;
}

//  Set peer ready

void
zyre_peer_set_ready (zyre_peer_t *self, bool ready)
{
    assert (self);
    self->ready = ready;
}

//  Get peer header value

const char *
zyre_peer_header (zyre_peer_t *self, char *key, char *default_value)
{
    assert (self);
    char *value = NULL;
    if (self->headers)
        value = (char *) (zhash_lookup (self->headers, key));
    if (!value)
        value = default_value;

    return value;
}

//  Get peer headers table

zhash_t *
zyre_peer_headers (zyre_peer_t *self)
{
    assert (self);
    return self->headers;
}

//  Set peer headers from provided dictionary

void
zyre_peer_set_headers (zyre_peer_t *self, zhash_t *headers)
{
    assert (self);
    zhash_destroy (&self->headers);
    self->headers = zhash_dup (headers);
}

//  Check if messages were lost from peer, returns true if they were

bool
zyre_peer_messages_lost (zyre_peer_t *self, zre_msg_t *msg)
{
    assert (self);
    assert (msg);

    //  The sequence number set by the peer, and our own calculated
    //  sequence number should be the same.
    if (self->verbose)
        zsys_info ("(%s) recv %s from peer=%s sequence=%d",
            self->origin,
            zre_msg_command (msg),
            self->name? self->name: "-",
            zre_msg_sequence (msg));

    //  HELLO always MUST have sequence = 1
    if (zre_msg_id (msg) == ZRE_MSG_HELLO)
        self->want_sequence = 1;
    else
        self->want_sequence += 1;

    if (self->want_sequence != zre_msg_sequence (msg)) {
        zsys_info ("(%s) seq error from peer=%s expect=%d, got=%d",
            self->origin,
            self->name? self->name: "-",
            self->want_sequence,
            zre_msg_sequence (msg));
        return true;
    }
    return false;
}

//  Ask peer to log all traffic via zsys

void
zyre_peer_set_verbose (zyre_peer_t *self, bool verbose)
{
    assert (self);
    self->verbose = verbose;
}

//  Return want_sequence

uint16_t
zyre_peer_want_sequence (zyre_peer_t *self)
{
    assert (self);
    return self->want_sequence;
}

//  Return sent_sequence

uint16_t
zyre_peer_sent_sequence (zyre_peer_t *self)
{
    assert (self);
    return self->sent_sequence;
}

void
zyre_peer_set_public_key (zyre_peer_t *self, const char *key)
{
    assert (self);
    free (self->public_key);
    self->public_key = strdup (key);
}

void
zyre_peer_set_secret_key (zyre_peer_t *self, const char *key)
{
    assert (self);
    free (self->secret_key);
    self->secret_key = strdup (key);
}

void
zyre_peer_set_server_key (zyre_peer_t *self, const char *key)
{
    assert (self);
    free (self->server_key);
    self->server_key = strdup (key);
}
