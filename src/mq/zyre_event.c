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

struct _zyre_event_t {
    char        *type;             //  Event type as string
    char        *peer_uuid;        //  Sender UUID as string
    char        *peer_name;        //  Sender public name as string
    char        *peer_addr;        //  Sender ipaddress as string, for an ENTER event
    zhash_t     *headers;          //  Headers, for an ENTER event
    char        *group;            //  Group name for a SHOUT event
    zmsg_t      *msg;              //  Message payload for SHOUT or WHISPER
};

//  Constructor: receive an event from the zyre node, wraps zyre_recv.
//  The event may be a control message (ENTER, EXIT, JOIN, LEAVE) or
//  data (WHISPER, SHOUT).
zyre_event_t *
zyre_event_new (zyre_t *node)
{
    zmsg_t *msg = zyre_recv(node);
    if (!msg)
        return NULL;        //  Interrupted
    zyre_event_t *self = (zyre_event_t *) zmalloc (sizeof (zyre_event_t));
    assert(self);

    self->type = zmsg_popstr(msg);
    self->peer_uuid = zmsg_popstr (msg);
    self->peer_name = zmsg_popstr(msg);

    if (streq (self->type, "ENTER")) {
        zframe_t  *headers = zmsg_pop (msg);
        if (headers) {
            self->headers = zhash_unpack(headers);
            zframe_destroy(&headers);
        }
        self->peer_addr = zmsg_popstr (msg);
    } else if (streq (self->type, "JOIN")) {
        self->group = zmsg_popstr(msg);
    } else if (streq (self->type, "LEAVE")) {
        self->group = zmsg_popstr(msg);
    } else if (streq (self->type, "WHISPER")) {
        self->msg = msg;
        msg = NULL;
    } else if (streq (self->type, "SHOUT")) {
        self->group = zmsg_popstr (msg);
        self->msg = msg;
        msg = NULL;
    } else if (streq (self->type, "LEADER")) {
        self->group = zmsg_popstr (msg);
    }

    zmsg_destroy (&msg);
    return self;
}

//  Destructor; destroys an event instance

void
zyre_event_destroy (zyre_event_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        zyre_event_t *self = *self_p;
        zhash_destroy (&self->headers);
        zmsg_destroy (&self->msg);
        free (self->peer_uuid);
        free (self->peer_name);
        free (self->peer_addr);
        free (self->group);
        free (self->type);
        free (self);
        *self_p = NULL;
    }
}

static int
zyre_event_log_pair (const char *key, void *item, void *argument)
{
    zsys_info ("   - %s: %s", key, (char *) item);
    return 0;
}

//  Print event to zsys log

void
zyre_event_print (zyre_event_t *self)
{
    zsys_info ("zyre_event:");
    zsys_info (" - from name=%s uuid=%s",
        zyre_event_peer_name (self),
        zyre_event_peer_uuid (self));
    zsys_info (" - type=%s", self->type);

    if (streq (self->type, "ENTER")) {
        void *item;
        zsys_info(" - headers=%zu:", zhash_size (self->headers));
        for (item = zhash_first (self->headers); item != NULL; item = zhash_next (self->headers)) {
            zyre_event_log_pair(zhash_cursor (self->headers), item, self);
        }
        zsys_info (" - address=%s", zyre_event_peer_addr (self));

    } else if (streq (self->type, "JOIN")) {
        zsys_info (" - group=%s", zyre_event_group (self));
    } else if (streq (self->type, "JOIN")) {
        zsys_info (" - group=%s", zyre_event_group (self));
    } else if (streq (self->type, "LEAVE")) {
        zsys_info (" - group=%s", zyre_event_group (self));
    } else if (streq(self->type, "SHOUT")) {
        zsys_info (" - message:");
        zmsg_print (self->msg);
    } else if (streq (self->type, "WHISPER")) {
        zsys_info (" - message:");
        zmsg_print (self->msg);
    } else if (streq (self->type, "LEADER")) {
        zsys_info (" - group=%s", zyre_event_group (self));
    }
}

//  Returns event type, as printable uppercase string

const char *
zyre_event_type (zyre_event_t *self)
{
    assert(self);
    return self->type;
}

//  Return the sending peer's UUID as a string

const char *
zyre_event_peer_uuid (zyre_event_t *self)
{
    assert (self);
    return self->peer_uuid;
}

//  Return the sending peer's public name as a string

const char *
zyre_event_peer_name (zyre_event_t *self)
{
    assert (self);
    return self->peer_name;
}


//  --------------------------------------------------------------------------
//  Return the sending peer's ipaddress as a string

const char *
zyre_event_peer_addr (zyre_event_t *self)
{
    assert (self);
    return self->peer_addr;
}


//  --------------------------------------------------------------------------
//  Returns the event headers, or NULL if there are none

zhash_t *
zyre_event_headers (zyre_event_t *self)
{
    assert (self);
    return self->headers;
}


//  --------------------------------------------------------------------------
//  Returns value of a header from the message headers
//  obtained by ENTER. Return NULL if no value was found.

const char *
zyre_event_header (zyre_event_t *self, const char *name)
{
    assert (self);
    if (!self->headers)
        return NULL;
    return (const char *) zhash_lookup (self->headers, name);
}


//  --------------------------------------------------------------------------
//  Returns the group name that a SHOUT event was sent to

const char *
zyre_event_group (zyre_event_t *self)
{
    assert (self);
    return self->group;
}


//  --------------------------------------------------------------------------
//  Returns the incoming message payload; the caller can modify the message
//  but does not own it and should not destroy it.

zmsg_t *
zyre_event_msg (zyre_event_t *self)
{
    assert (self);
    return self->msg;
}


//  --------------------------------------------------------------------------
//  Returns the incoming message payload, and pass ownership to the caller.
//  The caller must destroy the message when finished with it. After called
//  on the given event, further calls will return NULL.

zmsg_t *
zyre_event_get_msg (zyre_event_t *self)
{
    assert (self);
    zmsg_t *msg = self->msg;
    self->msg = NULL;
    return msg;
}