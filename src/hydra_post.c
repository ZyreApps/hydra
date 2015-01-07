/*  =========================================================================
    hydra_post - work with a single Hydra post

    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of zbroker, the ZeroMQ broker project.           
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

/*
@header
    Work with a Hydra post in memory.
@discuss
@end
*/

#include "hydra_classes.h"

#define ID_SIZE     40          //  Size of SHA1 digest as text string

//  Structure of our class

struct _hydra_post_t {
    char id [ID_SIZE + 1];      //  SHA1 (subject ":" timestamp ":" parent_id
                                //        ":" mime_type ":" digest)
    char *subject;              //  Post subject
    char timestamp [21];        //  Timestamp yyyy-mm-ddThh:mm:ssZ
    char parent_id [ID_SIZE + 1];   //  Parent ID, if any
    char *mime_type;            //  MIME type
    char *location;             //  Content filename, or
    zchunk_t *content;          //  Content chunk
    char digest [ID_SIZE + 1];  //  Content SHA1 digest
    size_t content_size;        //  Content size
    //  Directory constants
    const char *blob_dir;       //  For content blobs
    const char *post_dir;       //  For post properties
};


//  --------------------------------------------------------------------------
//  Create a new post

hydra_post_t *
hydra_post_new (char *subject)
{
    hydra_post_t *self = (hydra_post_t *) zmalloc (sizeof (hydra_post_t));
    if (self)
        self->subject = strdup (subject);
    if (self->subject) {
        time_t curtime = time (NULL);
        struct tm *utctime = gmtime (&curtime);
        strftime (self->timestamp, 21, "%Y-%m-%dT%H:%M:%SZ", utctime);
    }
    return self;
}


//  --------------------------------------------------------------------------
//  Destroy the post

void
hydra_post_destroy (hydra_post_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        hydra_post_t *self = *self_p;
        zchunk_destroy (&self->content);
        free (self->subject);
        free (self->mime_type);
        free (self->location);
        free (self);
        *self_p = NULL;
    }
}


//  --------------------------------------------------------------------------
//  Recalculate the post ID based on subject, timestamp, parent id, MIME
//  type, and content digest, and return post ID to caller.

const char *
hydra_post_id (hydra_post_t *self)
{
    assert (self);
    zdigest_t *digest = zdigest_new ();
    if (digest) {
        char *digest_text = zsys_sprintf ("%s:%s:%s:%s:%s",
            self->subject, self->timestamp, self->parent_id,
            self->mime_type? self->mime_type: "", self->digest);
        zdigest_update (digest, (byte *) digest_text, strlen (digest_text));
        assert (strlen (zdigest_string (digest)) == ID_SIZE);
        strcpy (self->id, zdigest_string (digest));
        zstr_free (&digest_text);
        zdigest_destroy (&digest);
    }
    return self->id;
}


//  --------------------------------------------------------------------------
//  Return the post subject, if set

const char *
hydra_post_subject (hydra_post_t *self)
{
    assert (self);
    return self->subject;
}


//  --------------------------------------------------------------------------
//  Return the post timestamp

const char *
hydra_post_timestamp (hydra_post_t *self)
{
    assert (self);
    return self->timestamp;
}


//  --------------------------------------------------------------------------
//  Return the post parent id, or empty string if not set

const char *
hydra_post_parent_id (hydra_post_t *self)
{
    assert (self);
    return self->parent_id;
}


//  --------------------------------------------------------------------------
//  Return the post MIME type, if set

const char *
hydra_post_mime_type (hydra_post_t *self)
{
    assert (self);
    return self->mime_type;
}


//  --------------------------------------------------------------------------
//  Set the post parent id, which must be a valid post ID

void
hydra_post_set_parent_id (hydra_post_t *self, const char *parent_id)
{
    assert (self);
    assert (strlen (parent_id) == 0 || strlen (parent_id) == ID_SIZE);
    strcpy (self->parent_id, parent_id);
}


//  --------------------------------------------------------------------------
//  Set the post MIME type

void
hydra_post_set_mime_type (hydra_post_t *self, const char *mime_type)
{
    assert (self);
    free (self->mime_type);
    self->mime_type = strdup (mime_type);
}


//  --------------------------------------------------------------------------
//  Set the post content to a text string. Recalculates the post digest from
//  from the new content value. Sets the MIME type to text/plain.

