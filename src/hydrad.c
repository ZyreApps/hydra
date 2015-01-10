/*  =========================================================================
    hydrad - command-line service

    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of the Hydra Project

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/
/*
@header
    Hydrad is a test program.
@discuss
@end
*/

#include "hydra_classes.h"

#define PRODUCT         "Hydra service/0.0.1"
#define COPYRIGHT       "Copyright (c) 2014 the Contributors"
#define NOWARRANTY \
"This Software is provided under the MPLv2 License on an \"as is\" basis,\n" \
"without warranty of any kind, either expressed, implied, or statutory.\n"

static void
s_handle_peer (char *endpoint, bool verbose)
{
    hydra_client_verbose = verbose;
    hydra_client_t *client = hydra_client_new (endpoint, 500);
    assert (client);
    int before = hydra_client_before (client);
    int after = hydra_client_after (client);
    zsys_info ("peer status before=%d after=%d", before, after);

    //  Fetch newer posts first
    while (after && !zsys_interrupted) {
        if (hydra_client_fetch (client, HYDRA_PROTO_FETCH_NEWER) == 0)
            after--;
        else {
            zsys_debug ("hydrad: could not fetch newer post");
            break;
        }
    }
    //  Now reset and repeat for all posts
//     if (hydra_client_fetch (client, HYDRA_PROTO_FETCH_RESET) == 0)
    
    zsys_info ("hydrad: synchronized with peer, goodbye");
    hydra_client_destroy (&client);
}

int main (int argc, char *argv [])
{
    puts (PRODUCT);
    puts (COPYRIGHT);
    puts (NOWARRANTY);

    int argn = 1;
    bool verbose = false;
    if (argn < argc && streq (argv [argn], "-h")) {
        puts ("syntax: hydrad [ directory ]");
        puts (" -- defaults to .hydra in current directory");
        exit (0);
    }
    if (argn < argc && streq (argv [argn], "-v")) {
        verbose = true;
        argn++;
    }
    //  By default, current node runs in .hydra directory; create this if
    //  it's missing (don't create directory passed as argument);
    char *workdir = ".hydra";
    if (argn < argc)
        workdir = argv [argn++];
    else
        zsys_dir_create (workdir);

    //  ----------------------------------------------------------------------
    //  This code eventually goes into a reusable hydra actor class

    //  Switch to working directory
    zsys_info ("hydrad: working in directory=%s", workdir);
    if (zsys_dir_change (workdir)) {
        zsys_error ("hydrad: cannot access %s: %s", workdir, strerror (errno));
        return 1;
    }
    //  Check we are the only process currently running here
    if (zsys_run_as ("hydrad.lock", NULL, NULL)) {
        zsys_error ("hydrad: cannot start process safely, exiting");
        return 1;
    }
    //  Start server and bind to ephemeral TCP port. We can run many
    //  servers on the same box, for testing.
    zactor_t *server = zactor_new (hydra_server, NULL);
    if (verbose)
        zstr_send (server, "VERBOSE");

    //  Bind Hydra service to ephemeral port and get that port number
    int port_nbr;
    zsock_send (server, "ss", "LOAD", "hydra.cfg");
    zsock_send (server, "ss", "BIND", "tcp://*:*");
    zsock_send (server, "s", "PORT");
    zsock_recv (server, "si", NULL, &port_nbr);
    zsys_info ("hydrad: TCP server started on port=%d", port_nbr);

    //  Here is how we set the node nickname, persistently
    zsock_send (server, "sss", "SET", "/hydra/nickname", "Anonymous Coward");
    zsock_send (server, "ss", "SAVE", "hydra.cfg");

    //  Provision the Hydra server with some test posts in a tree
    char *post_id;
    zsock_send (server, "ssssss", "POST", "This is a string", "", "text/plain",
                "string", "Hello, World");
    zsock_recv (server, "s", &post_id);
    
    zsock_send (server, "ssssss", "POST", "This is a disk file", post_id,
                "text/zpl", "file", "hydra.cfg");
    zstr_free (&post_id);
    zsock_recv (server, "s", &post_id);
    
    zsock_send (server, "sssssb", "POST", "This is a blob of data", post_id,
                "*/*", "frame", "ABCDEFGHIJ", 10);
    zstr_free (&post_id);
    zsock_recv (server, "s", &post_id);
    zstr_free (&post_id);
                        
    //  We're going to use Zyre for discovery and presence, and our own
    //  Hydra protocol for content exchange
    zyre_t *zyre = zyre_new (NULL);
    if (verbose)
        zyre_set_verbose (zyre);

    char *hostname = zsys_hostname ();
    char *endpoint = zsys_sprintf ("tcp://%s:%d", hostname, port_nbr);
    zyre_set_header (zyre, "X-HYDRA", "%s", endpoint);
    zstr_free (&endpoint);
    zstr_free (&hostname);
    if (zyre_start (zyre)) {
        zsys_info ("hydrad: can't start Zyre discovery service");
        zactor_destroy (&server);
        zyre_destroy (&zyre);
        return 1;
    }
    //  When we get a new peer, handle it
    zpoller_t *poller = zpoller_new (zyre_socket (zyre), NULL);
    while (!zpoller_terminated (poller)) {
        void *which = zpoller_wait (poller, -1);
        if (which == zyre_socket (zyre)) {
            zyre_event_t *event = zyre_event_new (zyre);
            if (zyre_event_type (event) == ZYRE_EVENT_ENTER) {
                char *endpoint = zyre_event_header (event, "X-HYDRA");
                zsys_debug ("hydrad: new peer name=%s endpoint=%s",
                            zyre_event_name (event), endpoint);
                s_handle_peer (endpoint, verbose);
            }
            zyre_event_destroy (&event);
        }
        else
            break;
    }
    zsys_info ("hydrad: shutting down...");
    zpoller_destroy (&poller);

    //  Shutdown all services
    zactor_destroy (&server);
    zyre_destroy (&zyre);
    return 0;
}
