/*  =========================================================================
    hydra_ledger - work with Hydra ledger of posts

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of zbroker, the ZeroMQ broker project.           
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

/*
@header
    hydra_ledger - 
@discuss
@end
*/

#include "../include/hydra.h"

//  Structure of our class

struct _hydra_ledger_t {
    int filler;
};

//  --------------------------------------------------------------------------
//  Create a new hydra_ledger

hydra_ledger_t *
hydra_ledger_new (void)
{
    hydra_ledger_t *self = (hydra_ledger_t *) zmalloc (sizeof (hydra_ledger_t));
    assert (self);

    //  TODO: Initialize properties

    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the hydra_ledger

void
hydra_ledger_destroy (hydra_ledger_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        hydra_ledger_t *self = *self_p;

        //  Free class properties

        //  Free object itself
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Print properties of object

void
hydra_ledger_print (hydra_ledger_t *self)
{
    assert (self);
}


//  --------------------------------------------------------------------------
//  Selftest

int
hydra_ledger_test (bool verbose)
{
    printf (" * hydra_ledger: ");

    //  @selftest
    //  Simple create/destroy test
    hydra_ledger_t *self = hydra_ledger_new ();
    assert (self);
    hydra_ledger_destroy (&self);
    //  @end

    printf ("OK\n");
    return 0;
}
