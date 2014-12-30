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
s_handle_peer (char *endpoint)
{
    
}

int main (int argc, char *argv [])
{
    puts (PRODUCT);
    puts (COPYRIGHT);
    puts (NOWARRANTY);

    int argn = 1;
    bool verbose = false;
    if (argc > argn && streq (argv [argn], "-v")) {
        verbose = true;
        argn++;
    }
    //  ----------------------------------------------------------------------
    //  This code would eventually go into a reusable hydra actor class
    
    //  Start server and bind to ephemeral TCP port. We can run many 
    //  servers on the same box, for testing.
    zactor_t *server = zactor_new (hydra_server, NULL);
    if (verbose)
        zstr_send (server, "VERBOSE");

    //  Bind Hydra service to ephemeral port and get that port number
    char *command;
    int port_nbr;
    zsock_send (server, "ss", "BIND", "tcp://*:*");
    zsock_send (server, "s", "PORT");
    zsock_recv (server, "si", &command, &port_nbr);
    zsys_debug ("hydrad: TCP server started on port=%d", port_nbr);
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
    zyre_start (zyre);
    zstr_free (&endpoint);
    zstr_free (&hostname);

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
                s_handle_peer (zyre_event_header (event, "X-HYDRA"));
            }
            zyre_event_destroy (&event);
        }
        else
            break;
    }
    zsys_debug ("hydrad: interrupted");
    zpoller_destroy (&poller);

    //  Shutdown all services
    zsys_debug ("hydrad: shutting down server...");
    zactor_destroy (&server);
    zsys_debug ("hydrad: shutting down Zyre...");
    zyre_destroy (&zyre);
    zsys_debug ("hydrad: exiting");
    return 0;
}
