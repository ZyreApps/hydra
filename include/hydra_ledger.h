/*  =========================================================================
    hydra_ledger - work with Hydra ledger of posts

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of zbroker, the ZeroMQ broker project.           
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

#ifndef __HYDRA_LEDGER_H_INCLUDED__
#define __HYDRA_LEDGER_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif


//  @interface
//  Create a new empty ledger instance.
HYDRA_EXPORT hydra_ledger_t *
    hydra_ledger_new (void);

//  Destroy the hydra_ledger
HYDRA_EXPORT void
    hydra_ledger_destroy (hydra_ledger_t **self_p);

//  Load the ledger data from disk, from the specified directory. Returns the
//  number of posts loaded, or -1 if there was an error reading the directory.
HYDRA_EXPORT int
    hydra_ledger_load (hydra_ledger_t *self);
    
//  Self test of this class
HYDRA_EXPORT int
    hydra_ledger_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
