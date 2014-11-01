/*  =========================================================================
    hydra_msg - The Hydra Protocol

    Codec class for hydra_msg.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: hydra_msg.xml, or
     * The code generation script that built this file: zproto_codec_c
    ************************************************************************
    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of zbroker, the ZeroMQ broker project.           
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

/*
@header
    hydra_msg - The Hydra Protocol
@discuss
@end
*/

#include "../include/hydra_msg.h"

//  Structure of our class

struct _hydra_msg_t {
    zframe_t *routing_id;               //  Routing_id from ROUTER, if any
    int id;                             //  hydra_msg message ID
    byte *needle;                       //  Read/write pointer for serialization
    byte *ceiling;                      //  Valid upper limit for read pointer
    char *post_id;                      //  Post identifier
    zlist_t *tags;                      //  List of known tags
    char *tag;                          //  Name of tag
    char *reply_to;                     //  Parent post, if any
    char *previous;                     //  Previous post, if any
    uint64_t timestamp;                 //  Post creation timestamp
    char *type;                         //  Content type
    zmsg_t *content;                    //  Content body
    char *reason;                       //  Reason for failure
};

//  --------------------------------------------------------------------------
//  Network data encoding macros

//  Put a block of octets to the frame
#define PUT_OCTETS(host,size) { \
    memcpy (self->needle, (host), size); \
    self->needle += size; \
}

//  Get a block of octets from the frame
#define GET_OCTETS(host,size) { \
    if (self->needle + size > self->ceiling) \
        goto malformed; \
    memcpy ((host), self->needle, size); \
    self->needle += size; \
}

//  Put a 1-byte number to the frame
#define PUT_NUMBER1(host) { \
    *(byte *) self->needle = (host); \
    self->needle++; \
}

//  Put a 2-byte number to the frame
#define PUT_NUMBER2(host) { \
    self->needle [0] = (byte) (((host) >> 8)  & 255); \
    self->needle [1] = (byte) (((host))       & 255); \
    self->needle += 2; \
}

//  Put a 4-byte number to the frame
#define PUT_NUMBER4(host) { \
    self->needle [0] = (byte) (((host) >> 24) & 255); \
    self->needle [1] = (byte) (((host) >> 16) & 255); \
    self->needle [2] = (byte) (((host) >> 8)  & 255); \
    self->needle [3] = (byte) (((host))       & 255); \
    self->needle += 4; \
}

//  Put a 8-byte number to the frame
#define PUT_NUMBER8(host) { \
    self->needle [0] = (byte) (((host) >> 56) & 255); \
    self->needle [1] = (byte) (((host) >> 48) & 255); \
    self->needle [2] = (byte) (((host) >> 40) & 255); \
    self->needle [3] = (byte) (((host) >> 32) & 255); \
    self->needle [4] = (byte) (((host) >> 24) & 255); \
    self->needle [5] = (byte) (((host) >> 16) & 255); \
    self->needle [6] = (byte) (((host) >> 8)  & 255); \
    self->needle [7] = (byte) (((host))       & 255); \
    self->needle += 8; \
}

//  Get a 1-byte number from the frame
#define GET_NUMBER1(host) { \
    if (self->needle + 1 > self->ceiling) \
        goto malformed; \
    (host) = *(byte *) self->needle; \
    self->needle++; \
}

//  Get a 2-byte number from the frame
#define GET_NUMBER2(host) { \
    if (self->needle + 2 > self->ceiling) \
        goto malformed; \
    (host) = ((uint16_t) (self->needle [0]) << 8) \
           +  (uint16_t) (self->needle [1]); \
    self->needle += 2; \
}

//  Get a 4-byte number from the frame
#define GET_NUMBER4(host) { \
    if (self->needle + 4 > self->ceiling) \
        goto malformed; \
    (host) = ((uint32_t) (self->needle [0]) << 24) \
           + ((uint32_t) (self->needle [1]) << 16) \
           + ((uint32_t) (self->needle [2]) << 8) \
           +  (uint32_t) (self->needle [3]); \
    self->needle += 4; \
}

//  Get a 8-byte number from the frame
#define GET_NUMBER8(host) { \
    if (self->needle + 8 > self->ceiling) \
        goto malformed; \
    (host) = ((uint64_t) (self->needle [0]) << 56) \
           + ((uint64_t) (self->needle [1]) << 48) \
           + ((uint64_t) (self->needle [2]) << 40) \
           + ((uint64_t) (self->needle [3]) << 32) \
           + ((uint64_t) (self->needle [4]) << 24) \
           + ((uint64_t) (self->needle [5]) << 16) \
           + ((uint64_t) (self->needle [6]) << 8) \
           +  (uint64_t) (self->needle [7]); \
    self->needle += 8; \
}

//  Put a string to the frame
#define PUT_STRING(host) { \
    size_t string_size = strlen (host); \
    PUT_NUMBER1 (string_size); \
    memcpy (self->needle, (host), string_size); \
    self->needle += string_size; \
}

//  Get a string from the frame
#define GET_STRING(host) { \
    size_t string_size; \
    GET_NUMBER1 (string_size); \
    if (self->needle + string_size > (self->ceiling)) \
        goto malformed; \
    (host) = (char *) malloc (string_size + 1); \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
}

//  Put a long string to the frame
#define PUT_LONGSTR(host) { \
    size_t string_size = strlen (host); \
    PUT_NUMBER4 (string_size); \
    memcpy (self->needle, (host), string_size); \
    self->needle += string_size; \
}

//  Get a long string from the frame
#define GET_LONGSTR(host) { \
    size_t string_size; \
    GET_NUMBER4 (string_size); \
    if (self->needle + string_size > (self->ceiling)) \
        goto malformed; \
    (host) = (char *) malloc (string_size + 1); \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
}


