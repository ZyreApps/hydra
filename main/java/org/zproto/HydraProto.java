/*  =========================================================================
    HydraProto - The Hydra Protocol

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

    * The XML model used for this code generation: hydra_proto.xml
    * The code generation script that built this file: zproto_codec_c
    ************************************************************************
    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of zbroker, the ZeroMQ broker project.           
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

/*  These are the HydraProto messages:

    HELLO - Open new connection, provide client credentials.
        identity            string      Client identity
        nickname            string      Client nickname

    HELLO_OK - Accept new connection, provide server credentials.
        identity            string      Server identity
        nickname            string      Server nickname

    STATUS - Client requests server status update, telling server the oldest and
newest post that it knows for that server. If the client never
received any posts from the server, these fields are empty.
        oldest              string      Oldest post
        newest              string      Newest post

    STATUS_OK - Server tells client how many posts it has, older and newer than the
range the client already knows.
        older               number 4    Number of older posts
        newer               number 4    Newest of newer posts

    HEADER - Client requests a post from the server, requesting either an older post
(previous to the oldest post it already has), a newer post (following the
newest post it has), or a fresh post (server's latest post, ignoring all
status).
        which               number 1    Which post to fetch

    HEADER_OK - Return a post's metadata.
        identifier          string      Post identifier
        subject             longstr     Subject line
        timestamp           string      Post creation timestamp
        parent_post         string      Parent post ID, if any
        content_digest      string      Content digest
        content_type        string      Content type
        content_size        number 8    Content size, octets

    FETCH - Client fetches a chunk of content data from the server. This command
always applies to the post returned by a HEADER-OK.
        offset              number 8    File offset in content
        octets              number 4    Number of octets to fetch

    FETCH_OK - Return a chunk of post content.
        offset              number 8    File offset in content
        octets              number 4    Number of octets to fetch
        content             chunk       Content data chunk

    GOODBYE - Close the connection politely

    GOODBYE_OK - Handshake a connection close

    ERROR - Command failed for some specific reason
        status              number 2    3-digit status code
        reason              string      Printable explanation
*/

package org.zproto;

import java.util.*;
import java.nio.ByteBuffer;

import org.zeromq.ZFrame;
import org.zeromq.ZMsg;
import org.zeromq.ZMQ;
import org.zeromq.ZMQ.Socket;

public class HydraProto implements java.io.Closeable
{
    public static final int HYDRA_PROTO_FETCH_OLDER         = 1;
    public static final int HYDRA_PROTO_FETCH_NEWER         = 2;
    public static final int HYDRA_PROTO_FETCH_FRESH         = 3;
    public static final int HYDRA_PROTO_SUCCESS             = 200;
    public static final int HYDRA_PROTO_STORED              = 201;
    public static final int HYDRA_PROTO_DELIVERED           = 202;
    public static final int HYDRA_PROTO_NOT_DELIVERED       = 300;
    public static final int HYDRA_PROTO_CONTENT_TOO_LARGE   = 301;
    public static final int HYDRA_PROTO_TIMEOUT_EXPIRED     = 302;
    public static final int HYDRA_PROTO_CONNECTION_REFUSED  = 303;
    public static final int HYDRA_PROTO_RESOURCE_LOCKED     = 400;
    public static final int HYDRA_PROTO_ACCESS_REFUSED      = 401;
    public static final int HYDRA_PROTO_NOT_FOUND           = 404;
    public static final int HYDRA_PROTO_COMMAND_INVALID     = 500;
    public static final int HYDRA_PROTO_NOT_IMPLEMENTED     = 501;
    public static final int HYDRA_PROTO_INTERNAL_ERROR      = 502;

    public static final int HELLO                 = 1;
    public static final int HELLO_OK              = 2;
    public static final int STATUS                = 3;
    public static final int STATUS_OK             = 4;
    public static final int HEADER                = 5;
    public static final int HEADER_OK             = 6;
    public static final int FETCH                 = 7;
    public static final int FETCH_OK              = 8;
    public static final int GOODBYE               = 9;
    public static final int GOODBYE_OK            = 10;
    public static final int ERROR                 = 11;

