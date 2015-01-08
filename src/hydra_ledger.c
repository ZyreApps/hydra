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
    Works with a ledger of posts. The ledger is the database of all posts
    held by a node.
@discuss
@end
*/

#include "hydra_classes.h"

//  Structure of our class

struct _hydra_ledger_t {
    zhash_t *posts_hash;    //  Hash table containing all post IDs
};


//  --------------------------------------------------------------------------
//  Create a new ledger instance. You must specify the directory that holds
//  post files.

hydra_ledger_t *
hydra_ledger_new (void)
{
    hydra_ledger_t *self = (hydra_ledger_t *) zmalloc (sizeof (hydra_ledger_t));
    if (self)
        self->posts_hash = zhash_new ();
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the ledger instance

void
hydra_ledger_destroy (hydra_ledger_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        hydra_ledger_t *self = *self_p;
        zhash_destroy (&self->posts_hash);
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Load the ledger data from disk, from the specified directory. Returns the
//  number of posts loaded, or -1 if there was an error reading the directory.

int
hydra_ledger_load (hydra_ledger_t *self)
{
    //  Load a list of all post files in the posts directory
    zdir_t *dir = zdir_new ("posts", "-");
    zfile_t **files = zdir_flatten (dir);
    int nbr_posts = 0;
    
    //  Now check each post file, and load post IDs into posts_hash
    uint index;
    for (index = 0; files [index]; index++) {
        zfile_t *file = files [index];
        char *filename = zfile_filename (file, NULL);
        assert (memcmp (filename, "posts/", 6) == 0);
        hydra_post_t *post = hydra_post_load (filename + 6);
        if (post) {
            //  We track posts ? how? filename, ID? array?
            zhash_insert (self->posts_hash, hydra_post_id (post), NULL);
            hydra_post_destroy (&post);
            nbr_posts++;
        }
    }
    zdir_flatten_free (&files);
    zdir_destroy (&dir);
    return nbr_posts;
}


//  --------------------------------------------------------------------------
//  Selftest

int
hydra_ledger_test (bool verbose)
{
    printf (" * hydra_ledger: ");

    //  @selftest
    //  Simple create/destroy test
    zsys_dir_create (".hydra_test");
    zsys_dir_change (".hydra_test");

    hydra_post_t *post = hydra_post_new ("Test post 1");
    hydra_post_set_content (post, "Hello, World");
    hydra_post_save (post, "testpost");
    hydra_post_destroy (&post);
    
    hydra_ledger_t *ledger = hydra_ledger_new ();
    assert (ledger);
    int rc = hydra_ledger_load (ledger);
    assert (rc == 1);
    hydra_ledger_destroy (&ledger);

    //  Delete the test directory
    zsys_dir_change ("..");
    zdir_t *dir = zdir_new (".hydra_test", NULL);
    assert (dir);
    zdir_remove (dir, true);
    zdir_destroy (&dir);
    //  @end

    printf ("OK\n");
    return 0;
}
