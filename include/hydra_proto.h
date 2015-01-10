/*  =========================================================================
    hydra_proto - The Hydra Protocol
    
    Codec header for hydra_proto.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: hydra_proto.xml, or
     * The code generation script that built this file: zproto_codec_c
    ************************************************************************
    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of zbroker, the ZeroMQ broker project.           
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

#ifndef __HYDRA_PROTO_H_INCLUDED__
#define __HYDRA_PROTO_H_INCLUDED__

/*  These are the hydra_proto messages:

    HELLO - Open new connection, provide client credentials.
        identity            string      Client identity
        nickname            string      Client nickname

    HELLO_OK - Accept new connection, provide server credentials.
        identity            string      Server identity
        nickname            string      Server nickname

    STATUS - Client requests server status update, telling server the oldest and
newest post that it knows for that server. If the client never
received any posts from the server, these fields are empty.
        oldest              string      Oldest post
        newest              string      Newest post

    STATUS_OK - Server tells client how many posts it has, older and newer than the
range the client already knows.
        before              number 4    Number of posts before oldest
        after               number 4    Newest of posts after newest

    HEADER - Client requests a post from the server, requesting either an older post
(previous to the oldest post it already has), a newer post (following the
newest post it has), or a fresh post (server's latest post, ignoring all
status).
        which               number 1    Which post to fetch

    HEADER_OK - Return a post's metadata.
        ident               string      Post identifier
        subject             longstr     Subject line
        timestamp           string      Post creation timestamp
        parent_id           string      Parent post ID, if any
        digest              string      Content SHA1 digest
        mime_type           string      Content MIME type
        content_size        number 8    Content size, octets

    HEADER_EMPTY - Server does not have a post to give to the client.

    CHUNK - Client fetches a chunk of content data from the server. This command
always applies to the post returned by a HEADER-OK. The
        offset              number 8    Chunk offset in file
        octets              number 4    Maximum chunk size to fetch

    CHUNK_OK - Return a chunk of post content.
        offset              number 8    Chunk offset in file
        content             chunk       Content data chunk

    GOODBYE - Close the connection politely

    GOODBYE_OK - Handshake a connection close

    ERROR - Command failed for some specific reason
        status              number 2    3-digit status code
        reason              string      Printable explanation
*/

#define HYDRA_PROTO_FETCH_OLDER             1
#define HYDRA_PROTO_FETCH_NEWER             2
#define HYDRA_PROTO_FETCH_RESET             3
#define HYDRA_PROTO_SUCCESS                 200
#define HYDRA_PROTO_STORED                  201
#define HYDRA_PROTO_DELIVERED               202
#define HYDRA_PROTO_NOT_DELIVERED           300
#define HYDRA_PROTO_CONTENT_TOO_LARGE       301
#define HYDRA_PROTO_TIMEOUT_EXPIRED         302
#define HYDRA_PROTO_CONNECTION_REFUSED      303
#define HYDRA_PROTO_RESOURCE_LOCKED         400
#define HYDRA_PROTO_ACCESS_REFUSED          401
#define HYDRA_PROTO_NOT_FOUND               404
#define HYDRA_PROTO_COMMAND_INVALID         500
#define HYDRA_PROTO_NOT_IMPLEMENTED         501
#define HYDRA_PROTO_INTERNAL_ERROR          502

#define HYDRA_PROTO_HELLO                   1
#define HYDRA_PROTO_HELLO_OK                2
#define HYDRA_PROTO_STATUS                  3
#define HYDRA_PROTO_STATUS_OK               4
#define HYDRA_PROTO_HEADER                  5
#define HYDRA_PROTO_HEADER_OK               6
#define HYDRA_PROTO_HEADER_EMPTY            7
#define HYDRA_PROTO_CHUNK                   8
#define HYDRA_PROTO_CHUNK_OK                9
#define HYDRA_PROTO_GOODBYE                 10
#define HYDRA_PROTO_GOODBYE_OK              11
#define HYDRA_PROTO_ERROR                   12

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
#ifndef HYDRA_PROTO_T_DEFINED
typedef struct _hydra_proto_t hydra_proto_t;
#define HYDRA_PROTO_T_DEFINED
#endif

//  @interface
//  Create a new empty hydra_proto
hydra_proto_t *
    hydra_proto_new (void);

//  Destroy a hydra_proto instance
void
    hydra_proto_destroy (hydra_proto_t **self_p);

//  Receive a hydra_proto from the socket. Returns 0 if OK, -1 if
//  there was an error. Blocks if there is no message waiting.
int
    hydra_proto_recv (hydra_proto_t *self, zsock_t *input);