    //  Structure of our class
    private ZFrame routingId;           // Routing_id from ROUTER, if any
    private int id;                     //  HydraProto message ID
    private ByteBuffer needle;          //  Read/write pointer for serialization

    private String identity;
    private String nickname;
    private String oldest;
    private String newest;
    private long older;
    private long newer;
    private int which;
    private String identifier;
    private String subject;
    private String timestamp;
    private String parent_post;
    private String content_digest;
    private String content_type;
    private long content_size;
    private long offset;
    private long octets;
    private byte[] content;
    private int status;
    private String reason;

    public HydraProto( int id )
    {
        this.id = id;
    }

    public void destroy()
    {
        close();
    }

    @Override
    public void close()
    {
        //  Destroy frame fields
    }
    //  --------------------------------------------------------------------------
    //  Network data encoding macros


    //  Put a 1-byte number to the frame
    private final void putNumber1 (int value)
    {
        needle.put ((byte) value);
    }

    //  Get a 1-byte number to the frame
    //  then make it unsigned
    private int getNumber1 ()
    {
        int value = needle.get ();
        if (value < 0)
            value = (0xff) & value;
        return value;
    }

    //  Put a 2-byte number to the frame
    private final void putNumber2 (int value)
    {
        needle.putShort ((short) value);
    }

    //  Get a 2-byte number to the frame
    private int getNumber2 ()
    {
        int value = needle.getShort ();
        if (value < 0)
            value = (0xffff) & value;
        return value;
    }

    //  Put a 4-byte number to the frame
    private final void putNumber4 (long value)
    {
        needle.putInt ((int) value);
    }

    //  Get a 4-byte number to the frame
    //  then make it unsigned
    private long getNumber4 ()
    {
        long value = needle.getInt ();
        if (value < 0)
            value = (0xffffffff) & value;
        return value;
    }

    //  Put a 8-byte number to the frame
    public void putNumber8 (long value)
    {
        needle.putLong (value);
    }

    //  Get a 8-byte number to the frame
    public long getNumber8 ()
    {
        return needle.getLong ();
    }


    //  Put a block to the frame
    private void putBlock (byte [] value, int size)
    {
        needle.put (value, 0, size);
    }

    private byte [] getBlock (int size)
    {
        byte [] value = new byte [size];
        needle.get (value);

        return value;
    }

    //  Put a string to the frame
    public void putString (String value)
    {
        needle.put ((byte) value.length ());
        needle.put (value.getBytes());
    }

    //  Get a string from the frame
    public String getString ()
    {
        int size = getNumber1 ();
        byte [] value = new byte [size];
        needle.get (value);

        return new String (value);
    }

        //  Put a string to the frame
    public void putLongString (String value)
    {
        needle.putInt (value.length ());
        needle.put (value.getBytes());
    }

    //  Get a string from the frame
    public String getLongString ()
    {
        long size = getNumber4 ();
        byte [] value = new byte [(int) size];
        needle.get (value);

        return new String (value);
    }
    //  --------------------------------------------------------------------------
    //  Receive and parse a HydraProto from the socket. Returns new object or
    //  null if error. Will block if there's no message waiting.

