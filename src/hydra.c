/*  =========================================================================
    hydra - main Hydra API

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of zbroker, the ZeroMQ broker project.           
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

/*
@header
    The hydra class provides a public API to the Hydra peer to peer
    service.
@discuss
@end
*/

#include "hydra_classes.h"

//  Structure of our class

struct _hydra_t {
    bool verbose;
};

//  --------------------------------------------------------------------------
//  Constructor, creates a new Hydra node. Note that until you start the
//  node it is silent and invisible to other nodes on the network. You
//  may specify the working directory, which defaults to .hydra in the
//  current working directory.

hydra_t *
hydra_new (const char *directory)
{
    hydra_t *self = (hydra_t *) zmalloc (sizeof (hydra_t));
    assert (self);
    return self;
}


//  --------------------------------------------------------------------------
//  Destructor, destroys a Hydra node. When you destroy a node, any
//  posts it is sending or receiving will be discarded.

void
hydra_destroy (hydra_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        hydra_t *self = *self_p;
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Return our node name

const char *
hydra_name (hydra_t *self)
{
    return NULL;
}


//  --------------------------------------------------------------------------
//  Set node name; this is saved persistently in the Hydra configuration
//  file.

void
hydra_set_name (hydra_t *self, const char *name)
{
}


//  --------------------------------------------------------------------------
//  Set verbose mode; this tells the node to log all traffic as well as
//  all major events.

void
hydra_set_verbose (hydra_t *self)
{
    self->verbose = true;
}


//  --------------------------------------------------------------------------
//  Start node. When you start a node it begins discovery and post exchange.
//  Returns 0 if OK, -1 if it wasn't possible to start the node.

int
hydra_start (hydra_t *self)
{
    return -1;
}


//  --------------------------------------------------------------------------
//  Return the Hydra version for run-time API detection

void
hydra_version (int *major, int *minor, int *patch)
{
    *major = HYDRA_VERSION_MAJOR;
    *minor = HYDRA_VERSION_MINOR;
    *patch = HYDRA_VERSION_PATCH;
}


//  --------------------------------------------------------------------------
//  Self test of this class

void
hydra_test (bool verbose)
{
    printf (" * hydra: ");

    //  @selftest
    //  Simple create/destroy test
    hydra_t *self = hydra_new (NULL);
    assert (self);
    hydra_destroy (&self);
    //  @end

    printf ("OK\n");
}