//  Send the hydra_proto to the output socket, does not destroy it
int
    hydra_proto_send (hydra_proto_t *self, zsock_t *output);
    
//  Print contents of message to stdout
void
    hydra_proto_print (hydra_proto_t *self);

//  Get/set the message routing id
zframe_t *
    hydra_proto_routing_id (hydra_proto_t *self);
void
    hydra_proto_set_routing_id (hydra_proto_t *self, zframe_t *routing_id);

//  Get the hydra_proto id and printable command
int
    hydra_proto_id (hydra_proto_t *self);
void
    hydra_proto_set_id (hydra_proto_t *self, int id);
const char *
    hydra_proto_command (hydra_proto_t *self);

//  Get/set the identity field
const char *
    hydra_proto_identity (hydra_proto_t *self);
void
    hydra_proto_set_identity (hydra_proto_t *self, const char *value);

//  Get/set the nickname field
const char *
    hydra_proto_nickname (hydra_proto_t *self);
void
    hydra_proto_set_nickname (hydra_proto_t *self, const char *value);

//  Get/set the oldest field
const char *
    hydra_proto_oldest (hydra_proto_t *self);
void
    hydra_proto_set_oldest (hydra_proto_t *self, const char *value);

//  Get/set the newest field
const char *
    hydra_proto_newest (hydra_proto_t *self);
void
    hydra_proto_set_newest (hydra_proto_t *self, const char *value);

//  Get/set the before field
uint32_t
    hydra_proto_before (hydra_proto_t *self);
void
    hydra_proto_set_before (hydra_proto_t *self, uint32_t before);

//  Get/set the after field
uint32_t
    hydra_proto_after (hydra_proto_t *self);
void
    hydra_proto_set_after (hydra_proto_t *self, uint32_t after);

//  Get/set the which field
byte
    hydra_proto_which (hydra_proto_t *self);
void
    hydra_proto_set_which (hydra_proto_t *self, byte which);

//  Get/set the ident field
const char *
    hydra_proto_ident (hydra_proto_t *self);
void
    hydra_proto_set_ident (hydra_proto_t *self, const char *value);

//  Get/set the subject field
const char *
    hydra_proto_subject (hydra_proto_t *self);
void
    hydra_proto_set_subject (hydra_proto_t *self, const char *value);

//  Get/set the timestamp field
const char *
    hydra_proto_timestamp (hydra_proto_t *self);
void
    hydra_proto_set_timestamp (hydra_proto_t *self, const char *value);

//  Get/set the parent_id field
const char *
    hydra_proto_parent_id (hydra_proto_t *self);
void
    hydra_proto_set_parent_id (hydra_proto_t *self, const char *value);

//  Get/set the digest field
const char *
    hydra_proto_digest (hydra_proto_t *self);
void
    hydra_proto_set_digest (hydra_proto_t *self, const char *value);

//  Get/set the mime_type field
const char *
    hydra_proto_mime_type (hydra_proto_t *self);
void
    hydra_proto_set_mime_type (hydra_proto_t *self, const char *value);

//  Get/set the content_size field
uint64_t
    hydra_proto_content_size (hydra_proto_t *self);
void
    hydra_proto_set_content_size (hydra_proto_t *self, uint64_t content_size);

//  Get/set the offset field
uint64_t
    hydra_proto_offset (hydra_proto_t *self);
void
    hydra_proto_set_offset (hydra_proto_t *self, uint64_t offset);

//  Get/set the octets field
uint32_t
    hydra_proto_octets (hydra_proto_t *self);
void
    hydra_proto_set_octets (hydra_proto_t *self, uint32_t octets);

//  Get a copy of the content field
zchunk_t *
    hydra_proto_content (hydra_proto_t *self);
//  Get the content field and transfer ownership to caller
zchunk_t *
    hydra_proto_get_content (hydra_proto_t *self);
//  Set the content field, transferring ownership from caller
void
    hydra_proto_set_content (hydra_proto_t *self, zchunk_t **chunk_p);

//  Get/set the status field
uint16_t
    hydra_proto_status (hydra_proto_t *self);
void
    hydra_proto_set_status (hydra_proto_t *self, uint16_t status);

//  Get/set the reason field
const char *
    hydra_proto_reason (hydra_proto_t *self);
void
    hydra_proto_set_reason (hydra_proto_t *self, const char *value);

//  Self test of this class
int
    hydra_proto_test (bool verbose);
//  @end

//  For backwards compatibility with old codecs
#define hydra_proto_dump    hydra_proto_print

#ifdef __cplusplus
}
#endif

#endif
