/*  =========================================================================
    hydra - main Hydra API

    Copyright (c) the Contributors as noted in the AUTHORS file.
    This file is part of zbroker, the ZeroMQ broker project.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
    =========================================================================
*/

#ifndef __HYDRA_API_H_INCLUDED__
#define __HYDRA_API_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif

//  Include the project library file
#include "hydra_library.h"

//  @interface
//  Constructor, creates a new Hydra node. Note that until you start the
//  node it is silent and invisible to other nodes on the network. You may
//  specify the working directory, which defaults to .hydra in the current
//  working directory. Creates the working directory if necessary.
HYDRA_EXPORT hydra_t *
    hydra_new (const char *directory);

//  Destructor, destroys a Hydra node. When you destroy a node, any posts
//  it is sending or receiving will be discarded.
HYDRA_EXPORT void
    hydra_destroy (hydra_t **self_p);

//  Set node nickname; this is saved persistently in the Hydra configuration
//  file.
HYDRA_EXPORT void
    hydra_set_nickname (hydra_t *self, const char *nickname);

//  Return our node nickname, as previously stored in hydra.cfg, or set by
//  the hydra_set_nickname() method. Caller must free returned string using
//  zstr_free ().
HYDRA_EXPORT const char *
    hydra_nickname (hydra_t *self);

//  Set the trace level to animation of main actors; this is helpful to
//  debug the Hydra protocol flow.
HYDRA_EXPORT void
    hydra_set_animate (hydra_t *self);

//  Set the trace level to animation of all actors including those used in
//  security and discovery. Use this to collect diagnostic logs.
HYDRA_EXPORT void
    hydra_set_verbose (hydra_t *self);

//  By default, Hydra needs a network interface capable of broadcast UDP
//  traffic, e.g. WiFi or LAN interface. To run nodes on a stand-alone PC,
//  set the local IPC option. The node will then do gossip discovery over
//  IPC. Gossip discovery needs at exactly one node to be running in a
//  directory called ".hydra".
HYDRA_EXPORT void
    hydra_set_local_ipc (hydra_t *self);

//  Start node. When you start a node it begins discovery and post exchange.
//  Returns 0 if OK, -1 if it wasn't possible to start the node.
HYDRA_EXPORT int
    hydra_start (hydra_t *self);

//  Return the Hydra version for run-time API detection
HYDRA_EXPORT void
    hydra_version (int *major, int *minor, int *patch);

//  Self test of this class
HYDRA_EXPORT void
    hydra_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
