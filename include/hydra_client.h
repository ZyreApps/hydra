/*  =========================================================================
    hydra_client - Hydra Client

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: hydra_client.xml, or
     * The code generation script that built this file: zproto_client_c
    ************************************************************************
    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of zbroker, the ZeroMQ broker project.           
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

#ifndef __HYDRA_CLIENT_H_INCLUDED__
#define __HYDRA_CLIENT_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
#ifndef HYDRA_CLIENT_T_DEFINED
typedef struct _hydra_client_t hydra_client_t;
#define HYDRA_CLIENT_T_DEFINED
#endif

//  @interface
//  Create a new hydra_client
//  Connect to server endpoint, with specified timeout in msecs (zero means wait    
//  forever). Constructor succeeds if connection is successful.                     
hydra_client_t *
    hydra_client_new (const char *endpoint, int timeout);

//  Destroy the hydra_client
void
    hydra_client_destroy (hydra_client_t **self_p);

//  Enable verbose logging of client activity
void
    hydra_client_verbose (hydra_client_t *self);

//  Return message pipe for asynchronous message I/O. In the high-volume case,
//  we send methods and get replies to the actor, in a synchronous manner, and
//  we send/recv high volume message data to a second pipe, the msgpipe. In
//  the low-volume case we can do everything over the actor pipe, if traffic
//  is never ambiguous.
zsock_t *
    hydra_client_msgpipe (hydra_client_t *self);

//  Get list of tags from peer                                                      
//  Returns >= 0 if successful, -1 if interrupted.
int
    hydra_client_get_tags (hydra_client_t *self);

//  Fetch latest post for a given tag                                               
//  Returns >= 0 if successful, -1 if interrupted.
int
    hydra_client_get_tag (hydra_client_t *self, const char *tag);

//  Fetch a specific post byt ID                                                    
//  Returns >= 0 if successful, -1 if interrupted.
int
    hydra_client_get_post (hydra_client_t *self, const char *post_id);

//  Return last received status
int 
    hydra_client_status (hydra_client_t *self);

//  Return last received reason
char *
    hydra_client_reason (hydra_client_t *self);

//  Return last received tags
char *
    hydra_client_tags (hydra_client_t *self);

//  Return last received tag
char *
    hydra_client_tag (hydra_client_t *self);

//  Return last received post_id
char *
    hydra_client_post_id (hydra_client_t *self);

//  Return last received reply_to
char *
    hydra_client_reply_to (hydra_client_t *self);

//  Return last received previous
char *
    hydra_client_previous (hydra_client_t *self);

//  Return last received timestamp
int 
    hydra_client_timestamp (hydra_client_t *self);

//  Return last received type
char *
    hydra_client_type (hydra_client_t *self);

//  Return last received content
char *
    hydra_client_content (hydra_client_t *self);

//  Self test of this class
void
    hydra_client_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