    public static HydraProto recv (Socket input)
    {
        assert (input != null);
        HydraProto self = new HydraProto (0);
        ZFrame frame = null;

        try {
            //  Read valid message frame from socket; we loop over any
            //  garbage data we might receive from badly-connected peers
            while (true) {
                //  If we're reading from a ROUTER socket, get routingId
                if (input.getType () == ZMQ.ROUTER) {
                    self.routingId = ZFrame.recvFrame (input);
                    if (self.routingId == null)
                        return null;         //  Interrupted
                    if (!self.routingId.hasData())
                        return null;         //  Empty Frame (eg recv-timeout)
                    if (!input.hasReceiveMore ())
                        throw new IllegalArgumentException ();
                }
                //  Read and parse command in frame
                frame = ZFrame.recvFrame (input);
                if (frame == null)
                    return null;             //  Interrupted

                //  Get and check protocol signature
                self.needle = ByteBuffer.wrap (frame.getData ());
                int signature = self.getNumber2 ();
                if (signature == (0xAAA0 | 0))
                    break;                  //  Valid signature

                //  Protocol assertion, drop message
                while (input.hasReceiveMore ()) {
                    frame.destroy ();
                    frame = ZFrame.recvFrame (input);
                }
                frame.destroy ();
            }

            //  Get message id, which is first byte in frame
            self.id = self.getNumber1 ();
            int listSize;
            int hashSize;

            switch (self.id) {
            case HELLO:
                self.identity = self.getString ();
                self.nickname = self.getString ();
                break;

            case HELLO_OK:
                self.identity = self.getString ();
                self.nickname = self.getString ();
                break;

            case STATUS:
                self.oldest = self.getString ();
                self.newest = self.getString ();
                break;

            case STATUS_OK:
                self.older = self.getNumber4 ();
                self.newer = self.getNumber4 ();
                break;

            case HEADER:
                self.which = self.getNumber1 ();
                break;

            case HEADER_OK:
                self.identifier = self.getString ();
                self.subject = self.getLongString ();
                self.timestamp = self.getString ();
                self.parent_post = self.getString ();
                self.content_digest = self.getString ();
                self.content_type = self.getString ();
                self.content_size = self.getNumber8 ();
                break;

            case FETCH:
                self.offset = self.getNumber8 ();
                self.octets = self.getNumber4 ();
                break;

            case FETCH_OK:
                self.offset = self.getNumber8 ();
                self.octets = self.getNumber4 ();
                self.content = self.getBlock((int) self.getNumber4());
                break;

            case GOODBYE:
                break;

            case GOODBYE_OK:
                break;

            case ERROR:
                self.status = self.getNumber2 ();
                self.reason = self.getString ();
                break;

            default:
                throw new IllegalArgumentException ();
            }

            return self;

        } catch (Exception e) {
            //  Error returns
            System.out.printf ("E: malformed message '%d'\n", self.id);
            self.destroy ();
            return null;
        } finally {
            if (frame != null)
                frame.destroy ();
        }
    }

    //  --------------------------------------------------------------------------
    //  Send the HydraProto to the socket, and destroy it