//  --------------------------------------------------------------------------
//  Create a new hydra_msg

hydra_msg_t *
hydra_msg_new (int id)
{
    hydra_msg_t *self = (hydra_msg_t *) zmalloc (sizeof (hydra_msg_t));
    self->id = id;
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the hydra_msg

void
hydra_msg_destroy (hydra_msg_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        hydra_msg_t *self = *self_p;

        //  Free class properties
        zframe_destroy (&self->routing_id);
        free (self->post_id);
        if (self->tags)
            zlist_destroy (&self->tags);
        free (self->tag);
        free (self->reply_to);
        free (self->previous);
        free (self->type);
        zmsg_destroy (&self->content);
        free (self->reason);

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Parse a hydra_msg from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and 
//  nullifies the msg reference.

hydra_msg_t *
hydra_msg_decode (zmsg_t **msg_p)
{
    assert (msg_p);
    zmsg_t *msg = *msg_p;
    if (msg == NULL)
        return NULL;
        
    hydra_msg_t *self = hydra_msg_new (0);
    //  Read and parse command in frame
    zframe_t *frame = zmsg_pop (msg);
    if (!frame) 
        goto empty;             //  Malformed or empty

    //  Get and check protocol signature
    self->needle = zframe_data (frame);
    self->ceiling = self->needle + zframe_size (frame);
    uint16_t signature;
    GET_NUMBER2 (signature);
    if (signature != (0xAAA0 | 0))
        goto empty;             //  Invalid signature

    //  Get message id and parse per message type
    GET_NUMBER1 (self->id);

    switch (self->id) {
        case HYDRA_MSG_HELLO:
            break;

        case HYDRA_MSG_HELLO_OK:
            GET_STRING (self->post_id);
            break;

        case HYDRA_MSG_GET_TAGS:
            break;

        case HYDRA_MSG_GET_TAGS_OK:
            {
                size_t list_size;
                GET_NUMBER4 (list_size);
                self->tags = zlist_new ();
                zlist_autofree (self->tags);
                while (list_size--) {
                    char *string;
                    GET_LONGSTR (string);
                    zlist_append (self->tags, string);
                    free (string);
                }
            }
            break;

        case HYDRA_MSG_GET_TAG:
            GET_STRING (self->tag);
            break;

        case HYDRA_MSG_GET_TAG_OK:
            GET_STRING (self->post_id);
            break;

        case HYDRA_MSG_GET_POST:
            GET_STRING (self->post_id);
            break;

        case HYDRA_MSG_GET_POST_OK:
            GET_STRING (self->post_id);
            GET_STRING (self->reply_to);
            GET_STRING (self->previous);
            {
                size_t list_size;
                GET_NUMBER4 (list_size);
                self->tags = zlist_new ();
                zlist_autofree (self->tags);
                while (list_size--) {
                    char *string;
                    GET_LONGSTR (string);
                    zlist_append (self->tags, string);
                    free (string);
                }
            }
            GET_NUMBER8 (self->timestamp);
            GET_STRING (self->type);
            //  Get zero or more remaining frames, leaving current
            //  frame untouched
            self->content = zmsg_new ();
            while (zmsg_size (msg))
                zmsg_add (self->content, zmsg_pop (msg));
            break;

        case HYDRA_MSG_GOODBYE:
            break;

        case HYDRA_MSG_GOODBYE_OK:
            break;

        case HYDRA_MSG_INVALID:
            break;

        case HYDRA_MSG_FAILED:
            GET_STRING (self->reason);
            break;

        default:
            goto malformed;
    }
    //  Successful return
    zframe_destroy (&frame);
    zmsg_destroy (msg_p);
    return self;

    //  Error returns
    malformed:
        zsys_error ("malformed message '%d'\n", self->id);
    empty:
        zframe_destroy (&frame);
        zmsg_destroy (msg_p);
        hydra_msg_destroy (&self);
        return (NULL);
}


//  --------------------------------------------------------------------------
//  Encode hydra_msg into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.

zmsg_t *
hydra_msg_encode (hydra_msg_t **self_p)
{
    assert (self_p);
    assert (*self_p);
    
    hydra_msg_t *self = *self_p;
    zmsg_t *msg = zmsg_new ();

    size_t frame_size = 2 + 1;          //  Signature and message ID
    switch (self->id) {
        case HYDRA_MSG_HELLO:
            break;
            
        case HYDRA_MSG_HELLO_OK:
            //  post_id is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->post_id)
                frame_size += strlen (self->post_id);
            break;
            
        case HYDRA_MSG_GET_TAGS:
            break;
            
        case HYDRA_MSG_GET_TAGS_OK:
            //  tags is an array of strings
            frame_size += 4;    //  Size is 4 octets
            if (self->tags) {
                //  Add up size of list contents
                char *tags = (char *) zlist_first (self->tags);
                while (tags) {
                    frame_size += 4 + strlen (tags);
                    tags = (char *) zlist_next (self->tags);
                }
            }
            break;
            
        case HYDRA_MSG_GET_TAG:
            //  tag is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->tag)
                frame_size += strlen (self->tag);
            break;
            
        case HYDRA_MSG_GET_TAG_OK:
            //  post_id is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->post_id)
                frame_size += strlen (self->post_id);
            break;
            
        case HYDRA_MSG_GET_POST:
            //  post_id is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->post_id)
                frame_size += strlen (self->post_id);
            break;
            
        case HYDRA_MSG_GET_POST_OK:
            //  post_id is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->post_id)
                frame_size += strlen (self->post_id);
            //  reply_to is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->reply_to)
                frame_size += strlen (self->reply_to);
            //  previous is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->previous)
                frame_size += strlen (self->previous);
            //  tags is an array of strings
            frame_size += 4;    //  Size is 4 octets
            if (self->tags) {
                //  Add up size of list contents
                char *tags = (char *) zlist_first (self->tags);
                while (tags) {
                    frame_size += 4 + strlen (tags);
                    tags = (char *) zlist_next (self->tags);
                }
            }
            //  timestamp is a 8-byte integer
            frame_size += 8;
            //  type is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->type)
                frame_size += strlen (self->type);
            break;
            
        case HYDRA_MSG_GOODBYE:
            break;
            
        case HYDRA_MSG_GOODBYE_OK:
            break;
            
        case HYDRA_MSG_INVALID:
            break;
            
        case HYDRA_MSG_FAILED:
            //  reason is a string with 1-byte length
            frame_size++;       //  Size is one octet
            if (self->reason)
                frame_size += strlen (self->reason);
            break;
            
        default:
            zsys_error ("bad message type '%d', not sent\n", self->id);
            //  No recovery, this is a fatal application error
            assert (false);
    }
    //  Now serialize message into the frame
    zframe_t *frame = zframe_new (NULL, frame_size);
    self->needle = zframe_data (frame);
    PUT_NUMBER2 (0xAAA0 | 0);
    PUT_NUMBER1 (self->id);

    switch (self->id) {
        case HYDRA_MSG_HELLO:
            break;

        case HYDRA_MSG_HELLO_OK:
            if (self->post_id) {
                PUT_STRING (self->post_id);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case HYDRA_MSG_GET_TAGS:
            break;

        case HYDRA_MSG_GET_TAGS_OK:
            if (self->tags) {
                PUT_NUMBER4 (zlist_size (self->tags));
                char *tags = (char *) zlist_first (self->tags);
                while (tags) {
                    PUT_LONGSTR (tags);
                    tags = (char *) zlist_next (self->tags);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty string array
            break;

        case HYDRA_MSG_GET_TAG:
            if (self->tag) {
                PUT_STRING (self->tag);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case HYDRA_MSG_GET_TAG_OK:
            if (self->post_id) {
                PUT_STRING (self->post_id);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case HYDRA_MSG_GET_POST:
            if (self->post_id) {
                PUT_STRING (self->post_id);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case HYDRA_MSG_GET_POST_OK:
            if (self->post_id) {
                PUT_STRING (self->post_id);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->reply_to) {
                PUT_STRING (self->reply_to);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->previous) {
                PUT_STRING (self->previous);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            if (self->tags) {
                PUT_NUMBER4 (zlist_size (self->tags));
                char *tags = (char *) zlist_first (self->tags);
                while (tags) {
                    PUT_LONGSTR (tags);
                    tags = (char *) zlist_next (self->tags);
                }
            }
            else
                PUT_NUMBER4 (0);    //  Empty string array
            PUT_NUMBER8 (self->timestamp);
            if (self->type) {
                PUT_STRING (self->type);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

        case HYDRA_MSG_GOODBYE:
            break;

        case HYDRA_MSG_GOODBYE_OK:
            break;

        case HYDRA_MSG_INVALID:
            break;

        case HYDRA_MSG_FAILED:
            if (self->reason) {
                PUT_STRING (self->reason);
            }
            else
                PUT_NUMBER1 (0);    //  Empty string
            break;

    }
    //  Now send the data frame
    if (zmsg_append (msg, &frame)) {
        zmsg_destroy (&msg);
        hydra_msg_destroy (self_p);
        return NULL;
    }
    //  Now send the message field if there is any
    if (self->id == HYDRA_MSG_GET_POST_OK) {
        if (self->content) {
            zframe_t *frame = zmsg_pop (self->content);
            while (frame) {
                zmsg_append (msg, &frame);
                frame = zmsg_pop (self->content);
            }
        }
        else {
            zframe_t *frame = zframe_new (NULL, 0);
            zmsg_append (msg, &frame);
        }
    }
    //  Destroy hydra_msg object
    hydra_msg_destroy (self_p);
    return msg;
}


//  --------------------------------------------------------------------------
//  Receive and parse a hydra_msg from the socket. Returns new object or
//  NULL if error. Will block if there's no message waiting.

hydra_msg_t *
hydra_msg_recv (void *input)
{
    assert (input);
    zmsg_t *msg = zmsg_recv (input);
    if (!msg)
        return NULL;            //  Interrupted
    //  If message came from a router socket, first frame is routing_id
    zframe_t *routing_id = NULL;
    if (zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER) {
        routing_id = zmsg_pop (msg);
        //  If message was not valid, forget about it
        if (!routing_id || !zmsg_next (msg))
            return NULL;        //  Malformed or empty
    }
    hydra_msg_t *hydra_msg = hydra_msg_decode (&msg);
    if (hydra_msg && zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER)
        hydra_msg->routing_id = routing_id;

    return hydra_msg;
}


//  --------------------------------------------------------------------------
//  Receive and parse a hydra_msg from the socket. Returns new object,
//  or NULL either if there was no input waiting, or the recv was interrupted.

hydra_msg_t *
hydra_msg_recv_nowait (void *input)
{
    assert (input);
    zmsg_t *msg = zmsg_recv_nowait (input);
    if (!msg)
        return NULL;            //  Interrupted
    //  If message came from a router socket, first frame is routing_id
    zframe_t *routing_id = NULL;
    if (zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER) {
        routing_id = zmsg_pop (msg);
        //  If message was not valid, forget about it
        if (!routing_id || !zmsg_next (msg))
            return NULL;        //  Malformed or empty
    }
    hydra_msg_t *hydra_msg = hydra_msg_decode (&msg);
    if (hydra_msg && zsocket_type (zsock_resolve (input)) == ZMQ_ROUTER)
        hydra_msg->routing_id = routing_id;

    return hydra_msg;
}


//  --------------------------------------------------------------------------
//  Send the hydra_msg to the socket, and destroy it
//  Returns 0 if OK, else -1

int
hydra_msg_send (hydra_msg_t **self_p, void *output)
{
    assert (self_p);
    assert (*self_p);
    assert (output);

    //  Save routing_id if any, as encode will destroy it
    hydra_msg_t *self = *self_p;
    zframe_t *routing_id = self->routing_id;
    self->routing_id = NULL;

    //  Encode hydra_msg message to a single zmsg
    zmsg_t *msg = hydra_msg_encode (self_p);
    
    //  If we're sending to a ROUTER, send the routing_id first
    if (zsocket_type (zsock_resolve (output)) == ZMQ_ROUTER) {
        assert (routing_id);
        zmsg_prepend (msg, &routing_id);
    }
    else
        zframe_destroy (&routing_id);
        
    if (msg && zmsg_send (&msg, output) == 0)
        return 0;
    else
        return -1;              //  Failed to encode, or send
}


//  --------------------------------------------------------------------------
//  Send the hydra_msg to the output, and do not destroy it

int
hydra_msg_send_again (hydra_msg_t *self, void *output)
{
    assert (self);
    assert (output);
    self = hydra_msg_dup (self);
    return hydra_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Encode HELLO message

zmsg_t * 
hydra_msg_encode_hello (
)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_HELLO);
    return hydra_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode HELLO_OK message

zmsg_t * 
hydra_msg_encode_hello_ok (
    const char *post_id)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_HELLO_OK);
    hydra_msg_set_post_id (self, post_id);
    return hydra_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GET_TAGS message

zmsg_t * 
hydra_msg_encode_get_tags (
)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_GET_TAGS);
    return hydra_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GET_TAGS_OK message

zmsg_t * 
hydra_msg_encode_get_tags_ok (
    zlist_t *tags)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_GET_TAGS_OK);
    zlist_t *tags_copy = zlist_dup (tags);
    hydra_msg_set_tags (self, &tags_copy);
    return hydra_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GET_TAG message

zmsg_t * 
hydra_msg_encode_get_tag (
    const char *tag)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_GET_TAG);
    hydra_msg_set_tag (self, tag);
    return hydra_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GET_TAG_OK message

zmsg_t * 
hydra_msg_encode_get_tag_ok (
    const char *post_id)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_GET_TAG_OK);
    hydra_msg_set_post_id (self, post_id);
    return hydra_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GET_POST message

zmsg_t * 
hydra_msg_encode_get_post (
    const char *post_id)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_GET_POST);
    hydra_msg_set_post_id (self, post_id);
    return hydra_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GET_POST_OK message

zmsg_t * 
hydra_msg_encode_get_post_ok (
    const char *post_id,
    const char *reply_to,
    const char *previous,
    zlist_t *tags,
    uint64_t timestamp,
    const char *type,
    zmsg_t *content)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_GET_POST_OK);
    hydra_msg_set_post_id (self, post_id);
    hydra_msg_set_reply_to (self, reply_to);
    hydra_msg_set_previous (self, previous);
    zlist_t *tags_copy = zlist_dup (tags);
    hydra_msg_set_tags (self, &tags_copy);
    hydra_msg_set_timestamp (self, timestamp);
    hydra_msg_set_type (self, type);
    zmsg_t *content_copy = zmsg_dup (content);
    hydra_msg_set_content (self, &content_copy);
    return hydra_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GOODBYE message

zmsg_t * 
hydra_msg_encode_goodbye (
)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_GOODBYE);
    return hydra_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode GOODBYE_OK message

zmsg_t * 
hydra_msg_encode_goodbye_ok (
)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_GOODBYE_OK);
    return hydra_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode INVALID message

zmsg_t * 
hydra_msg_encode_invalid (
)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_INVALID);
    return hydra_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Encode FAILED message

