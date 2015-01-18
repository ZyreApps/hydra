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
//  node it is silent and invisible to other nodes on the network. You
//  may specify the working directory, which defaults to .hydra in the
//  current working directory.
HYDRA_EXPORT hydra_t *
    hydra_new (const char *directory);

//  Destructor, destroys a Hydra node. When you destroy a node, any
//  posts it is sending or receiving will be discarded.
HYDRA_EXPORT void
    hydra_destroy (hydra_t **self_p);

//  Return our node name
HYDRA_EXPORT const char *
    hydra_name (hydra_t *self);

//  Set node name; this is saved persistently in the Hydra configuration
//  file.
HYDRA_EXPORT void
    hydra_set_name (hydra_t *self, const char *name);

//  Set verbose mode; this tells the node to log all traffic as well as
//  all major events.
HYDRA_EXPORT void
    hydra_set_verbose (hydra_t *self);

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
