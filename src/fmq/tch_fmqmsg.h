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
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * These are the fmq_msg messages:

    OHAI - Client opens peering
        protocol            string      Constant "FILEMQ"
        version             number 2    Protocol version 2

    OHAI_OK - Server grants the client access

    ICANHAZ - Client subscribes to a path
        path                longstr     Full path or path prefix
        options             hash        Subscription options
        cache               hash        File SHA-1 signatures

    ICANHAZ_OK - Server confirms the subscription

    NOM - Client sends credit to the server
        credit              number 8    Credit, in bytes
        sequence            number 8    Chunk sequence, 0 and up

    CHEEZBURGER - The server sends a file chunk
        sequence            number 8    File offset in bytes
        operation           number 1    Create=%d1 delete=%d2
        filename            longstr     Relative name of file
        offset              number 8    File offset in bytes
        eof                 number 1    Last chunk in file?
        headers             hash        File properties
        chunk               chunk       Data chunk

    HUGZ - Client sends a heartbeat

    HUGZ_OK - Server answers a heartbeat

    KTHXBAI - Client closes the peering

    SRSLY - Server refuses client due to access rights
        reason              string      Printable explanation, 255 characters

    RTFM - Server tells client it sent an invalid message
        reason              string      Printable explanation, 255 characters
 */
#ifndef __TCH_FMQ_MSG__
#define __TCH_FMQ_MSG__

#include <czmq.h>

#define FMQ_MSG_VERSION                     2
#define FMQ_MSG_FILE_CREATE                 1
#define FMQ_MSG_FILE_DELETE                 2

#define FMQ_MSG_OHAI                        1
#define FMQ_MSG_OHAI_OK                     4
#define FMQ_MSG_ICANHAZ                     5
#define FMQ_MSG_ICANHAZ_OK                  6
#define FMQ_MSG_NOM                         7
#define FMQ_MSG_CHEEZBURGER                 8
#define FMQ_MSG_HUGZ                        9
#define FMQ_MSG_HUGZ_OK                     10
#define FMQ_MSG_KTHXBAI                     11
#define FMQ_MSG_SRSLY                       128
#define FMQ_MSG_RTFM                        129

typedef struct _fmq_msg_t fmq_msg_t;

/* Create a new empty fmq_msg */
fmq_msg_t *fmq_msg_new(void);

/* Destroy a fmq_msg instance */
void fmq_msg_destroy(fmq_msg_t **self_p);

/* Receive a fmq_msg from the socket. Returns 0 if OK, -1 if
 * there was an error. Blocks if there is no message waiting. */
int fmq_msg_recv(fmq_msg_t *self, zsock_t *input);

/* Send the fmq_msg to the output socket, does not destroy it */
int fmq_msg_send(fmq_msg_t *self, zsock_t *output);

/* Print contents of message to stdout */
void fmq_msg_print(fmq_msg_t *self);

/* Get/set the message routing id */
zframe_t *fmq_msg_routing_id(fmq_msg_t *self);
void fmq_msg_set_routing_id(fmq_msg_t *self, zframe_t *routing_id);

/* Get the fmq_msg id and printable command */
int fmq_msg_id(fmq_msg_t *self);
void fmq_msg_set_id(fmq_msg_t *self, int id);
const char *fmq_msg_command(fmq_msg_t *self);

/* Get/set the path field */
const char *fmq_msg_path (fmq_msg_t *self);
void fmq_msg_set_path (fmq_msg_t *self, const char *value);

/* Get a copy of the options field */
zhash_t *fmq_msg_options (fmq_msg_t *self);
/* Get the options field and transfer ownership to caller */
zhash_t *fmq_msg_get_options (fmq_msg_t *self);
/* Set the options field, transferring ownership from caller */
void fmq_msg_set_options (fmq_msg_t *self, zhash_t **hash_p);

/* Get a copy of the cache field */
zhash_t *fmq_msg_cache (fmq_msg_t *self);
/* Get the cache field and transfer ownership to caller */
zhash_t *fmq_msg_get_cache (fmq_msg_t *self);
/* Set the cache field, transferring ownership from caller */
void fmq_msg_set_cache (fmq_msg_t *self, zhash_t **hash_p);

/* Get/set the credit field */
uint64_t fmq_msg_credit (fmq_msg_t *self);
void fmq_msg_set_credit (fmq_msg_t *self, uint64_t credit);

/* Get/set the sequence field */
uint64_t fmq_msg_sequence (fmq_msg_t *self);
void fmq_msg_set_sequence (fmq_msg_t *self, uint64_t sequence);

/* Get/set the operation field */
byte fmq_msg_operation (fmq_msg_t *self);
void fmq_msg_set_operation (fmq_msg_t *self, byte operation);

/* Get/set the filename field */
const char *fmq_msg_filename (fmq_msg_t *self);
void fmq_msg_set_filename (fmq_msg_t *self, const char *value);

/* Get/set the offset field */
uint64_t fmq_msg_offset (fmq_msg_t *self);
void fmq_msg_set_offset (fmq_msg_t *self, uint64_t offset);

/* Get/set the eof field */
byte fmq_msg_eof (fmq_msg_t *self);
void fmq_msg_set_eof (fmq_msg_t *self, byte eof);

/* Get a copy of the headers field */
zhash_t *fmq_msg_headers (fmq_msg_t *self);
/* Get the headers field and transfer ownership to caller*/
zhash_t *fmq_msg_get_headers (fmq_msg_t *self);
/* Set the headers field, transferring ownership from caller */
void fmq_msg_set_headers (fmq_msg_t *self, zhash_t **hash_p);

/* Get a copy of the chunk field */
zchunk_t *fmq_msg_chunk (fmq_msg_t *self);
/* Get the chunk field and transfer ownership to caller */
zchunk_t *fmq_msg_get_chunk (fmq_msg_t *self);
/* Set the chunk field, transferring ownership from caller */
void fmq_msg_set_chunk (fmq_msg_t *self, zchunk_t **chunk_p);

/* Get/set the reason field */
const char *fmq_msg_reason (fmq_msg_t *self);
void fmq_msg_set_reason (fmq_msg_t *self, const char *value);

//  For backwards compatibility with old codecs
#define fmq_msg_dump        fmq_msg_print

#endif