zmsg_t * 
hydra_msg_encode_failed (
    const char *reason)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_FAILED);
    hydra_msg_set_reason (self, reason);
    return hydra_msg_encode (&self);
}


//  --------------------------------------------------------------------------
//  Send the HELLO to the socket in one step

int
hydra_msg_send_hello (
    void *output)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_HELLO);
    return hydra_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the HELLO_OK to the socket in one step

int
hydra_msg_send_hello_ok (
    void *output,
    const char *post_id)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_HELLO_OK);
    hydra_msg_set_post_id (self, post_id);
    return hydra_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GET_TAGS to the socket in one step

int
hydra_msg_send_get_tags (
    void *output)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_GET_TAGS);
    return hydra_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GET_TAGS_OK to the socket in one step

int
hydra_msg_send_get_tags_ok (
    void *output,
    zlist_t *tags)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_GET_TAGS_OK);
    zlist_t *tags_copy = zlist_dup (tags);
    hydra_msg_set_tags (self, &tags_copy);
    return hydra_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GET_TAG to the socket in one step

int
hydra_msg_send_get_tag (
    void *output,
    const char *tag)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_GET_TAG);
    hydra_msg_set_tag (self, tag);
    return hydra_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GET_TAG_OK to the socket in one step

int
hydra_msg_send_get_tag_ok (
    void *output,
    const char *post_id)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_GET_TAG_OK);
    hydra_msg_set_post_id (self, post_id);
    return hydra_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GET_POST to the socket in one step

