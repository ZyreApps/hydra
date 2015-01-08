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
    zsock_t *pipe;              //  Actor pipe back to caller
    zconfig_t *config;          //  Current loaded configuration
    hydra_ledger_t *ledger;     //  Posts ledger
};

//  ---------------------------------------------------------------------------
//  This structure defines the state for each client connection. It will
//  be passed to each action in the 'self' argument.

struct _client_t {
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
    self->ledger = hydra_ledger_new ();
//     int posts = hydra_ledger_load (self->ledger);
    return 0;
}

//  Free properties and structures for a server instance

static void
server_terminate (server_t *self)
{
    hydra_ledger_destroy (&self->ledger);
}

//  Process server API method, return reply message if any

static zmsg_t *
server_method (server_t *self, const char *method, zmsg_t *msg)
{
    char *subject = zmsg_popstr (msg);
    char *parent_id = zmsg_popstr (msg);
    hydra_post_t *post = hydra_post_new (subject);
    hydra_post_set_parent_id (post, parent_id);
    
    if (streq (method, "POST")) {
        char *content = zmsg_popstr (msg);
        hydra_post_set_content (post, content);
        zstr_free (&content);
    }
    else
    if (streq (method, "POST FILE")) {
        char *mime_type = zmsg_popstr (msg);
        hydra_post_set_mime_type (post, mime_type);
        zstr_free (&mime_type);
        char *filename = zmsg_popstr (msg);
        hydra_post_set_file (post, filename);
        zstr_free (&filename);
    }
    else
    if (streq (method, "POST DATA")) {
        char *mime_type = zmsg_popstr (msg);
        hydra_post_set_mime_type (post, mime_type);
        zstr_free (&mime_type);
        zframe_t *frame = zmsg_pop (msg);
        hydra_post_set_data (post, zframe_data (frame), zframe_size (frame));
        zframe_destroy (&frame);
    }
    else {
        zsys_error ("unknown server method '%s' - failure", method);
        assert (false);
    }
    zstr_free (&subject);
    zstr_free (&parent_id);

    zmsg_t *reply = zmsg_new ();
    zmsg_addstr (reply, hydra_post_id (post));

    //  add file to ledger and get new filename
    static int counter = 0;
    char filename [20];
    sprintf (filename, "%04d.txt", ++counter);
    hydra_post_save (post, filename);
    hydra_post_destroy (&post);
    return reply;
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
//  calculate_status_for_client
//

static void
calculate_status_for_client (client_t *self)
{

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
    zstr_sendx (server, "LOAD", "hydra.cfg", NULL);
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
