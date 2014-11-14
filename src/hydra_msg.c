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
    char post_id [256];                 //  Post identifier
    char tags [256];                    //  List of known tags
    char tag [256];                     //  Name of tag
    char reply_to [256];                //  Parent post, if any
    char previous [256];                //  Previous post, if any
    uint64_t timestamp;                 //  Post creation timestamp
    char type [256];                    //  Content type
    char content [256];                 //  Content body
    char reason [256];                  //  Reason for failure
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
    if (self->needle + size > self->ceiling) { \
        zsys_warning ("hydra_msg: GET_OCTETS failed"); \
        goto malformed; \
    } \
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
    if (self->needle + 1 > self->ceiling) { \
        zsys_warning ("hydra_msg: GET_NUMBER1 failed"); \
        goto malformed; \
    } \
    (host) = *(byte *) self->needle; \
    self->needle++; \
}

//  Get a 2-byte number from the frame
#define GET_NUMBER2(host) { \
    if (self->needle + 2 > self->ceiling) { \
        zsys_warning ("hydra_msg: GET_NUMBER2 failed"); \
        goto malformed; \
    } \
    (host) = ((uint16_t) (self->needle [0]) << 8) \
           +  (uint16_t) (self->needle [1]); \
    self->needle += 2; \
}

//  Get a 4-byte number from the frame
#define GET_NUMBER4(host) { \
    if (self->needle + 4 > self->ceiling) { \
        zsys_warning ("hydra_msg: GET_NUMBER4 failed"); \
        goto malformed; \
    } \
    (host) = ((uint32_t) (self->needle [0]) << 24) \
           + ((uint32_t) (self->needle [1]) << 16) \
           + ((uint32_t) (self->needle [2]) << 8) \
           +  (uint32_t) (self->needle [3]); \
    self->needle += 4; \
}

//  Get a 8-byte number from the frame
#define GET_NUMBER8(host) { \
    if (self->needle + 8 > self->ceiling) { \
        zsys_warning ("hydra_msg: GET_NUMBER8 failed"); \
        goto malformed; \
    } \
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
    if (self->needle + string_size > (self->ceiling)) { \
        zsys_warning ("hydra_msg: GET_STRING failed"); \
        goto malformed; \
    } \
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
    if (self->needle + string_size > (self->ceiling)) { \
        zsys_warning ("hydra_msg: GET_LONGSTR failed"); \
        goto malformed; \
    } \
    free ((host)); \
    (host) = (char *) malloc (string_size + 1); \
    memcpy ((host), self->needle, string_size); \
    (host) [string_size] = 0; \
    self->needle += string_size; \
}


//  --------------------------------------------------------------------------
//  Create a new hydra_msg

hydra_msg_t *
hydra_msg_new (void)
{
    hydra_msg_t *self = (hydra_msg_t *) zmalloc (sizeof (hydra_msg_t));
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

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Receive a hydra_msg from the socket. Returns 0 if OK, -1 if
//  there was an error. Blocks if there is no message waiting.

int
hydra_msg_recv (hydra_msg_t *self, zsock_t *input)
{
    assert (input);
    
    if (zsock_type (input) == ZMQ_ROUTER) {
        zframe_destroy (&self->routing_id);
        self->routing_id = zframe_recv (input);
        if (!self->routing_id || !zsock_rcvmore (input)) {
            zsys_warning ("hydra_msg: no routing ID");
            return -1;          //  Interrupted or malformed
        }
    }
    zmq_msg_t frame;
    zmq_msg_init (&frame);
    int size = zmq_msg_recv (&frame, zsock_resolve (input), 0);
    if (size == -1) {
        zsys_warning ("hydra_msg: interrupted");
        goto malformed;         //  Interrupted
    }
    //  Get and check protocol signature
    self->needle = (byte *) zmq_msg_data (&frame);
    self->ceiling = self->needle + zmq_msg_size (&frame);
    
    uint16_t signature;
    GET_NUMBER2 (signature);
    if (signature != (0xAAA0 | 0)) {
        zsys_warning ("hydra_msg: invalid signature");
        //  TODO: discard invalid messages and loop, and return
        //  -1 only on interrupt
        goto malformed;         //  Interrupted
    }
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
            GET_STRING (self->tags);
            break;

        case HYDRA_MSG_GET_TAG:
            GET_STRING (self->tag);
            break;

        case HYDRA_MSG_GET_TAG_OK:
            GET_STRING (self->tag);
            GET_STRING (self->post_id);
            break;

        case HYDRA_MSG_GET_POST:
            GET_STRING (self->post_id);
            break;

        case HYDRA_MSG_GET_POST_OK:
            GET_STRING (self->post_id);
            GET_STRING (self->reply_to);
            GET_STRING (self->previous);
            GET_STRING (self->tags);
            GET_NUMBER8 (self->timestamp);
            GET_STRING (self->type);
            GET_STRING (self->content);
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
            zsys_warning ("hydra_msg: bad message ID");
            goto malformed;
    }
    //  Successful return
    zmq_msg_close (&frame);
    return 0;

    //  Error returns
    malformed:
        zsys_warning ("hydra_msg: hydra_msg malformed message, fail");
        zmq_msg_close (&frame);
        return -1;              //  Invalid message
}


