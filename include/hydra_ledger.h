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
//  Create a new hydra_ledger
HYDRA_EXPORT hydra_ledger_t *
    hydra_ledger_new (void);

//  Destroy the hydra_ledger
HYDRA_EXPORT void
    hydra_ledger_destroy (hydra_ledger_t **self_p);

//  Print properties of object
HYDRA_EXPORT void
    hydra_ledger_print (hydra_ledger_t *self);

//  Self test of this class
HYDRA_EXPORT int
    hydra_ledger_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
