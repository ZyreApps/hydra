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

    HELLO - Open new connection
        identity            string      Client identity
        nickname            string      Client nickname

    HELLO_OK - Accept new connection and return most recent post, if any.
        post_id             string      Post identifier
        identity            string      Server identity
        nickname            string      Server nickname

    GET_POST - Fetch a given post's content
        post_id             string      Post identifier

    GET_POST_OK - Return a post's metadata and content
        post_id             string      Post identifier
        reply_to            string      Parent post, if any
        previous            string      Previous post, if any
        tags                string      Content tags, space delimited
        timestamp           string      Content date/time
        digest              string      Content digest
        type                string      Content type
        content             chunk       Content body

    GET_TAGS - Request list of all tags known by peer

    GET_TAGS_OK - Return list of known tags
        tags                string      Known tags, space delimited

    GET_TAG - Request summary for a given tag
        tag                 string      Name of tag

    GET_TAG_OK - Return latest post ID for given tag
        tag                 string      Name of tag
        post_id             string      Post identifier

    GOODBYE - Close the connection politely

    GOODBYE_OK - Handshake a connection close

    ERROR - Command failed for some specific reason
        status              number 2    3-digit status code
        reason              string      Printable explanation
*/

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
#define HYDRA_PROTO_GET_POST                3
#define HYDRA_PROTO_GET_POST_OK             4
#define HYDRA_PROTO_GET_TAGS                5
#define HYDRA_PROTO_GET_TAGS_OK             6
#define HYDRA_PROTO_GET_TAG                 7
#define HYDRA_PROTO_GET_TAG_OK              8
#define HYDRA_PROTO_GOODBYE                 9
#define HYDRA_PROTO_GOODBYE_OK              10
#define HYDRA_PROTO_ERROR                   11

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

//  Get/set the post_id field
const char *
    hydra_proto_post_id (hydra_proto_t *self);
void
    hydra_proto_set_post_id (hydra_proto_t *self, const char *value);

//  Get/set the reply_to field
const char *
    hydra_proto_reply_to (hydra_proto_t *self);
void
    hydra_proto_set_reply_to (hydra_proto_t *self, const char *value);

//  Get/set the previous field
const char *
    hydra_proto_previous (hydra_proto_t *self);
void
    hydra_proto_set_previous (hydra_proto_t *self, const char *value);

//  Get/set the tags field
const char *
    hydra_proto_tags (hydra_proto_t *self);
void
    hydra_proto_set_tags (hydra_proto_t *self, const char *value);

//  Get/set the timestamp field
const char *
    hydra_proto_timestamp (hydra_proto_t *self);
void
    hydra_proto_set_timestamp (hydra_proto_t *self, const char *value);

//  Get/set the digest field
const char *
    hydra_proto_digest (hydra_proto_t *self);
void
    hydra_proto_set_digest (hydra_proto_t *self, const char *value);

//  Get/set the type field
const char *
    hydra_proto_type (hydra_proto_t *self);
void
    hydra_proto_set_type (hydra_proto_t *self, const char *value);

//  Get a copy of the content field
zchunk_t *
    hydra_proto_content (hydra_proto_t *self);
//  Get the content field and transfer ownership to caller
zchunk_t *
    hydra_proto_get_content (hydra_proto_t *self);
//  Set the content field, transferring ownership from caller
void
    hydra_proto_set_content (hydra_proto_t *self, zchunk_t **chunk_p);

//  Get/set the tag field
const char *
    hydra_proto_tag (hydra_proto_t *self);
void
    hydra_proto_set_tag (hydra_proto_t *self, const char *value);

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
