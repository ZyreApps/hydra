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

#include "../include/hydra.h"

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
    hydra_client_fetch (client);
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
    zsys_info ("hydrad: data store in %s directory", workdir);
    if (zsys_dir_change (workdir)) {
        zsys_error ("hydrad: cannot access %s: %s", workdir, strerror (errno));
        return 1;
    }
    //  Check we are the only process currently running here
    if (zsys_run_as ("hydrad.lock", NULL, NULL)) {
        zsys_error ("hydrad: cannot start process safely, exiting");
        return 1;
    }
    //  Get node identity from config file, or generate new identity
    zconfig_t *config = zconfig_load ("hydra.cfg");
    if (!config) {
        //  Set defaults for Hydra service
        config = zconfig_new ("root", NULL);
        zconfig_put (config, "/server/timeout", "5000");
        zconfig_put (config, "/server/background", "0");
        zconfig_put (config, "/server/verbose", "0");
    }
    char *identity = zconfig_resolve (config, "/hydra/identity", NULL);
    if (!identity) {
        zuuid_t *uuid = zuuid_new ();
        zconfig_put (config, "/hydra/identity", zuuid_str (uuid));
        zconfig_put (config, "/hydra/nickname", "Anonymous");
        zconfig_save (config, "hydra.cfg");
        zuuid_destroy (&uuid);
    }
    //  Create store structure, if necessary
    zsys_dir_create ("content");
    zsys_dir_create ("posts");
    
    //  Start server and bind to ephemeral TCP port. We can run many
    //  servers on the same box, for testing.
    zactor_t *server = zactor_new (hydra_server, NULL);
    if (verbose)
        zstr_send (server, "VERBOSE");

    //  Bind Hydra service to ephemeral port and get that port number
    char *command;
    int port_nbr;
    zsock_send (server, "ss", "CONFIGURE", "hydra.cfg");
    zsock_send (server, "ss", "BIND", "tcp://*:*");
    zsock_send (server, "s", "PORT");
    zsock_recv (server, "si", &command, &port_nbr);
    zsys_info ("hydrad: TCP server started on port=%d", port_nbr);
    assert (streq (command, "PORT"));
    free (command);

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
                zsys_debug ("hydrad: new peer name=%s endpoint=%s",
                            zyre_event_name (event),
                            zyre_event_header (event, "X-HYDRA"));
                s_handle_peer (zyre_event_header (event, "X-HYDRA"), verbose);
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
    zconfig_destroy (&config);
    return 0;
}