//  --------------------------------------------------------------------------
//  Send the hydra_msg to the socket. Does not destroy it. Returns 0 if
//  OK, else -1.

int
hydra_msg_send (hydra_msg_t *self, zsock_t *output)
{
    assert (self);
    assert (output);

    if (zsock_type (output) == ZMQ_ROUTER)
        zframe_send (&self->routing_id, output, ZFRAME_MORE + ZFRAME_REUSE);

    size_t frame_size = 2 + 1;          //  Signature and message ID
    switch (self->id) {
        case HYDRA_MSG_HELLO_OK:
            frame_size += 1 + strlen (self->post_id);
            break;
        case HYDRA_MSG_GET_TAGS_OK:
            frame_size += 1 + strlen (self->tags);
            break;
        case HYDRA_MSG_GET_TAG:
            frame_size += 1 + strlen (self->tag);
            break;
        case HYDRA_MSG_GET_TAG_OK:
            frame_size += 1 + strlen (self->tag);
            frame_size += 1 + strlen (self->post_id);
            break;
        case HYDRA_MSG_GET_POST:
            frame_size += 1 + strlen (self->post_id);
            break;
        case HYDRA_MSG_GET_POST_OK:
            frame_size += 1 + strlen (self->post_id);
            frame_size += 1 + strlen (self->reply_to);
            frame_size += 1 + strlen (self->previous);
            frame_size += 1 + strlen (self->tags);
            frame_size += 8;            //  timestamp
            frame_size += 1 + strlen (self->type);
            frame_size += 1 + strlen (self->content);
            break;
        case HYDRA_MSG_FAILED:
            frame_size += 1 + strlen (self->reason);
            break;
    }
    //  Now serialize message into the frame
    zmq_msg_t frame;
    zmq_msg_init_size (&frame, frame_size);
    self->needle = (byte *) zmq_msg_data (&frame);
    PUT_NUMBER2 (0xAAA0 | 0);
    PUT_NUMBER1 (self->id);
    size_t nbr_frames = 1;              //  Total number of frames to send
    
    switch (self->id) {
        case HYDRA_MSG_HELLO_OK:
            PUT_STRING (self->post_id);
            break;

        case HYDRA_MSG_GET_TAGS_OK:
            PUT_STRING (self->tags);
            break;

        case HYDRA_MSG_GET_TAG:
            PUT_STRING (self->tag);
            break;

        case HYDRA_MSG_GET_TAG_OK:
            PUT_STRING (self->tag);
            PUT_STRING (self->post_id);
            break;

        case HYDRA_MSG_GET_POST:
            PUT_STRING (self->post_id);
            break;

        case HYDRA_MSG_GET_POST_OK:
            PUT_STRING (self->post_id);
            PUT_STRING (self->reply_to);
            PUT_STRING (self->previous);
            PUT_STRING (self->tags);
            PUT_NUMBER8 (self->timestamp);
            PUT_STRING (self->type);
            PUT_STRING (self->content);
            break;

        case HYDRA_MSG_FAILED:
            PUT_STRING (self->reason);
            break;

    }
    //  Now send the data frame
    zmq_msg_send (&frame, zsock_resolve (output), --nbr_frames? ZMQ_SNDMORE: 0);
    
    return 0;
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
            if (self->tags)
                zsys_debug ("    tags='%s'", self->tags);
            else
                zsys_debug ("    tags=");
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
            if (self->tag)
                zsys_debug ("    tag='%s'", self->tag);
            else
                zsys_debug ("    tag=");
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
            if (self->tags)
                zsys_debug ("    tags='%s'", self->tags);
            else
                zsys_debug ("    tags=");
            zsys_debug ("    timestamp=%ld", (long) self->timestamp);
            if (self->type)
                zsys_debug ("    type='%s'", self->type);
            else
                zsys_debug ("    type=");
            if (self->content)
                zsys_debug ("    content='%s'", self->content);
            else
                zsys_debug ("    content=");
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
hydra_msg_set_post_id (hydra_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->post_id)
        return;
    strncpy (self->post_id, value, 255);
    self->post_id [255] = 0;
}


