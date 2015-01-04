/*  =========================================================================
    hydra_server - Hydra Server (in C)

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of zbroker, the ZeroMQ broker project.           
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

/*
@header
    This is a simple client API for the Hydra protocol.
@discuss
    Detailed discussion of the class, if any.
@end
*/

#include "hydra_classes.h"

//  Maximum size of a content we'll work with for now is 10MB
//  We should use chunking / credit based flow control for larger files
#define CONTENT_MAX_SIZE    10 * 1024 * 1024

//  ---------------------------------------------------------------------------
//  Forward declarations for the two main classes we use here

typedef struct _server_t server_t;
typedef struct _client_t client_t;

//  This structure defines the context for each running server. Store
//  whatever properties and structures you need for the server.

struct _server_t {
    //  These properties must always be present in the server_t
    //  and are set by the generated engine; do not modify them!
    zsock_t *pipe;              //  Actor pipe back to caller
    zconfig_t *config;          //  Current loaded configuration
    
    zfile_t *ledger;            //  Post ledger
    const char *last_post_id;   //  Last post we stored
};

//  ---------------------------------------------------------------------------
//  This structure defines the state for each client connection. It will
//  be passed to each action in the 'self' argument.

struct _client_t {
    //  These properties must always be present in the client_t
    //  and are set by the generated engine; do not modify them!
    server_t *server;           //  Reference to parent server
    hydra_proto_t *message;     //  Message from and to client
};

//  Include the generated server engine
#include "hydra_server_engine.inc"

//  Allocate properties and structures for a new server instance.
//  Return 0 if OK, or -1 if there was an error.

static int
server_initialize (server_t *self)
{
    //  Open ledger and read to end
    self->last_post_id = "";
    self->ledger = zfile_new (".", "ledger.txt");
    if (zfile_input (self->ledger) == 0) {
        const char *post_id = zfile_readln (self->ledger);
        while (post_id) {
            //  Check post exists
            char *filename = zsys_sprintf ("posts/%s", post_id);
            if (!zsys_file_exists (filename))
                zsys_error ("Post %s is missing", post_id);
            zstr_free (&filename);
            
            self->last_post_id = post_id;
            post_id = zfile_readln (self->ledger);
        }
        zfile_close (self->ledger);
    }
    return 0;
}

//  Free properties and structures for a server instance

static void
server_terminate (server_t *self)
{
    zfile_destroy (&self->ledger);
}

//  Process server API method, return reply message if any

static zmsg_t *
server_method (server_t *self, const char *method, zmsg_t *msg)
{
    return NULL;
}


//  Allocate properties and structures for a new client connection and
//  optionally engine_set_next_event (). Return 0 if OK, or -1 on error.

static int
client_initialize (client_t *self)
{
    //  Construct properties here
    return 0;
}

//  Free properties and structures for a client connection

static void
client_terminate (client_t *self)
{
    //  Destroy properties here
}


//  ---------------------------------------------------------------------------
//  set_server_identity
//

static void
set_server_identity (client_t *self)
{
    char *identity = zconfig_resolve (self->server->config, "/hydra/identity", NULL);
    char *nickname = zconfig_resolve (self->server->config, "/hydra/nickname", "");
    assert (identity);
    hydra_proto_set_identity (self->message, identity);
    hydra_proto_set_nickname (self->message, nickname);
}



//  ---------------------------------------------------------------------------
//  get_most_recent_post
//

static void
get_most_recent_post (client_t *self)
{
    hydra_proto_set_post_id (self->message, self->server->last_post_id);
}


//  ---------------------------------------------------------------------------
//  get_single_post
//

static void
get_single_post (client_t *self)
{
    zsys_info ("sending post=%s", hydra_proto_post_id (self->message));
    char *filename = zsys_sprintf ("posts/%s", hydra_proto_post_id (self->message));
    zconfig_t *post = zconfig_load (filename);
    zstr_free (&filename);
    if (post) {
        //  Get post metadata into message
        hydra_proto_set_reply_to  (self->message, zconfig_resolve (post, "/post/reply_to", ""));
        hydra_proto_set_previous  (self->message, zconfig_resolve (post, "/post/previous", ""));
        hydra_proto_set_timestamp (self->message, zconfig_resolve (post, "/post/timestamp", ""));
        hydra_proto_set_digest    (self->message, zconfig_resolve (post, "/post/digest", ""));
        hydra_proto_set_type      (self->message, zconfig_resolve (post, "/post/type", ""));
        
        //  Get post content into message
        if (*hydra_proto_digest (self->message)) {
            filename = zsys_sprintf ("content/%s", hydra_proto_digest (self->message));
            zchunk_t *chunk = zchunk_slurp (filename, CONTENT_MAX_SIZE);
            if (chunk) {
                zsys_info (" - content size=%zd", zchunk_size (chunk));
                hydra_proto_set_content (self->message, &chunk);
            }
            zstr_free (&filename);
        }
    }
    else {
        hydra_proto_set_status (self->message, HYDRA_PROTO_NOT_FOUND);
        engine_set_exception (self, exception_event);
    }
}


//  ---------------------------------------------------------------------------
//  allow_time_to_settle
//

static void
allow_time_to_settle (client_t *self)
{
    //  Give client to come back with HELLO if we restarted
    engine_set_wakeup_event (self, 200, settled_event);
}


//  ---------------------------------------------------------------------------
//  signal_command_invalid
//

static void
signal_command_invalid (client_t *self)
{
    hydra_proto_set_status (self->message, HYDRA_PROTO_COMMAND_INVALID);
}


//  ---------------------------------------------------------------------------
//  Selftest

void
hydra_server_test (bool verbose)
{
    printf (" * hydra_server: ");
    if (verbose)
        printf ("\n");
    
    //  @selftest
    zactor_t *server = zactor_new (hydra_server, "server");
    if (verbose)
        zstr_send (server, "VERBOSE");
    zstr_sendx (server, "CONFIGURE", "hydra.cfg", NULL);
    zstr_sendx (server, "BIND", "ipc://@/hydra_server", NULL);

    zsock_t *client = zsock_new (ZMQ_DEALER);
    assert (client);
    zsock_set_rcvtimeo (client, 2000);
    zsock_connect (client, "ipc://@/hydra_server");

    hydra_proto_t *message = hydra_proto_new ();
    hydra_proto_set_id (message, HYDRA_PROTO_HELLO);
    hydra_proto_send (message, client);
    hydra_proto_recv (message, client);
    assert (hydra_proto_id (message) == HYDRA_PROTO_HELLO_OK);
    hydra_proto_destroy (&message);
    
    zsock_destroy (&client);
    zactor_destroy (&server);
    //  @end
    printf ("OK\n");
}