int
hydra_msg_send_get_post (
    void *output,
    const char *post_id)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_GET_POST);
    hydra_msg_set_post_id (self, post_id);
    return hydra_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GET_POST_OK to the socket in one step

int
hydra_msg_send_get_post_ok (
    void *output,
    const char *post_id,
    const char *reply_to,
    const char *previous,
    zlist_t *tags,
    uint64_t timestamp,
    const char *type,
    zmsg_t *content)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_GET_POST_OK);
    hydra_msg_set_post_id (self, post_id);
    hydra_msg_set_reply_to (self, reply_to);
    hydra_msg_set_previous (self, previous);
    zlist_t *tags_copy = zlist_dup (tags);
    hydra_msg_set_tags (self, &tags_copy);
    hydra_msg_set_timestamp (self, timestamp);
    hydra_msg_set_type (self, type);
    zmsg_t *content_copy = zmsg_dup (content);
    hydra_msg_set_content (self, &content_copy);
    return hydra_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GOODBYE to the socket in one step

int
hydra_msg_send_goodbye (
    void *output)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_GOODBYE);
    return hydra_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the GOODBYE_OK to the socket in one step

int
hydra_msg_send_goodbye_ok (
    void *output)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_GOODBYE_OK);
    return hydra_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the INVALID to the socket in one step

