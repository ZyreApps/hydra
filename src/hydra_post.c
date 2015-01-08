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
//  Return the post content size

size_t
hydra_post_content_size (hydra_post_t *self)
{
    assert (self);
    return self->content_size;
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
//  if the file was unreadable. The location may be relative to the current
//  working, or it may be an absolute file path.

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
//  Save the post to disk under the specified filename. Returns 0 if OK, -1
//  if the file could not be created. Posts are always stored in the "posts"
//  subdirectory of the current working directory.

int
hydra_post_save (hydra_post_t *self, const char *filename)
{
    assert (self);
    assert (filename);

    //  Creeate subdirectories if necessary
    zsys_dir_create ("posts");
    zsys_dir_create ("posts/blobs");

    //  If post content hasn't yet been serialised, write it to disk in the
    //  blobs directory and set the location property to point to it.
    if (self->content) {
        assert (!self->location);
        self->location = zsys_sprintf ("posts/blobs/%s", self->digest);
        FILE *output = fopen (self->location, "wb");
        zchunk_write (self->content, output);
        zchunk_destroy (&self->content);
        fclose (output);
    }
    zconfig_t *root = zconfig_new ("root", NULL);
    zconfig_put (root, "/post/id", hydra_post_id (self));
    zconfig_put (root, "/post/subject", self->subject);
    zconfig_put (root, "/post/timestamp", self->timestamp);
    zconfig_put (root, "/post/parent-id", self->parent_id);
    zconfig_put (root, "/post/mime-type", self->mime_type);
    zconfig_put (root, "/post/digest", self->digest);
    zconfig_put (root, "/post/location", self->location);
    zconfig_putf (root, "/post/content-size", "%ld", self->content_size);
    zconfig_savef (root, "posts/%s", filename);
    zconfig_destroy (&root);
    return 0;
}


//  --------------------------------------------------------------------------
//  Load post from the specified filename. Posts are always read from the
//  "posts" subdirectory of the current working directory. Returns a new post
//  instance if the file could be loaded, else returns null.

hydra_post_t *
hydra_post_load (const char *filename)
{
    assert (filename);
    zconfig_t *root = zconfig_loadf ("posts/%s", filename);
    if (!root)
        return NULL;            //  No such file

    hydra_post_t *self = NULL;
    char *id = zconfig_resolve (root, "/post/id", "");
    char *subject = zconfig_resolve (root, "/post/subject", NULL);
    char *timestamp = zconfig_resolve (root, "/post/timestamp", NULL);
    char *parent_id = zconfig_resolve (root, "/post/parent-id", "");
    char *mime_type = zconfig_resolve (root, "/post/mime-type", NULL);
    char *digest = zconfig_resolve (root, "/post/digest", NULL);
    char *location = zconfig_resolve (root, "/post/location", NULL);
    
    if (subject && timestamp && mime_type && digest && location
    && (strlen (id) == ID_SIZE)
    && (strlen (parent_id) == 0 || strlen (parent_id) == ID_SIZE)
    && (strlen (timestamp) == 20)
    && (strlen (digest) == ID_SIZE)) {
        self = hydra_post_new (subject);
        strcpy (self->id, id);
        strcpy (self->timestamp, timestamp);
        strcpy (self->parent_id, parent_id);
        self->mime_type = strdup (mime_type);
        self->location = strdup (location);
        strcpy (self->digest, digest);
        self->content_size = atoll (zconfig_resolve (root, "/post/content-size", "0"));
    }
    zconfig_destroy (&root);
    return self;
}


//  --------------------------------------------------------------------------
//  Fetch a chunk of content for the post. The caller specifies the size and
//  offset of the chunk. A size of 0 means all content, which will fail if
//  there is insufficient memory available. The caller must destroy the chunk
//  when finished with it.

zchunk_t *
hydra_post_fetch (hydra_post_t *self, size_t size, size_t offset)
{
    assert (self);

    if (self->content)
        return zchunk_dup (self->content);
    else {
        zfile_t *file = zfile_new (NULL, self->location);
        if (zfile_input (file) == 0) {
            zchunk_t *chunk = zfile_read (file, size, offset);
            zfile_destroy (&file);
            return chunk;
        }
    }
    return NULL;
}


//  --------------------------------------------------------------------------
//  Print the post file to stdout

void
hydra_post_print (hydra_post_t *self)
{
    assert (self);
    printf ("POST post-id: %s\n", self->id);
    printf ("     subject: %s\n", self->subject);
    printf ("   timestamp: %s\n", self->timestamp);
    printf ("   parent-id: %s\n", self->parent_id);
    printf ("   MIME-type: %s\n", self->mime_type);
    printf ("    location: %s\n", self->location);
    printf ("      digest: %s\n", self->digest);
    printf ("content-size: %zd\n", self->content_size);
}


//  --------------------------------------------------------------------------
//  Selftest

int
hydra_post_test (bool verbose)
{
    printf (" * hydra_post: ");
    if (verbose)
        printf ("\n");

    //  @selftest
    //  Simple create/destroy test
    zsys_dir_create (".hydra_test");
    zsys_dir_change (".hydra_test");
        
    hydra_post_t *post = hydra_post_new ("Test post");
    assert (post);
    hydra_post_set_content (post, "Hello, World");
    assert (streq (hydra_post_mime_type (post), "text/plain"));
    int rc = hydra_post_save (post, "testpost");
    assert (rc == 0);
    hydra_post_destroy (&post);

    post = hydra_post_load ("testpost");
    assert (post);
    assert (hydra_post_content_size (post) == 12);
    if (verbose)
        hydra_post_print (post);
    zchunk_t *chunk = hydra_post_fetch (
        post, hydra_post_content_size (post), 0);
    assert (chunk);
    assert (zchunk_size (chunk) == 12);
    zchunk_destroy (&chunk);
    hydra_post_destroy (&post);

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
