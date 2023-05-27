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

struct _zyre_group_t {
   char              *name;      //  Group name
   zhash_t           *peers;     //  Peers in group
   bool               contest;   //  Wheather the peer actively contest for leadership of this group
   zyre_peer_t       *leader;    //  Peer that has been elected as leader for this group
   zyre_election_t   *election;  //  Election handler, is NULL if there's no active election
};

//  Callback when we remove group from container

static void
s_delete_group (void *argument)
{
    zyre_group_t *group = (zyre_group_t *) argument;
    zyre_group_destroy (&group);
}

zyre_group_t *
zyre_group_new (const char *name, zhash_t *container)
{
   zyre_group_t  *self = (zyre_group_t *) zmalloc (sizeof (zyre_group_t));
   self->name = strdup(name);
   self->peers = zhash_new();
   self->contest = false;

   //  Insert into container if requested
   if (container) {
      zhash_insert (container, name, self);
      zhash_freefn (container, name, s_delete_group);
   }
   return self;
}

//  Destroy group object

void
zyre_group_destroy (zyre_group_t **self_p)
{
   assert(self_p);
   if (*self_p) {
      zyre_group_t *self = *self_p;
      zhash_destroy(&self->peers);
      zyre_election_destroy (&self->election);
      free(self->name);
      free(self);
      *self_p = NULL;
   }
}

//  Add peer to group
//  Ignore duplicate joins
void
zyre_group_join (zyre_group_t *self, zyre_peer_t *peer)
{
   assert(self);
   assert(peer);
   zhash_insert(self->peers, zyre_peer_identity (peer), peer);
   zyre_peer_set_status(peer, zyre_peer_status (peer) + 1);
}

//  Remove peer from group

void
zyre_group_leave (zyre_group_t *self, zyre_peer_t *peer)
{
   assert(self);
   assert(peer);
   zhash_delete(self->peers, zyre_peer_identity (peer));
   zyre_peer_set_status(peer, zyre_peer_status (peer) + 1);
}

static int
s_peer_send (const char *key, void *item, void *argument)
{
   zyre_peer_t *peer = (zyre_peer_t *) item;
   zre_msg_t *msg = zre_msg_dup ((zre_msg_t *) argument);
   zyre_peer_send (peer, &msg);
   return 0;
}

//  Send message to all peers in group

void
zyre_group_send (zyre_group_t *self, zre_msg_t **msg_p)
{
   void *item;
   assert(self);
   for (item = zhash_first (self->peers); item != NULL; item = zhash_next (self->peers))
      s_peer_send(zhash_cursor (self->peers), item, *msg_p);
   zre_msg_destroy (msg_p);
}

//  Return zlist of peer ids currently in this group
//  Caller owns return value and must destroy it when done.

zlist_t *
zyre_group_peers (zyre_group_t *self)
{
   return zhash_keys(self->peers);
}

//  Find or create an election for a group

zyre_election_t *
zyre_group_require_election (zyre_group_t *self)
{
    assert (self);
    if (!self->election)
        self->election = zyre_election_new ();

    return self->election;
}

//  Enables peer to actively contest for leadership in this group.

void
zyre_group_set_contest (zyre_group_t *self) {
    assert (self);
    self->contest = true;
}

//  Returns true if this peer actively contests for leadership, otherwise
//  false.

bool
zyre_group_contest (zyre_group_t *self) {
    assert (self);
    return self->contest;
}

//  Return the election handler for this group.

zyre_election_t *
zyre_group_election (zyre_group_t *self) {
    assert (self);
    return self->election;
}

//  Sets the election handler for this group.

void
zyre_group_set_election (zyre_group_t *self, zyre_election_t *election) {
    assert (self);
    self->election = election;
}

//  Return the peer that has been elected leader of this group.

zyre_peer_t *
zyre_group_leader (zyre_group_t *self) {
    assert (self);
    return self->leader;
}

//  Sets the peer that has been elected leader of this group.

void
zyre_group_set_leader (zyre_group_t *self, zyre_peer_t *leader) {
    assert (self);
    self->leader = leader;
}
