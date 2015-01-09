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
    char **posts_list;      //  Array of post IDs, oldest to newest
    size_t size;            //  Current size of posts list
    size_t max_size;        //  Maximum size of posts list (allocated)
    int post_seq;           //  Number of posts created today
};

static void
s_have_new_post (hydra_ledger_t *self, hydra_post_t *post)
{
    //  Hash table looks up post id to item in posts_list
    zhash_insert (self->posts_hash, hydra_post_id (post), &self->posts_list [self->size]);
    
    //  Store post ID in posts_list and bump size
    if (self->size == self->max_size - 1) {
        self->max_size *= 2;
        self->posts_list = (char **) realloc (
            self->posts_list, sizeof (char *) * self->max_size);
    }
    self->posts_list [self->size++] = strdup (hydra_post_id (post));
}


//  --------------------------------------------------------------------------
//  Create a new ledger instance. You must specify the directory that holds
//  post files.

hydra_ledger_t *
hydra_ledger_new (void)
{
    hydra_ledger_t *self = (hydra_ledger_t *) zmalloc (sizeof (hydra_ledger_t));
    if (self)
        self->posts_hash = zhash_new ();
    if (self->posts_hash) {
        self->max_size = 256;      //  Arbitrary, this is expanded on demand
        self->posts_list = (char **) malloc (sizeof (char *) * self->max_size);
    }
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
        //  Free list of post IDs
        uint post_nbr;
        for (post_nbr = 0; post_nbr < self->size; post_nbr++)
            free (self->posts_list [post_nbr]);
        free (self->posts_list);
        //  Free object instance
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Return ledger size.

size_t
hydra_ledger_size (hydra_ledger_t *self)
{
    assert (self);
    return self->size;
}


//  --------------------------------------------------------------------------
//  Load the ledger data from disk, from the specified directory. Returns the
//  number of posts loaded, or -1 if there was an error reading the directory.
//  Not valid if the ledger already has posts loaded.

int
hydra_ledger_load (hydra_ledger_t *self)
{
    assert (self);
    assert (self->size == 0);
    
    //  Load a list of all post files in the posts directory
    zdir_t *dir = zdir_new ("posts", "-");
    zfile_t **files = zdir_flatten (dir);

    //  Get yyyy-mm-dd string for checking today's files
    char *today = zclock_timestr ();
    today [11] = 0;
    
    zrex_t *rex = zrex_new ("^(\\d\\d\\d\\d-\\d\\d-\\d\\d)\\((\\d+)\\)$");
    assert (rex && zrex_valid (rex));

    //  Now check each post file, and load post IDs into posts_hash
    uint index;
    for (index = 0; files [index]; index++) {
        zfile_t *file = files [index];
        char *filename = zfile_filename (file, NULL);
        assert (memcmp (filename, "posts/", 6) == 0);
        filename += 6;
        hydra_post_t *post = hydra_post_load (filename);
        if (post) {
            if (zrex_matches (rex, filename) && streq (zrex_hit (rex, 1), today)) {
                int post_seq = atoi (zrex_hit (rex, 2));
                if (self->post_seq < post_seq)
                    self->post_seq = post_seq;
            }
            s_have_new_post (self, post);
            hydra_post_destroy (&post);
        }
    }
    zstr_free (&today);
    zrex_destroy (&rex);
    zdir_flatten_free (&files);
    zdir_destroy (&dir);
    return self->size;
}


//  --------------------------------------------------------------------------
//  Save a post via the ledger. This saves the post to disk, adds the post to
//  the ledger, and then destroys the post. Use in place of hydra_post_save
//  to ensure post filenames are correctly generated.

int
hydra_ledger_save_post (hydra_ledger_t *self, hydra_post_t **post_p)
{
    assert (self);
    assert (post_p && *post_p);
    
    //  Get yyyy-mm-dd string for filename
    char *today = zclock_timestr ();
    today [10] = 0;
    char *filename = zsys_sprintf ("%s(%08d)", today, ++self->post_seq);
    int rc = hydra_post_save (*post_p, filename);
    s_have_new_post (self, *post_p);
    hydra_post_destroy (post_p);
    *post_p = NULL;
    zstr_free (&filename);
    zstr_free (&today);
    return rc;
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

    //  Create one pre-existing post 
    hydra_post_t *post = hydra_post_new ("Test post 1");
    hydra_post_set_content (post, "Hello, World");
    hydra_post_save (post, "20150108_00000001");
    hydra_post_destroy (&post);
    
    //  Load ledger, it will have the one post
    hydra_ledger_t *ledger = hydra_ledger_new ();
    assert (ledger);
    assert (hydra_ledger_size (ledger) == 0);
    int rc = hydra_ledger_load (ledger);
    assert (rc == 1);
    assert (hydra_ledger_size (ledger) == 1);
    
    //  Now create second post and save via ledger
    post = hydra_post_new ("Test post 2");
    hydra_post_set_content (post, "Hello, Again");
    hydra_ledger_save_post (ledger, &post);
    assert (hydra_ledger_size (ledger) == 2);

    //  Done, destroy ledger
    hydra_ledger_destroy (&ledger);

    //  Delete the test directory
    zsys_dir_change ("..");
    zdir_t *dir = zdir_new (".hydra_test", NULL);
    assert (dir);
//     zdir_remove (dir, true);
    zdir_destroy (&dir);
    //  @end

    printf ("OK\n");
    return 0;
}