int
hydra_msg_send_invalid (
    void *output)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_INVALID);
    return hydra_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Send the FAILED to the socket in one step

int
hydra_msg_send_failed (
    void *output,
    const char *reason)
{
    hydra_msg_t *self = hydra_msg_new (HYDRA_MSG_FAILED);
    hydra_msg_set_reason (self, reason);
    return hydra_msg_send (&self, output);
}


//  --------------------------------------------------------------------------
//  Duplicate the hydra_msg message

hydra_msg_t *
hydra_msg_dup (hydra_msg_t *self)
{
    if (!self)
        return NULL;
        
    hydra_msg_t *copy = hydra_msg_new (self->id);
    if (self->routing_id)
        copy->routing_id = zframe_dup (self->routing_id);
    switch (self->id) {
        case HYDRA_MSG_HELLO:
            break;

        case HYDRA_MSG_HELLO_OK:
            copy->post_id = self->post_id? strdup (self->post_id): NULL;
            break;

        case HYDRA_MSG_GET_TAGS:
            break;

        case HYDRA_MSG_GET_TAGS_OK:
            copy->tags = self->tags? zlist_dup (self->tags): NULL;
            break;

        case HYDRA_MSG_GET_TAG:
            copy->tag = self->tag? strdup (self->tag): NULL;
            break;

        case HYDRA_MSG_GET_TAG_OK:
            copy->post_id = self->post_id? strdup (self->post_id): NULL;
            break;

        case HYDRA_MSG_GET_POST:
            copy->post_id = self->post_id? strdup (self->post_id): NULL;
            break;

        case HYDRA_MSG_GET_POST_OK:
            copy->post_id = self->post_id? strdup (self->post_id): NULL;
            copy->reply_to = self->reply_to? strdup (self->reply_to): NULL;
            copy->previous = self->previous? strdup (self->previous): NULL;
            copy->tags = self->tags? zlist_dup (self->tags): NULL;
            copy->timestamp = self->timestamp;
            copy->type = self->type? strdup (self->type): NULL;
            copy->content = self->content? zmsg_dup (self->content): NULL;
            break;

        case HYDRA_MSG_GOODBYE:
            break;

        case HYDRA_MSG_GOODBYE_OK:
            break;

        case HYDRA_MSG_INVALID:
            break;

        case HYDRA_MSG_FAILED:
            copy->reason = self->reason? strdup (self->reason): NULL;
            break;

    }
    return copy;
}


//  --------------------------------------------------------------------------
//  Print contents of message to stdout

void
hydra_msg_print (hydra_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case HYDRA_MSG_HELLO:
            zsys_debug ("HYDRA_MSG_HELLO:");
            break;
            
        case HYDRA_MSG_HELLO_OK:
            zsys_debug ("HYDRA_MSG_HELLO_OK:");
            if (self->post_id)
                zsys_debug ("    post_id='%s'", self->post_id);
            else
                zsys_debug ("    post_id=");
            break;
            
        case HYDRA_MSG_GET_TAGS:
            zsys_debug ("HYDRA_MSG_GET_TAGS:");
            break;
            
        case HYDRA_MSG_GET_TAGS_OK:
            zsys_debug ("HYDRA_MSG_GET_TAGS_OK:");
            zsys_debug ("    tags=");
            if (self->tags) {
                char *tags = (char *) zlist_first (self->tags);
                while (tags) {
                    zsys_debug ("        '%s'", tags);
                    tags = (char *) zlist_next (self->tags);
                }
            }
            break;
            
        case HYDRA_MSG_GET_TAG:
            zsys_debug ("HYDRA_MSG_GET_TAG:");
            if (self->tag)
                zsys_debug ("    tag='%s'", self->tag);
            else
                zsys_debug ("    tag=");
            break;
            
        case HYDRA_MSG_GET_TAG_OK:
            zsys_debug ("HYDRA_MSG_GET_TAG_OK:");
            if (self->post_id)
                zsys_debug ("    post_id='%s'", self->post_id);
            else
                zsys_debug ("    post_id=");
            break;
            
        case HYDRA_MSG_GET_POST:
            zsys_debug ("HYDRA_MSG_GET_POST:");
            if (self->post_id)
                zsys_debug ("    post_id='%s'", self->post_id);
            else
                zsys_debug ("    post_id=");
            break;
            
        case HYDRA_MSG_GET_POST_OK:
            zsys_debug ("HYDRA_MSG_GET_POST_OK:");
            if (self->post_id)
                zsys_debug ("    post_id='%s'", self->post_id);
            else
                zsys_debug ("    post_id=");
            if (self->reply_to)
                zsys_debug ("    reply_to='%s'", self->reply_to);
            else
                zsys_debug ("    reply_to=");
            if (self->previous)
                zsys_debug ("    previous='%s'", self->previous);
            else
                zsys_debug ("    previous=");
            zsys_debug ("    tags=");
            if (self->tags) {
                char *tags = (char *) zlist_first (self->tags);
                while (tags) {
                    zsys_debug ("        '%s'", tags);
                    tags = (char *) zlist_next (self->tags);
                }
            }
            zsys_debug ("    timestamp=%ld", (long) self->timestamp);
            if (self->type)
                zsys_debug ("    type='%s'", self->type);
            else
                zsys_debug ("    type=");
            zsys_debug ("    content=");
            if (self->content)
                zmsg_print (self->content);
            else
                zsys_debug ("(NULL)");
            break;
            
        case HYDRA_MSG_GOODBYE:
            zsys_debug ("HYDRA_MSG_GOODBYE:");
            break;
            
        case HYDRA_MSG_GOODBYE_OK:
            zsys_debug ("HYDRA_MSG_GOODBYE_OK:");
            break;
            
        case HYDRA_MSG_INVALID:
            zsys_debug ("HYDRA_MSG_INVALID:");
            break;
            
        case HYDRA_MSG_FAILED:
            zsys_debug ("HYDRA_MSG_FAILED:");
            if (self->reason)
                zsys_debug ("    reason='%s'", self->reason);
            else
                zsys_debug ("    reason=");
            break;
            
    }
}


