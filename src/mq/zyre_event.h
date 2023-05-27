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

#ifndef ZYRE_EVENT_H_INCLUDED
#define ZYRE_EVENT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif
    
#include "zyre_library.h"
#include "zyre_classes.h"

//  This is a stable class, and may not change except for emergencies. It
//  is provided in stable builds.
//  Constructor: receive an event from the zyre node, wraps zyre_recv.
//  The event may be a control message (ENTER, EXIT, JOIN, LEAVE) or
//  data (WHISPER, SHOUT).
ZYRE_EXPORT zyre_event_t *
    zyre_event_new (zyre_t *node);

//  Destructor; destroys an event instance
ZYRE_EXPORT void
    zyre_event_destroy (zyre_event_t **self_p);

//  Returns event type, as printable uppercase string. Choices are:
//  "ENTER", "EXIT", "JOIN", "LEAVE", "EVASIVE", "WHISPER" and "SHOUT"
//  and for the local node: "STOP"
ZYRE_EXPORT const char *
    zyre_event_type (zyre_event_t *self);

//  Return the sending peer's uuid as a string
ZYRE_EXPORT const char *
    zyre_event_peer_uuid (zyre_event_t *self);

//  Return the sending peer's public name as a string
ZYRE_EXPORT const char *
    zyre_event_peer_name (zyre_event_t *self);

//  Return the sending peer's ipaddress as a string
ZYRE_EXPORT const char *
    zyre_event_peer_addr (zyre_event_t *self);

//  Returns the event headers, or NULL if there are none
ZYRE_EXPORT zhash_t *
    zyre_event_headers (zyre_event_t *self);

//  Returns value of a header from the message headers
//  obtained by ENTER. Return NULL if no value was found.
ZYRE_EXPORT const char *
    zyre_event_header (zyre_event_t *self, const char *name);

//  Returns the group name that a SHOUT event was sent to
ZYRE_EXPORT const char *
    zyre_event_group (zyre_event_t *self);

//  Returns the incoming message payload; the caller can modify the
//  message but does not own it and should not destroy it.
ZYRE_EXPORT zmsg_t *
    zyre_event_msg (zyre_event_t *self);

//  Returns the incoming message payload, and pass ownership to the
//  caller. The caller must destroy the message when finished with it.
//  After called on the given event, further calls will return NULL.
//  Caller owns return value and must destroy it when done.
ZYRE_EXPORT zmsg_t *
    zyre_event_get_msg (zyre_event_t *self);

//  Print event to zsys log
ZYRE_EXPORT void
    zyre_event_print (zyre_event_t *self);

#ifdef __cplusplus
}
#endif

#endif