    public boolean send (Socket socket)
    {
        assert (socket != null);

        ZMsg msg = new ZMsg();
        //  If we're sending to a ROUTER, send the routingId first
        if (socket.getType () == ZMQ.ROUTER) {
            msg.add (routingId);
        }

        int frameSize = 2 + 1;          //  Signature and message ID
        switch (id) {
        case HELLO:
            //  identity is a string with 1-byte length
            frameSize ++;
            frameSize += (identity != null) ? identity.length() : 0;
            //  nickname is a string with 1-byte length
            frameSize ++;
            frameSize += (nickname != null) ? nickname.length() : 0;
            break;

        case HELLO_OK:
            //  identity is a string with 1-byte length
            frameSize ++;
            frameSize += (identity != null) ? identity.length() : 0;
            //  nickname is a string with 1-byte length
            frameSize ++;
            frameSize += (nickname != null) ? nickname.length() : 0;
            break;

        case STATUS:
            //  oldest is a string with 1-byte length
            frameSize ++;
            frameSize += (oldest != null) ? oldest.length() : 0;
            //  newest is a string with 1-byte length
            frameSize ++;
            frameSize += (newest != null) ? newest.length() : 0;
            break;

        case STATUS_OK:
            //  older is a 4-byte integer
            frameSize += 4;
            //  newer is a 4-byte integer
            frameSize += 4;
            break;

        case HEADER:
            //  which is a 1-byte integer
            frameSize += 1;
            break;

        case HEADER_OK:
            //  identifier is a string with 1-byte length
            frameSize ++;
            frameSize += (identifier != null) ? identifier.length() : 0;
            //  subject is a long string with 4-byte length
            frameSize += 4;
            frameSize += (subject != null) ? subject.length() : 0;
            //  timestamp is a string with 1-byte length
            frameSize ++;
            frameSize += (timestamp != null) ? timestamp.length() : 0;
            //  parent_post is a string with 1-byte length
            frameSize ++;
            frameSize += (parent_post != null) ? parent_post.length() : 0;
            //  content_digest is a string with 1-byte length
            frameSize ++;
            frameSize += (content_digest != null) ? content_digest.length() : 0;
            //  content_type is a string with 1-byte length
            frameSize ++;
            frameSize += (content_type != null) ? content_type.length() : 0;
            //  content_size is a 8-byte integer
            frameSize += 8;
            break;

        case FETCH:
            //  offset is a 8-byte integer
            frameSize += 8;
            //  octets is a 4-byte integer
            frameSize += 4;
            break;

        case FETCH_OK:
            //  offset is a 8-byte integer
            frameSize += 8;
            //  octets is a 4-byte integer
            frameSize += 4;
            //  content is a chunk with 4-byte length
            frameSize += 4;
            frameSize += (content != null) ? content.length : 0;
            break;

        case GOODBYE:
            break;

        case GOODBYE_OK:
            break;

        case ERROR:
            //  status is a 2-byte integer
            frameSize += 2;
            //  reason is a string with 1-byte length
            frameSize ++;
            frameSize += (reason != null) ? reason.length() : 0;
            break;

        default:
            System.out.printf ("E: bad message type '%d', not sent\n", id);
            assert (false);
        }
        //  Now serialize message into the frame
        ZFrame frame = new ZFrame (new byte [frameSize]);
        needle = ByteBuffer.wrap (frame.getData ());
        int frameFlags = 0;
        putNumber2 (0xAAA0 | 0);
        putNumber1 ((byte) id);

        switch (id) {
        case HELLO:
            if (identity != null)
                putString (identity);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (nickname != null)
                putString (nickname);
            else
                putNumber1 ((byte) 0);      //  Empty string
            break;

        case HELLO_OK:
            if (identity != null)
                putString (identity);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (nickname != null)
                putString (nickname);
            else
                putNumber1 ((byte) 0);      //  Empty string
            break;

        case STATUS:
            if (oldest != null)
                putString (oldest);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (newest != null)
                putString (newest);
            else
                putNumber1 ((byte) 0);      //  Empty string
            break;

        case STATUS_OK:
            putNumber4 (older);
            putNumber4 (newer);
            break;

        case HEADER:
            putNumber1 (which);
            break;

        case HEADER_OK:
            if (identifier != null)
                putString (identifier);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (subject != null)
                putLongString (subject);
            else
                putNumber4 (0);      //  Empty string
            if (timestamp != null)
                putString (timestamp);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (parent_post != null)
                putString (parent_post);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (content_digest != null)
                putString (content_digest);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (content_type != null)
                putString (content_type);
            else
                putNumber1 ((byte) 0);      //  Empty string
            putNumber8 (content_size);
            break;

        case FETCH:
            putNumber8 (offset);
            putNumber4 (octets);
            break;

        case FETCH_OK:
            putNumber8 (offset);
            putNumber4 (octets);
              if(content != null) {
                  putNumber4(content.length);
                  needle.put(content, 0, content.length);
              } else {
                  putNumber4(0);
              }
            break;

        case GOODBYE:
            break;

        case GOODBYE_OK:
            break;

        case ERROR:
            putNumber2 (status);
            if (reason != null)
                putString (reason);
            else
                putNumber1 ((byte) 0);      //  Empty string
            break;

        }
        //  Now send the data frame
        msg.add(frame);

        //  Now send any frame fields, in order
        switch (id) {
        }
        switch (id) {
        }
        //  Destroy HydraProto object
        msg.send(socket);
        destroy ();
        return true;
    }


//  --------------------------------------------------------------------------
//  Send the HELLO to the socket in one step