//  --------------------------------------------------------------------------
//  Get/set the message routing_id

zframe_t *
hydra_msg_routing_id (hydra_msg_t *self)
{
    assert (self);
    return self->routing_id;
}

void
hydra_msg_set_routing_id (hydra_msg_t *self, zframe_t *routing_id)
{
    if (self->routing_id)
        zframe_destroy (&self->routing_id);
    self->routing_id = zframe_dup (routing_id);
}


//  --------------------------------------------------------------------------
//  Get/set the hydra_msg id

int
hydra_msg_id (hydra_msg_t *self)
{
    assert (self);
    return self->id;
}

void
hydra_msg_set_id (hydra_msg_t *self, int id)
{
    self->id = id;
}

//  --------------------------------------------------------------------------
//  Return a printable command string

const char *
hydra_msg_command (hydra_msg_t *self)
{
    assert (self);
    switch (self->id) {
        case HYDRA_MSG_HELLO:
            return ("HELLO");
            break;
        case HYDRA_MSG_HELLO_OK:
            return ("HELLO_OK");
            break;
        case HYDRA_MSG_GET_TAGS:
            return ("GET_TAGS");
            break;
        case HYDRA_MSG_GET_TAGS_OK:
            return ("GET_TAGS_OK");
            break;
        case HYDRA_MSG_GET_TAG:
            return ("GET_TAG");
            break;
        case HYDRA_MSG_GET_TAG_OK:
            return ("GET_TAG_OK");
            break;
        case HYDRA_MSG_GET_POST:
            return ("GET_POST");
            break;
        case HYDRA_MSG_GET_POST_OK:
            return ("GET_POST_OK");
            break;
        case HYDRA_MSG_GOODBYE:
            return ("GOODBYE");
            break;
        case HYDRA_MSG_GOODBYE_OK:
            return ("GOODBYE_OK");
            break;
        case HYDRA_MSG_INVALID:
            return ("INVALID");
            break;
        case HYDRA_MSG_FAILED:
            return ("FAILED");
            break;
    }
    return "?";
}

//  --------------------------------------------------------------------------
//  Get/set the post_id field

const char *
hydra_msg_post_id (hydra_msg_t *self)
{
    assert (self);
    return self->post_id;
}