//  --------------------------------------------------------------------------
//  Get/set the tags field

const char *
hydra_msg_tags (hydra_msg_t *self)
{
    assert (self);
    return self->tags;
}

void
hydra_msg_set_tags (hydra_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->tags)
        return;
    strncpy (self->tags, value, 255);
    self->tags [255] = 0;
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
hydra_msg_set_tag (hydra_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->tag)
        return;
    strncpy (self->tag, value, 255);
    self->tag [255] = 0;
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
hydra_msg_set_reply_to (hydra_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->reply_to)
        return;
    strncpy (self->reply_to, value, 255);
    self->reply_to [255] = 0;
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
hydra_msg_set_previous (hydra_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->previous)
        return;
    strncpy (self->previous, value, 255);
    self->previous [255] = 0;
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
hydra_msg_set_type (hydra_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->type)
        return;
    strncpy (self->type, value, 255);
    self->type [255] = 0;
}


//  --------------------------------------------------------------------------
//  Get/set the content field

const char *
hydra_msg_content (hydra_msg_t *self)
{
    assert (self);
    return self->content;
}

void
hydra_msg_set_content (hydra_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->content)
        return;
    strncpy (self->content, value, 255);
    self->content [255] = 0;
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
hydra_msg_set_reason (hydra_msg_t *self, const char *value)
{
    assert (self);
    assert (value);
    if (value == self->reason)
        return;
    strncpy (self->reason, value, 255);
    self->reason [255] = 0;
}



//  --------------------------------------------------------------------------
//  Selftest