    public static void sendHello (
        Socket output,
        String identity,
        String nickname)
    {
	sendHello (
		    output,
		    null,
		    identity,
		    nickname);
    }

//  --------------------------------------------------------------------------
//  Send the HELLO to a router socket in one step

    public static void sendHello (
        Socket output,
	ZFrame routingId,
        String identity,
        String nickname)
    {
        HydraProto self = new HydraProto (HydraProto.HELLO);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setIdentity (identity);
        self.setNickname (nickname);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the HELLO_OK to the socket in one step

    public static void sendHello_Ok (
        Socket output,
        String identity,
        String nickname)
    {
	sendHello_Ok (
		    output,
		    null,
		    identity,
		    nickname);
    }

//  --------------------------------------------------------------------------
//  Send the HELLO_OK to a router socket in one step

    public static void sendHello_Ok (
        Socket output,
	ZFrame routingId,
        String identity,
        String nickname)
    {
        HydraProto self = new HydraProto (HydraProto.HELLO_OK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setIdentity (identity);
        self.setNickname (nickname);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the STATUS to the socket in one step

    public static void sendStatus (
        Socket output,
        String oldest,
        String newest)
    {
	sendStatus (
		    output,
		    null,
		    oldest,
		    newest);
    }

//  --------------------------------------------------------------------------
//  Send the STATUS to a router socket in one step

    public static void sendStatus (
        Socket output,
	ZFrame routingId,
        String oldest,
        String newest)
    {
        HydraProto self = new HydraProto (HydraProto.STATUS);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setOldest (oldest);
        self.setNewest (newest);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the STATUS_OK to the socket in one step

    public static void sendStatus_Ok (
        Socket output,
        long older,
        long newer)
    {
	sendStatus_Ok (
		    output,
		    null,
		    older,
		    newer);
    }

//  --------------------------------------------------------------------------
//  Send the STATUS_OK to a router socket in one step

    public static void sendStatus_Ok (
        Socket output,
	ZFrame routingId,
        long older,
        long newer)
    {
        HydraProto self = new HydraProto (HydraProto.STATUS_OK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setOlder (older);
        self.setNewer (newer);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the HEADER to the socket in one step

    public static void sendHeader (
        Socket output,
        int which)
    {
	sendHeader (
		    output,
		    null,
		    which);
    }

//  --------------------------------------------------------------------------
//  Send the HEADER to a router socket in one step

    public static void sendHeader (
        Socket output,
	ZFrame routingId,
        int which)
    {
        HydraProto self = new HydraProto (HydraProto.HEADER);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setWhich (which);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the HEADER_OK to the socket in one step

    public static void sendHeader_Ok (
        Socket output,
        String identifier,
        String subject,
        String timestamp,
        String parent_post,
        String content_digest,
        String content_type,
        long content_size)
    {
	sendHeader_Ok (
		    output,
		    null,
		    identifier,
		    subject,
		    timestamp,
		    parent_post,
		    content_digest,
		    content_type,
		    content_size);
    }

//  --------------------------------------------------------------------------
//  Send the HEADER_OK to a router socket in one step

    public static void sendHeader_Ok (
        Socket output,
	ZFrame routingId,
        String identifier,
        String subject,
        String timestamp,
        String parent_post,
        String content_digest,
        String content_type,
        long content_size)
    {
        HydraProto self = new HydraProto (HydraProto.HEADER_OK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setIdentifier (identifier);
        self.setSubject (subject);
        self.setTimestamp (timestamp);
        self.setParent_Post (parent_post);
        self.setContent_Digest (content_digest);
        self.setContent_Type (content_type);
        self.setContent_Size (content_size);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the FETCH to the socket in one step

    public static void sendFetch (
        Socket output,
        long offset,
        long octets)
    {
	sendFetch (
		    output,
		    null,
		    offset,
		    octets);
    }

//  --------------------------------------------------------------------------
//  Send the FETCH to a router socket in one step

    public static void sendFetch (
        Socket output,
	ZFrame routingId,
        long offset,
        long octets)
    {
        HydraProto self = new HydraProto (HydraProto.FETCH);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setOffset (offset);
        self.setOctets (octets);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the FETCH_OK to the socket in one step

    public static void sendFetch_Ok (
        Socket output,
        long offset,
        long octets,
        byte[] content)
    {
	sendFetch_Ok (
		    output,
		    null,
		    offset,
		    octets,
		    content);
    }

//  --------------------------------------------------------------------------
//  Send the FETCH_OK to a router socket in one step

    public static void sendFetch_Ok (
        Socket output,
	ZFrame routingId,
        long offset,
        long octets,
        byte[] content)
    {
        HydraProto self = new HydraProto (HydraProto.FETCH_OK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setOffset (offset);
        self.setOctets (octets);
        self.setContent (content);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the GOODBYE to the socket in one step

    public static void sendGoodbye (
        Socket output)
    {
	sendGoodbye (
		    output,
		    null);
    }

//  --------------------------------------------------------------------------
//  Send the GOODBYE to a router socket in one step

    public static void sendGoodbye (
        Socket output,
	ZFrame routingId)
    {
        HydraProto self = new HydraProto (HydraProto.GOODBYE);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the GOODBYE_OK to the socket in one step

    public static void sendGoodbye_Ok (
        Socket output)
    {
	sendGoodbye_Ok (
		    output,
		    null);
    }

//  --------------------------------------------------------------------------
//  Send the GOODBYE_OK to a router socket in one step

    public static void sendGoodbye_Ok (
        Socket output,
	ZFrame routingId)
    {
        HydraProto self = new HydraProto (HydraProto.GOODBYE_OK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the ERROR to the socket in one step

    public static void sendError (
        Socket output,
        int status,
        String reason)
    {
	sendError (
		    output,
		    null,
		    status,
		    reason);
    }

//  --------------------------------------------------------------------------
//  Send the ERROR to a router socket in one step

    public static void sendError (
        Socket output,
	ZFrame routingId,
        int status,
        String reason)
    {
        HydraProto self = new HydraProto (HydraProto.ERROR);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setStatus (status);
        self.setReason (reason);
        self.send (output);
    }


    //  --------------------------------------------------------------------------
    //  Duplicate the HydraProto message

    public HydraProto dup ()
    {
        HydraProto copy = new HydraProto (this.id);
        if (this.routingId != null)
            copy.routingId = this.routingId.duplicate ();
        switch (this.id) {
        case HELLO:
            copy.identity = this.identity;
            copy.nickname = this.nickname;
        break;
        case HELLO_OK:
            copy.identity = this.identity;
            copy.nickname = this.nickname;
        break;
        case STATUS:
            copy.oldest = this.oldest;
            copy.newest = this.newest;
        break;
        case STATUS_OK:
            copy.older = this.older;
            copy.newer = this.newer;
        break;
        case HEADER:
            copy.which = this.which;
        break;
        case HEADER_OK:
            copy.identifier = this.identifier;
            copy.subject = this.subject;
            copy.timestamp = this.timestamp;
            copy.parent_post = this.parent_post;
            copy.content_digest = this.content_digest;
            copy.content_type = this.content_type;
            copy.content_size = this.content_size;
        break;
        case FETCH:
            copy.offset = this.offset;
            copy.octets = this.octets;
        break;
        case FETCH_OK:
            copy.offset = this.offset;
            copy.octets = this.octets;
            copy.content = this.content;
        break;
        case GOODBYE:
        break;
        case GOODBYE_OK:
        break;
        case ERROR:
            copy.status = this.status;
            copy.reason = this.reason;
        break;
        }
        return copy;
    }


    //  --------------------------------------------------------------------------
    //  Print contents of message to stdout

    public void dump ()
    {
        switch (id) {
        case HELLO:
            System.out.println ("HELLO:");
            if (identity != null)
                System.out.printf ("    identity='%s'\n", identity);
            else
                System.out.printf ("    identity=\n");
            if (nickname != null)
                System.out.printf ("    nickname='%s'\n", nickname);
            else
                System.out.printf ("    nickname=\n");
            break;

        case HELLO_OK:
            System.out.println ("HELLO_OK:");
            if (identity != null)
                System.out.printf ("    identity='%s'\n", identity);
            else
                System.out.printf ("    identity=\n");
            if (nickname != null)
                System.out.printf ("    nickname='%s'\n", nickname);
            else
                System.out.printf ("    nickname=\n");
            break;

        case STATUS:
            System.out.println ("STATUS:");
            if (oldest != null)
                System.out.printf ("    oldest='%s'\n", oldest);
            else
                System.out.printf ("    oldest=\n");
            if (newest != null)
                System.out.printf ("    newest='%s'\n", newest);
            else
                System.out.printf ("    newest=\n");
            break;

        case STATUS_OK:
            System.out.println ("STATUS_OK:");
            System.out.printf ("    older=%d\n", (long)older);
            System.out.printf ("    newer=%d\n", (long)newer);
            break;

        case HEADER:
            System.out.println ("HEADER:");
            System.out.printf ("    which=%d\n", (long)which);
            break;

        case HEADER_OK:
            System.out.println ("HEADER_OK:");
            if (identifier != null)
                System.out.printf ("    identifier='%s'\n", identifier);
            else
                System.out.printf ("    identifier=\n");
            if (subject != null)
                System.out.printf ("    subject='%s'\n", subject);
            else
                System.out.printf ("    subject=\n");
            if (timestamp != null)
                System.out.printf ("    timestamp='%s'\n", timestamp);
            else
                System.out.printf ("    timestamp=\n");
            if (parent_post != null)
                System.out.printf ("    parent_post='%s'\n", parent_post);
            else
                System.out.printf ("    parent_post=\n");
            if (content_digest != null)
                System.out.printf ("    content_digest='%s'\n", content_digest);
            else
                System.out.printf ("    content_digest=\n");
            if (content_type != null)
                System.out.printf ("    content_type='%s'\n", content_type);
            else
                System.out.printf ("    content_type=\n");
            System.out.printf ("    content_size=%d\n", (long)content_size);
            break;

        case FETCH:
            System.out.println ("FETCH:");
            System.out.printf ("    offset=%d\n", (long)offset);
            System.out.printf ("    octets=%d\n", (long)octets);
            break;

        case FETCH_OK:
            System.out.println ("FETCH_OK:");
            System.out.printf ("    offset=%d\n", (long)offset);
            System.out.printf ("    octets=%d\n", (long)octets);
            break;

        case GOODBYE:
            System.out.println ("GOODBYE:");
            break;

        case GOODBYE_OK:
            System.out.println ("GOODBYE_OK:");
            break;

        case ERROR:
            System.out.println ("ERROR:");
            System.out.printf ("    status=%d\n", (long)status);
            if (reason != null)
                System.out.printf ("    reason='%s'\n", reason);
            else
                System.out.printf ("    reason=\n");
            break;

        }
    }


    //  --------------------------------------------------------------------------
    //  Get/set the message routing id

    public ZFrame routingId ()
    {
        return routingId;
    }

    public void setRoutingId (ZFrame routingId)
    {
        if (this.routingId != null)
            this.routingId.destroy ();
        this.routingId = routingId.duplicate ();
    }


    //  --------------------------------------------------------------------------
    //  Get/set the hydra_proto id

    public int id ()
    {
        return id;
    }

    public void setId (int id)
    {
        this.id = id;
    }

    //  --------------------------------------------------------------------------
    //  Get/set the identity field

    public String identity ()
    {
        return identity;
    }

    public void setIdentity (String format, Object ... args)
    {
        //  Format into newly allocated string
        identity = String.format (format, args);
    }

    //  --------------------------------------------------------------------------
    //  Get/set the nickname field

    public String nickname ()
    {
        return nickname;
    }

    public void setNickname (String format, Object ... args)
    {
        //  Format into newly allocated string
        nickname = String.format (format, args);
    }

    //  --------------------------------------------------------------------------
    //  Get/set the oldest field

    public String oldest ()
    {
        return oldest;
    }

    public void setOldest (String format, Object ... args)
    {
        //  Format into newly allocated string
        oldest = String.format (format, args);
    }

    //  --------------------------------------------------------------------------
    //  Get/set the newest field

    public String newest ()
    {
        return newest;
    }

    public void setNewest (String format, Object ... args)
    {
        //  Format into newly allocated string
        newest = String.format (format, args);
    }

    //  --------------------------------------------------------------------------
    //  Get/set the older field

    public long older ()
    {
        return older;
    }

    public void setOlder (long older)
    {
        this.older = older;
    }

    //  --------------------------------------------------------------------------
    //  Get/set the newer field

    public long newer ()
    {
        return newer;
    }

    public void setNewer (long newer)
    {
        this.newer = newer;
    }

    //  --------------------------------------------------------------------------
    //  Get/set the which field

    public int which ()
    {
        return which;
    }

    public void setWhich (int which)
    {
        this.which = which;
    }

    //  --------------------------------------------------------------------------
    //  Get/set the identifier field

    public String identifier ()
    {
        return identifier;
    }

    public void setIdentifier (String format, Object ... args)
    {
        //  Format into newly allocated string
        identifier = String.format (format, args);
    }

    //  --------------------------------------------------------------------------
    //  Get/set the subject field

    public String subject ()
    {
        return subject;
    }

    public void setSubject (String format, Object ... args)
    {
        //  Format into newly allocated string
        subject = String.format (format, args);
    }

    //  --------------------------------------------------------------------------
    //  Get/set the timestamp field

    public String timestamp ()
    {
        return timestamp;
    }

    public void setTimestamp (String format, Object ... args)
    {
        //  Format into newly allocated string
        timestamp = String.format (format, args);
    }

    //  --------------------------------------------------------------------------
    //  Get/set the parent_post field

    public String parent_post ()
    {
        return parent_post;
    }

    public void setParent_Post (String format, Object ... args)
    {
        //  Format into newly allocated string
        parent_post = String.format (format, args);
    }

    //  --------------------------------------------------------------------------
    //  Get/set the content_digest field

    public String content_digest ()
    {
        return content_digest;
    }

    public void setContent_Digest (String format, Object ... args)
    {
        //  Format into newly allocated string
        content_digest = String.format (format, args);
    }

    //  --------------------------------------------------------------------------
    //  Get/set the content_type field

    public String content_type ()
    {
        return content_type;
    }

    public void setContent_Type (String format, Object ... args)
    {
        //  Format into newly allocated string
        content_type = String.format (format, args);
    }

    //  --------------------------------------------------------------------------
    //  Get/set the content_size field

    public long content_size ()
    {
        return content_size;
    }

    public void setContent_Size (long content_size)
    {
        this.content_size = content_size;
    }

    //  --------------------------------------------------------------------------
    //  Get/set the offset field

    public long offset ()
    {
        return offset;
    }

    public void setOffset (long offset)
    {
        this.offset = offset;
    }

    //  --------------------------------------------------------------------------
    //  Get/set the octets field

    public long octets ()
    {
        return octets;
    }

    public void setOctets (long octets)
    {
        this.octets = octets;
    }

    //  --------------------------------------------------------------------------
    //  Get/set the content field

    public byte[] content ()
    {
        return content;
    }

    //  Takes ownership of supplied frame
    public void setContent (byte[] content)
    {
        this.content = content;
    }
    //  --------------------------------------------------------------------------
    //  Get/set the status field

    public int status ()
    {
        return status;
    }

    public void setStatus (int status)
    {
        this.status = status;
    }

    //  --------------------------------------------------------------------------
    //  Get/set the reason field

    public String reason ()
    {
        return reason;
    }

    public void setReason (String format, Object ... args)
    {
        //  Format into newly allocated string
        reason = String.format (format, args);
    }

}

