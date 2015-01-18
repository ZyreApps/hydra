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
    int count = hydra_client_sync (client);
    if (count >= 0)
        zsys_info ("hydrad: received %d posts from peer", count);
    else
        zsys_error ("hydrad: failed - %s", hydra_client_reason (client));
    hydra_client_destroy (&client);
}


int main (int argc, char *argv [])
{
    puts (PRODUCT);
    puts (COPYRIGHT);
    puts (NOWARRANTY);

    int argn = 1;
    if (argn < argc && streq (argv [argn], "-h")) {
        puts ("syntax: hydrad [-v] [-z] [ [-t] [-i] directory ]");
        puts (" -- defaults to .hydra in current directory");
        puts (" -v = run Hydra protocol in verbose mode");
        puts (" -z = run Zyre discovery in verbose mode");
        puts (" -t = create some test posts");
        puts (" -i = run over ipc:// without networking");
        exit (0);
    }
    bool verbose = false;
    bool zverbose = false;
    bool testmode = false;
    bool localhost = false;
    while (argn < argc && *argv [argn] == '-') {
        if (streq (argv [argn], "-v"))
            verbose = true;
        else
        if (streq (argv [argn], "-z"))
            zverbose = true;
        else
        if (streq (argv [argn], "-t"))
            testmode = true;
        else
        if (streq (argv [argn], "-i"))
            localhost = true;
        else {
            puts ("Invalid option, run hydrad -h to see options");
            exit (0);
        }
        argn++;
    }
    //  ----------------------------------------------------------------------
    //  This code eventually goes into a reusable hydra actor class
    
    //  Specify directory on command line or use .hydra by default
    //  Create directory if necessary
    char *workdir = ".hydra";
    if (argn < argc)
        workdir = argv [argn++];
    zsys_dir_create (workdir);

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
    zactor_t *server = zactor_new (hydra_server, NULL);
    if (verbose)
        zstr_send (server, "VERBOSE");

    //  Here is how we set the node nickname, persistently.
    //  TODO: make API getter/setter methods for this
    zsock_send (server, "ss", "LOAD", "hydra.cfg");
    zsock_send (server, "sss", "SET", "/hydra/nickname", "Anonymous Coward");
    zsock_send (server, "ss", "SAVE", "hydra.cfg");

    if (testmode) {
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
    }
    //  We're going to use Zyre for discovery and presence, and our own Hydra
    //  protocol for content exchange
    zyre_t *zyre = zyre_new (NULL);
    if (zverbose)
        zyre_set_verbose (zyre);

    //  Set up networking; either over TCP (requires active network interface)
    //  or over IPC, using the Zyre gossip API
    char *endpoint;
    if (localhost) {
        zyre_set_endpoint (zyre, "ipc://@/zyre/%s", workdir);
        if (streq (workdir, ".hydra"))
            zyre_gossip_bind (zyre, "ipc://@/hydra_gossip_hub");
        else
            zyre_gossip_connect (zyre, "ipc://@/hydra_gossip_hub");
        endpoint = zsys_sprintf ("ipc://@/hydra/%s", workdir);
        zsock_send (server, "ss", "BIND", endpoint);
    }
    else {
        //  Bind Hydra service to ephemeral port and get that port number
        int port_nbr;
        zsock_send (server, "ss", "BIND", "tcp://*:*");
        zsock_send (server, "s", "PORT");
        zsock_recv (server, "si", NULL, &port_nbr);
        char *hostname = zsys_hostname ();
        endpoint = zsys_sprintf ("tcp://%s:%d", hostname, port_nbr);
        zstr_free (&hostname);
    }
    zsys_info ("hydrad: Hydra server started on %s", endpoint);
    zyre_set_header (zyre, "X-HYDRA", "%s", endpoint);
    zstr_free (&endpoint);
    
    if (zyre_start (zyre)) {
        zsys_info ("hydrad: can't start Zyre discovery service");
        zactor_destroy (&server);
        zyre_destroy (&zyre);
        return 1;
    }
    if (verbose)
        zyre_dump (zyre);
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