int
hydra_msg_test (bool verbose)
{
    printf (" * hydra_msg: ");

    //  @selftest
    //  Simple create/destroy test
    hydra_msg_t *self = hydra_msg_new ();
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
    self = hydra_msg_new ();
    hydra_msg_set_id (self, HYDRA_MSG_HELLO);

    //  Send twice
    hydra_msg_send (self, output);
    hydra_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        hydra_msg_recv (self, input);
        assert (hydra_msg_routing_id (self));
    }
    hydra_msg_set_id (self, HYDRA_MSG_HELLO_OK);

    hydra_msg_set_post_id (self, "Life is short but Now lasts for ever");
    //  Send twice
    hydra_msg_send (self, output);
    hydra_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        hydra_msg_recv (self, input);
        assert (hydra_msg_routing_id (self));
        assert (streq (hydra_msg_post_id (self), "Life is short but Now lasts for ever"));
    }
    hydra_msg_set_id (self, HYDRA_MSG_GET_TAGS);

    //  Send twice
    hydra_msg_send (self, output);
    hydra_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        hydra_msg_recv (self, input);
        assert (hydra_msg_routing_id (self));
    }
    hydra_msg_set_id (self, HYDRA_MSG_GET_TAGS_OK);

    hydra_msg_set_tags (self, "Life is short but Now lasts for ever");
    //  Send twice
    hydra_msg_send (self, output);
    hydra_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        hydra_msg_recv (self, input);
        assert (hydra_msg_routing_id (self));
        assert (streq (hydra_msg_tags (self), "Life is short but Now lasts for ever"));
    }
    hydra_msg_set_id (self, HYDRA_MSG_GET_TAG);

    hydra_msg_set_tag (self, "Life is short but Now lasts for ever");
    //  Send twice
    hydra_msg_send (self, output);
    hydra_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        hydra_msg_recv (self, input);
        assert (hydra_msg_routing_id (self));
        assert (streq (hydra_msg_tag (self), "Life is short but Now lasts for ever"));
    }
    hydra_msg_set_id (self, HYDRA_MSG_GET_TAG_OK);

    hydra_msg_set_tag (self, "Life is short but Now lasts for ever");
    hydra_msg_set_post_id (self, "Life is short but Now lasts for ever");
    //  Send twice
    hydra_msg_send (self, output);
    hydra_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        hydra_msg_recv (self, input);
        assert (hydra_msg_routing_id (self));
        assert (streq (hydra_msg_tag (self), "Life is short but Now lasts for ever"));
        assert (streq (hydra_msg_post_id (self), "Life is short but Now lasts for ever"));
    }
    hydra_msg_set_id (self, HYDRA_MSG_GET_POST);

    hydra_msg_set_post_id (self, "Life is short but Now lasts for ever");
    //  Send twice
    hydra_msg_send (self, output);
    hydra_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        hydra_msg_recv (self, input);
        assert (hydra_msg_routing_id (self));
        assert (streq (hydra_msg_post_id (self), "Life is short but Now lasts for ever"));
    }
    hydra_msg_set_id (self, HYDRA_MSG_GET_POST_OK);

    hydra_msg_set_post_id (self, "Life is short but Now lasts for ever");
    hydra_msg_set_reply_to (self, "Life is short but Now lasts for ever");
    hydra_msg_set_previous (self, "Life is short but Now lasts for ever");
    hydra_msg_set_tags (self, "Life is short but Now lasts for ever");
    hydra_msg_set_timestamp (self, 123);
    hydra_msg_set_type (self, "Life is short but Now lasts for ever");
    hydra_msg_set_content (self, "Life is short but Now lasts for ever");
    //  Send twice
    hydra_msg_send (self, output);
    hydra_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        hydra_msg_recv (self, input);
        assert (hydra_msg_routing_id (self));
        assert (streq (hydra_msg_post_id (self), "Life is short but Now lasts for ever"));
        assert (streq (hydra_msg_reply_to (self), "Life is short but Now lasts for ever"));
        assert (streq (hydra_msg_previous (self), "Life is short but Now lasts for ever"));
        assert (streq (hydra_msg_tags (self), "Life is short but Now lasts for ever"));
        assert (hydra_msg_timestamp (self) == 123);
        assert (streq (hydra_msg_type (self), "Life is short but Now lasts for ever"));
        assert (streq (hydra_msg_content (self), "Life is short but Now lasts for ever"));
    }
    hydra_msg_set_id (self, HYDRA_MSG_GOODBYE);

    //  Send twice
    hydra_msg_send (self, output);
    hydra_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        hydra_msg_recv (self, input);
        assert (hydra_msg_routing_id (self));
    }
    hydra_msg_set_id (self, HYDRA_MSG_GOODBYE_OK);

    //  Send twice
    hydra_msg_send (self, output);
    hydra_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        hydra_msg_recv (self, input);
        assert (hydra_msg_routing_id (self));
    }
    hydra_msg_set_id (self, HYDRA_MSG_INVALID);

    //  Send twice
    hydra_msg_send (self, output);
    hydra_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        hydra_msg_recv (self, input);
        assert (hydra_msg_routing_id (self));
    }
    hydra_msg_set_id (self, HYDRA_MSG_FAILED);

    hydra_msg_set_reason (self, "Life is short but Now lasts for ever");
    //  Send twice
    hydra_msg_send (self, output);
    hydra_msg_send (self, output);

    for (instance = 0; instance < 2; instance++) {
        hydra_msg_recv (self, input);
        assert (hydra_msg_routing_id (self));
        assert (streq (hydra_msg_reason (self), "Life is short but Now lasts for ever"));
    }

    hydra_msg_destroy (&self);
    zsock_destroy (&input);
    zsock_destroy (&output);
    //  @end

    printf ("OK\n");
    return 0;
}
