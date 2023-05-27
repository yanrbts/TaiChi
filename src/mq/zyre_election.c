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

struct _zyre_election_t {
    char            *caw;               //  Current active wave
    zyre_peer_t     *father;            //  Father in the current active wave
    unsigned int     erec;              //  Number of received election messages
    unsigned int     lrec;              //  Number of received leader messages
    bool             state;             //  True if leader else false

    char            *leader;            //  Leader identity
};

//  Create a new zyre_election

zyre_election_t *
zyre_election_new ()
{
    zyre_election_t *self = (zyre_election_t *) zmalloc (sizeof (zyre_election_t));
    assert(self);
    //  Initialize class properties here
    self->caw = NULL;
    self->father = NULL;
    self->erec = 0;
    self->lrec = 0;
    self->state = false;
    self->leader = NULL;

    return self;
}

//  Destroy the zyre_election
void
zyre_election_destroy (zyre_election_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        zyre_election_t *self = *self_p;
        zstr_free(&self->caw);
        zstr_free(&self->leader);
        free(self);
        *self_p = NULL;
    }
}

bool
zyre_election_challenger_superior (zyre_election_t *self, const char *r) 
{
    assert(self);
    assert(r);
    return !self->caw || strcmp(r, self->caw) < 0;
}

void
zyre_election_reset (zyre_election_t *self)
{
    assert(self);
    zstr_free(&self->caw);          //  Free caw when re-initiated
    zstr_free(&self->leader);       //  Free leader when re-initiated
    self->father = NULL;            //  Reset father when re-initiated
    self->erec = 0;
    self->lrec = 0;
}

const char *
zyre_election_caw (zyre_election_t *self)
{
    assert (self);
    return self->caw;
}

void
zyre_election_set_caw (zyre_election_t *self, char *caw)
{
    assert (self);
    self->caw = caw;
}

zyre_peer_t *
zyre_election_father (zyre_election_t *self)
{
    assert (self);
    return self->father;
}

void
zyre_election_set_father (zyre_election_t *self, zyre_peer_t *father)
{
    assert (self);
    self->father = father;
}

zre_msg_t *
zyre_election_build_elect_msg (zyre_election_t *self)
{
    assert(self);
    zre_msg_t *election_msg = zre_msg_new();
    zre_msg_set_id(election_msg, ZRE_MSG_ELECT);
    zre_msg_set_challenger_id (election_msg, self->caw);
    return election_msg;
}

zre_msg_t *
zyre_election_build_leader_msg (zyre_election_t *self)
{
    assert(self);
    assert(self->caw);
    zre_msg_t *election_msg = zre_msg_new ();
    zre_msg_set_id (election_msg, ZRE_MSG_LEADER);
    zre_msg_set_leader_id (election_msg, self->caw);
    return election_msg;
}

bool
zyre_election_supporting_challenger (zyre_election_t *self, const char *r)
{
    assert(self);
    assert(self->caw);
    assert(r);
    return strcmp (self->caw, r) == 0;
}

void
zyre_election_increment_erec (zyre_election_t *self)
{
    assert (self);
    self->erec++;
}

void
zyre_election_increment_lrec (zyre_election_t *self)
{
    assert (self);
    self->lrec++;
}

bool
zyre_election_erec_complete (zyre_election_t *self, zyre_group_t *group)
{
    assert(self);
    zlist_t *neighbors = zyre_group_peers (group);
    bool complete = self->erec == zlist_size (neighbors);
    zlist_destroy (&neighbors);
    return complete;
}

bool
zyre_election_lrec_complete (zyre_election_t *self, zyre_group_t *group)
{
    assert(self);
    zlist_t *neighbors = zyre_group_peers (group);
    bool complete = self->lrec == zlist_size (neighbors);
    zlist_destroy(&neighbors);
    return complete;
}

bool
zyre_election_lrec_started (zyre_election_t *self)
{
    assert (self);
    return self->lrec > 0;
}

//  Returns the leader if an election is finished, otherwise NULL.

const char *
zyre_election_leader (zyre_election_t *self)
{
    assert (self);
    return self->leader;
}

//  Sets the leader if an election is finished, otherwise NULL.

void
zyre_election_set_leader (zyre_election_t *self, char *leader)
{
    assert (self);
    zstr_free (&self->leader);
    self->leader = leader;
}

//  Returns true if an election is finished, otherwise false.

bool
zyre_election_finished (zyre_election_t *self)
{
    assert (self);
    return !self->caw && self->leader;
}

//  Returns true if an election is won, otherwise false.

bool
zyre_election_won (zyre_election_t *self)
{
    assert (self);
    return self->leader? self->state: false;
}

//  Print election status to command line

void
zyre_election_print (zyre_election_t *self) {
    printf ("zyre_election : {\n");
    printf ("    father: %s\n", zyre_peer_name (self->father));
    printf ("    CAW: %s\n", self->caw);
    printf ("    election count: %d\n", self->erec);
    printf ("    leader count: %d\n", self->lrec);
    printf ("    state: %s\n", !self->leader? "undecided": self->state? "leader": "looser");
    printf ("    leader: %s\n", self->leader);
    printf ("}\n");
}