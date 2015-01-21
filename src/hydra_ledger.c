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
    zhash_t *post_files;    //  Hash table maps post IDs to filenames
    char **posts_list;      //  Array of post IDs, oldest to newest
    size_t size;            //  Current size of posts list
    size_t max_size;        //  Maximum size of posts list (allocated)
    int sequence;           //  Number of posts created today
};

static void
s_have_new_post (hydra_ledger_t *self, hydra_post_t *post, char *filename)
{
    //  If the post is new then we store it, otherwise we get rid of it
    if (zhash_insert (self->post_files, hydra_post_ident (post), filename) == 0) {
        //  Store post ID in posts_list and bump size
        zsys_warning ("hydra_ledger: store post, ident=%s", hydra_post_ident (post));
        if (self->size == self->max_size - 1) {
            self->max_size *= 2;
            self->posts_list = (char **) realloc (
                self->posts_list, sizeof (char *) * self->max_size);
        }
        self->posts_list [self->size++] = strdup (hydra_post_ident (post));
    }
    else {
        zsys_warning ("hydra_ledger: duplicate post, ident=%s", hydra_post_ident (post));
        zsys_warning ("hydra_ledger: deleting filename=%s", filename);
        char *fullname = zsys_sprintf ("posts/%s", filename);
        zsys_file_delete (fullname);
        zstr_free (&fullname);
    }
}


//  --------------------------------------------------------------------------
//  Create a new ledger instance. You must specify the directory that holds
//  post files.

hydra_ledger_t *
hydra_ledger_new (void)
{
    hydra_ledger_t *self = (hydra_ledger_t *) zmalloc (sizeof (hydra_ledger_t));
    if (self) {
        self->max_size = 256;      //  Arbitrary, this is expanded on demand
        self->posts_list = (char **) malloc (sizeof (char *) * self->max_size);
    }
    if (self->posts_list)
        self->post_files = zhash_new ();
    if (self->post_files)
        zhash_autofree (self->post_files);
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
        zhash_destroy (&self->post_files);
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
//  Return ledger size, i.e. number of posts stored in the ledger.

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
    today [10] = 0;

    zrex_t *rex = zrex_new ("^(\\d\\d\\d\\d-\\d\\d-\\d\\d)\\((\\d+)\\)$");
    assert (rex && zrex_valid (rex));

    //  Now check each post file, and load post IDs into posts list
    uint index;
    for (index = 0; files [index]; index++) {
        zfile_t *file = files [index];
        char *filename = zfile_filename (file, NULL);
        assert (memcmp (filename, "posts/", 6) == 0);
        filename += 6;
        hydra_post_t *post = hydra_post_load (filename);
        if (post) {
            if (zrex_matches (rex, filename) && streq (zrex_hit (rex, 1), today)) {
                int sequence = atoi (zrex_hit (rex, 2));
                if (self->sequence < sequence)
                    self->sequence = sequence;
            }
            s_have_new_post (self, post, filename);
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
hydra_ledger_store (hydra_ledger_t *self, hydra_post_t **post_p)
{
    assert (self);
    assert (post_p && *post_p);
    hydra_post_t *post = *post_p;

    //  Get yyyy-mm-dd string for filename
    char *today = zclock_timestr ();
    today [10] = 0;
    char *filename = zsys_sprintf ("%s(%08d)", today, ++self->sequence);
    zsys_info ("hydrad: store new post filename=%s bytes=%zd",
               filename, hydra_post_content_size (post));
    int rc = hydra_post_save (post, filename);
    s_have_new_post (self, post, filename);
    hydra_post_destroy (post_p);
    *post_p = NULL;
    zstr_free (&filename);
    zstr_free (&today);
    return rc;
}


//  --------------------------------------------------------------------------
//  Return post at specified index; if the index does not refer to a valid
//  post, returns NULL.

hydra_post_t *
hydra_ledger_fetch (hydra_ledger_t *self, int index)
{
    hydra_post_t *post = NULL;
    if (index >= 0 && index < self->size) {
        char *post_ident = self->posts_list [index];
        char *filename = (char *) zhash_lookup (self->post_files, post_ident);
        if (filename)
            post = hydra_post_load (filename);
    }
    return post;
}


//  --------------------------------------------------------------------------
//  Lookup post in ledger and return post index (0 .. size - 1); if the post
//  does not exist, returns -1.

int
hydra_ledger_index (hydra_ledger_t *self, const char *post_ident)
{
    if (*post_ident) {
        uint post_nbr;
        for (post_nbr = 0; post_nbr < self->size; post_nbr++) {
            if (streq (post_ident, self->posts_list [post_nbr]))
                return post_nbr;
        }
    }
    return -1;
}


//  --------------------------------------------------------------------------
//  Selftest

int
hydra_ledger_test (bool verbose)
{
    printf (" * hydra_ledger: ");
    if (verbose)
        printf ("\n");

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
    char *post_ident = strdup (hydra_post_ident (post));
    hydra_ledger_store (ledger, &post);
    assert (hydra_ledger_size (ledger) == 2);

    //  Test index method
    assert (hydra_ledger_index (ledger, post_ident) == 1);
    assert (hydra_ledger_index (ledger, "") == -1);
    assert (hydra_ledger_index (ledger, "no such id") == -1);

    //  Test we can load a post via the ledger
    post = hydra_ledger_fetch (ledger, 1);
    assert (post);
    assert (streq (post_ident, hydra_post_ident (post)));
    free (post_ident);

    //  Done, destroy ledger
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
