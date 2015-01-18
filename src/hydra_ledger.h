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

typedef struct _hydra_ledger_t hydra_ledger_t;

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

//  Return ledger size, i.e. number of posts stored in the ledger.
HYDRA_EXPORT size_t
    hydra_ledger_size (hydra_ledger_t *self);
    
//  Load the ledger data from disk, from the specified directory. Returns the
//  number of posts loaded, or -1 if there was an error reading the directory.
HYDRA_EXPORT int
    hydra_ledger_load (hydra_ledger_t *self);
    
//  Save a post via the ledger. This saves the post to disk, adds the post to
//  the ledger, and then destroys the post. Use in place of hydra_post_save
//  to ensure post filenames are correctly generated.
HYDRA_EXPORT int
    hydra_ledger_store (hydra_ledger_t *self, hydra_post_t **post_p);
    
//  Return post at specified index; if the index does not refer to a valid
//  post, returns NULL.
HYDRA_EXPORT hydra_post_t *
    hydra_ledger_fetch (hydra_ledger_t *self, int index);

//  Lookup post in ledger and return post index (0 .. size - 1); if the post
//  does not exist, returns -1.
HYDRA_EXPORT int
    hydra_ledger_index (hydra_ledger_t *self, const char *post_id);

//  Self test of this class
HYDRA_EXPORT int
    hydra_ledger_test (bool verbose);
//  @end

#ifdef __cplusplus
}
#endif

#endif