void
hydra_msg_set_post_id (hydra_msg_t *self, const char *format, ...)
{
    //  Format post_id from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->post_id);
    self->post_id = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get the tags field, without transferring ownership

zlist_t *
hydra_msg_tags (hydra_msg_t *self)
{
    assert (self);
    return self->tags;
}

//  Get the tags field and transfer ownership to caller

zlist_t *
hydra_msg_get_tags (hydra_msg_t *self)
{
    assert (self);
    zlist_t *tags = self->tags;
    self->tags = NULL;
    return tags;
}

//  Set the tags field, transferring ownership from caller

void
hydra_msg_set_tags (hydra_msg_t *self, zlist_t **tags_p)
{
    assert (self);
    assert (tags_p);
    zlist_destroy (&self->tags);
    self->tags = *tags_p;
    *tags_p = NULL;
}

//  --------------------------------------------------------------------------
//  Iterate through the tags field, and append a tags value

const char *
hydra_msg_tags_first (hydra_msg_t *self)
{
    assert (self);
    if (self->tags)
        return (char *) (zlist_first (self->tags));
    else
        return NULL;
}

const char *
hydra_msg_tags_next (hydra_msg_t *self)
{
    assert (self);
    if (self->tags)
        return (char *) (zlist_next (self->tags));
    else
        return NULL;
}

void
hydra_msg_tags_append (hydra_msg_t *self, const char *format, ...)
{
    //  Format into newly allocated string
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    char *string = zsys_vprintf (format, argptr);
    va_end (argptr);

    //  Attach string to list
    if (!self->tags) {
        self->tags = zlist_new ();
        zlist_autofree (self->tags);
    }
    zlist_append (self->tags, string);
    free (string);
}

size_t
hydra_msg_tags_size (hydra_msg_t *self)
{
    return zlist_size (self->tags);
}


//  --------------------------------------------------------------------------
//  Get/set the tag field

const char *
hydra_msg_tag (hydra_msg_t *self)
{
    assert (self);
    return self->tag;
}

void
hydra_msg_set_tag (hydra_msg_t *self, const char *format, ...)
{
    //  Format tag from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->tag);
    self->tag = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the reply_to field

const char *
hydra_msg_reply_to (hydra_msg_t *self)
{
    assert (self);
    return self->reply_to;
}

void
hydra_msg_set_reply_to (hydra_msg_t *self, const char *format, ...)
{
    //  Format reply_to from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->reply_to);
    self->reply_to = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the previous field

const char *
hydra_msg_previous (hydra_msg_t *self)
{
    assert (self);
    return self->previous;
}

void
hydra_msg_set_previous (hydra_msg_t *self, const char *format, ...)
{
    //  Format previous from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->previous);
    self->previous = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get/set the timestamp field

uint64_t
hydra_msg_timestamp (hydra_msg_t *self)
{
    assert (self);
    return self->timestamp;
}

void
hydra_msg_set_timestamp (hydra_msg_t *self, uint64_t timestamp)
{
    assert (self);
    self->timestamp = timestamp;
}


//  --------------------------------------------------------------------------
//  Get/set the type field

const char *
hydra_msg_type (hydra_msg_t *self)
{
    assert (self);
    return self->type;
}

void
hydra_msg_set_type (hydra_msg_t *self, const char *format, ...)
{
    //  Format type from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->type);
    self->type = zsys_vprintf (format, argptr);
    va_end (argptr);
}


//  --------------------------------------------------------------------------
//  Get the content field without transferring ownership

zmsg_t *
hydra_msg_content (hydra_msg_t *self)
{
    assert (self);
    return self->content;
}

//  Get the content field and transfer ownership to caller

zmsg_t *
hydra_msg_get_content (hydra_msg_t *self)
{
    zmsg_t *content = self->content;
    self->content = NULL;
    return content;
}

//  Set the content field, transferring ownership from caller

void
hydra_msg_set_content (hydra_msg_t *self, zmsg_t **msg_p)
{
    assert (self);
    assert (msg_p);
    zmsg_destroy (&self->content);
    self->content = *msg_p;
    *msg_p = NULL;
}


//  --------------------------------------------------------------------------
//  Get/set the reason field

const char *
hydra_msg_reason (hydra_msg_t *self)
{
    assert (self);
    return self->reason;
}

void
hydra_msg_set_reason (hydra_msg_t *self, const char *format, ...)
{
    //  Format reason from provided arguments
    assert (self);
    va_list argptr;
    va_start (argptr, format);
    free (self->reason);
    self->reason = zsys_vprintf (format, argptr);
    va_end (argptr);
}



//  --------------------------------------------------------------------------
//  Selftest

int
hydra_msg_test (bool verbose)
{
    printf (" * hydra_msg: ");

    //  @selftest
    //  Simple create/destroy test
    hydra_msg_t *self = hydra_msg_new (0);
    assert (self);
    hydra_msg_destroy (&self);

    //  Create pair of sockets we can send through
    zsock_t *input = zsock_new (ZMQ_ROUTER);
    assert (input);
    zsock_connect (input, "inproc://selftest-hydra_msg");

    zsock_t *output = zsock_new (ZMQ_DEALER);
    assert (output);
    zsock_bind (output, "inproc://selftest-hydra_msg");

    //  Encode/send/decode and verify each message type
    int instance;
    hydra_msg_t *copy;
    self = hydra_msg_new (HYDRA_MSG_HELLO);
    
    //  Check that _dup works on empty message
    copy = hydra_msg_dup (self);
    assert (copy);
    hydra_msg_destroy (&copy);

    //  Send twice from same object
    hydra_msg_send_again (self, output);
    hydra_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = hydra_msg_recv (input);
        assert (self);
        assert (hydra_msg_routing_id (self));
        
        hydra_msg_destroy (&self);
    }
    self = hydra_msg_new (HYDRA_MSG_HELLO_OK);
    
    //  Check that _dup works on empty message
    copy = hydra_msg_dup (self);
    assert (copy);
    hydra_msg_destroy (&copy);

    hydra_msg_set_post_id (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    hydra_msg_send_again (self, output);
    hydra_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = hydra_msg_recv (input);
        assert (self);
        assert (hydra_msg_routing_id (self));
        
        assert (streq (hydra_msg_post_id (self), "Life is short but Now lasts for ever"));
        hydra_msg_destroy (&self);
    }
    self = hydra_msg_new (HYDRA_MSG_GET_TAGS);
    
    //  Check that _dup works on empty message
    copy = hydra_msg_dup (self);
    assert (copy);
    hydra_msg_destroy (&copy);

    //  Send twice from same object
    hydra_msg_send_again (self, output);
    hydra_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = hydra_msg_recv (input);
        assert (self);
        assert (hydra_msg_routing_id (self));
        
        hydra_msg_destroy (&self);
    }
    self = hydra_msg_new (HYDRA_MSG_GET_TAGS_OK);
    
    //  Check that _dup works on empty message
    copy = hydra_msg_dup (self);
    assert (copy);
    hydra_msg_destroy (&copy);

    hydra_msg_tags_append (self, "Name: %s", "Brutus");
    hydra_msg_tags_append (self, "Age: %d", 43);
    //  Send twice from same object
    hydra_msg_send_again (self, output);
    hydra_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = hydra_msg_recv (input);
        assert (self);
        assert (hydra_msg_routing_id (self));
        
        assert (hydra_msg_tags_size (self) == 2);
        assert (streq (hydra_msg_tags_first (self), "Name: Brutus"));
        assert (streq (hydra_msg_tags_next (self), "Age: 43"));
        hydra_msg_destroy (&self);
    }
    self = hydra_msg_new (HYDRA_MSG_GET_TAG);
    
    //  Check that _dup works on empty message
    copy = hydra_msg_dup (self);
    assert (copy);
    hydra_msg_destroy (&copy);

    hydra_msg_set_tag (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    hydra_msg_send_again (self, output);
    hydra_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = hydra_msg_recv (input);
        assert (self);
        assert (hydra_msg_routing_id (self));
        
        assert (streq (hydra_msg_tag (self), "Life is short but Now lasts for ever"));
        hydra_msg_destroy (&self);
    }
    self = hydra_msg_new (HYDRA_MSG_GET_TAG_OK);
    
    //  Check that _dup works on empty message
    copy = hydra_msg_dup (self);
    assert (copy);
    hydra_msg_destroy (&copy);

    hydra_msg_set_post_id (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    hydra_msg_send_again (self, output);
    hydra_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = hydra_msg_recv (input);
        assert (self);
        assert (hydra_msg_routing_id (self));
        
        assert (streq (hydra_msg_post_id (self), "Life is short but Now lasts for ever"));
        hydra_msg_destroy (&self);
    }
    self = hydra_msg_new (HYDRA_MSG_GET_POST);
    
    //  Check that _dup works on empty message
    copy = hydra_msg_dup (self);
    assert (copy);
    hydra_msg_destroy (&copy);

    hydra_msg_set_post_id (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    hydra_msg_send_again (self, output);
    hydra_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = hydra_msg_recv (input);
        assert (self);
        assert (hydra_msg_routing_id (self));
        
        assert (streq (hydra_msg_post_id (self), "Life is short but Now lasts for ever"));
        hydra_msg_destroy (&self);
    }
    self = hydra_msg_new (HYDRA_MSG_GET_POST_OK);
    
    //  Check that _dup works on empty message
    copy = hydra_msg_dup (self);
    assert (copy);
    hydra_msg_destroy (&copy);

    hydra_msg_set_post_id (self, "Life is short but Now lasts for ever");
    hydra_msg_set_reply_to (self, "Life is short but Now lasts for ever");
    hydra_msg_set_previous (self, "Life is short but Now lasts for ever");
    hydra_msg_tags_append (self, "Name: %s", "Brutus");
    hydra_msg_tags_append (self, "Age: %d", 43);
    hydra_msg_set_timestamp (self, 123);
    hydra_msg_set_type (self, "Life is short but Now lasts for ever");
    zmsg_t *get_post_ok_content = zmsg_new ();
    hydra_msg_set_content (self, &get_post_ok_content);
    zmsg_addstr (hydra_msg_content (self), "Hello, World");
    //  Send twice from same object
    hydra_msg_send_again (self, output);
    hydra_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = hydra_msg_recv (input);
        assert (self);
        assert (hydra_msg_routing_id (self));
        
        assert (streq (hydra_msg_post_id (self), "Life is short but Now lasts for ever"));
        assert (streq (hydra_msg_reply_to (self), "Life is short but Now lasts for ever"));
        assert (streq (hydra_msg_previous (self), "Life is short but Now lasts for ever"));
        assert (hydra_msg_tags_size (self) == 2);
        assert (streq (hydra_msg_tags_first (self), "Name: Brutus"));
        assert (streq (hydra_msg_tags_next (self), "Age: 43"));
        assert (hydra_msg_timestamp (self) == 123);
        assert (streq (hydra_msg_type (self), "Life is short but Now lasts for ever"));
        assert (zmsg_size (hydra_msg_content (self)) == 1);
        hydra_msg_destroy (&self);
    }
    self = hydra_msg_new (HYDRA_MSG_GOODBYE);
    
    //  Check that _dup works on empty message
    copy = hydra_msg_dup (self);
    assert (copy);
    hydra_msg_destroy (&copy);

    //  Send twice from same object
    hydra_msg_send_again (self, output);
    hydra_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = hydra_msg_recv (input);
        assert (self);
        assert (hydra_msg_routing_id (self));
        
        hydra_msg_destroy (&self);
    }
    self = hydra_msg_new (HYDRA_MSG_GOODBYE_OK);
    
    //  Check that _dup works on empty message
    copy = hydra_msg_dup (self);
    assert (copy);
    hydra_msg_destroy (&copy);

    //  Send twice from same object
    hydra_msg_send_again (self, output);
    hydra_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = hydra_msg_recv (input);
        assert (self);
        assert (hydra_msg_routing_id (self));
        
        hydra_msg_destroy (&self);
    }
    self = hydra_msg_new (HYDRA_MSG_INVALID);
    
    //  Check that _dup works on empty message
    copy = hydra_msg_dup (self);
    assert (copy);
    hydra_msg_destroy (&copy);

    //  Send twice from same object
    hydra_msg_send_again (self, output);
    hydra_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = hydra_msg_recv (input);
        assert (self);
        assert (hydra_msg_routing_id (self));
        
        hydra_msg_destroy (&self);
    }
    self = hydra_msg_new (HYDRA_MSG_FAILED);
    
    //  Check that _dup works on empty message
    copy = hydra_msg_dup (self);
    assert (copy);
    hydra_msg_destroy (&copy);

    hydra_msg_set_reason (self, "Life is short but Now lasts for ever");
    //  Send twice from same object
    hydra_msg_send_again (self, output);
    hydra_msg_send (&self, output);

    for (instance = 0; instance < 2; instance++) {
        self = hydra_msg_recv (input);
        assert (self);
        assert (hydra_msg_routing_id (self));
        
        assert (streq (hydra_msg_reason (self), "Life is short but Now lasts for ever"));
        hydra_msg_destroy (&self);
    }

    zsock_destroy (&input);
    zsock_destroy (&output);
    //  @end

    printf ("OK\n");
    return 0;
}