void
hydra_post_set_content (hydra_post_t *self, const char *content)
{
    assert (self);
    hydra_post_set_mime_type (self, "text/plain");
    hydra_post_set_data (self, content, strlen (content));
}


//  --------------------------------------------------------------------------
//  Set the post content to a chunk of data. Recalculates the post digest
//  from the chunk contents. Takes ownership of the chunk.

void
hydra_post_set_data (hydra_post_t *self, const void *data, size_t size)
{
    assert (self);
    zstr_free (&self->location);
    zchunk_destroy (&self->content);
    self->content = zchunk_new (data, size);
    strcpy (self->digest, zchunk_digest (self->content));
    self->content_size = zchunk_size (self->content);
}


//  --------------------------------------------------------------------------
//  Set the post content to point to a specified file. The file must exist.
//  Recalculates the post digest from the file contents. Returns 0 if OK, -1
//  if the file was unreadable.

int
hydra_post_set_file (hydra_post_t *self, const char *location)
{
    assert (self);
    free (self->location);

    int rc = 0;
    self->location = strdup (location);
    zchunk_destroy (&self->content);
    zfile_t *file = zfile_new (NULL, self->location);
    if (file && zfile_is_readable (file)) {
        self->content_size = zfile_cursize (file);
        strcpy (self->digest, zfile_digest (file));
    }
    else
        rc = -1;

    zfile_destroy (&file);
    return rc;
}


//  --------------------------------------------------------------------------
//  Set the posts directory, creates the directory if it doesn't exist.

void
hydra_post_set_post_dir (hydra_post_t *self, const char *post_dir)
{
    assert (self);
    self->post_dir = post_dir;
    zsys_dir_create (self->post_dir);
}


//  --------------------------------------------------------------------------
//  Set the blobs directory, creates the directory if it doesn't exist.

void
hydra_post_set_blob_dir (hydra_post_t *self, const char *blob_dir)
{
    assert (self);
    self->blob_dir = blob_dir;
    zsys_dir_create (self->blob_dir);
}


//  --------------------------------------------------------------------------
//  Save the post to disk under the specified filename. Returns 0 if OK, -1
//  if the file could not be created.

int
hydra_post_save (hydra_post_t *self, const char *filename)
{
    assert (self);

    //  If post content isn't yet serialised, write it to disk
    if (self->content) {
        assert (!self->location);
        if (self->blob_dir)
            self->location = zsys_sprintf ("%s/%s", self->blob_dir, self->digest);
        else
            self->location = strdup (self->digest);
        
        FILE *output = fopen (self->location, "wb");
        zchunk_write (self->content, output);
        zchunk_destroy (&self->content);
        fclose (output);
    }
    zconfig_t *config = zconfig_new ("root", NULL);
    zconfig_put (config, "/post/id", self->id);
    zconfig_put (config, "/post/subject", self->subject);
    zconfig_put (config, "/post/timestamp", self->timestamp);
    zconfig_put (config, "/post/parent-id", self->parent_id);
    zconfig_put (config, "/post/mime-type", self->mime_type);
    zconfig_put (config, "/post/digest", self->digest);
    zconfig_put (config, "/post/location", self->location);
    zconfig_putf (config, "/post/content-size", "%ld", self->content_size);
    if (self->post_dir)
        zconfig_savef (config, "%s/%s", self->post_dir, filename);
    else
        zconfig_save (config, filename);
    zconfig_destroy (&config);
    return -1;
}


//  --------------------------------------------------------------------------
//  Selftest

int
hydra_post_test (bool verbose)
{
    printf (" * hydra_post: ");

    //  @selftest
    //  Simple create/destroy test
    hydra_post_t *post = hydra_post_new ("Test post");
    assert (post);
    hydra_post_set_post_dir (post, ".hydra_post_test/posts");
    hydra_post_set_blob_dir (post, ".hydra_post_test/blobs");
    hydra_post_set_mime_type (post, "text/plain");
    hydra_post_set_content (post, "Hello, World");
    hydra_post_save (post, "testpost");
    hydra_post_destroy (&post);

    //  Delete all test files
    zdir_t *dir = zdir_new (".hydra_post_test", NULL);
    assert (dir);
    zdir_remove (dir, true);
    zdir_destroy (&dir);
    //  @end

    printf ("OK\n");
    return 0;
}
