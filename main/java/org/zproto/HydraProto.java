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

    NEXT_OLDER - Client requests next post that is older than the specified post ID.
If the post ID is "HEAD", fetches the newest post that the server has.
        ident               string      Client's oldest post ID

    NEXT_NEWER - Client requests next post that is newer than the specified post ID.
If the post ID is "TAIL", fetches the oldest post that the server has.
        ident               string      Client's newest post ID

    NEXT_OK - Server returns a post identity to the client. This command does not
provide all metadata, only the post identity string. Clients can then
filter out posts they already have.
        ident               string      Post identifier

    NEXT_EMPTY - Server signals that it has no (more) posts for the client.

    META - Client requests the metadata for the current post. A META command only
makes sense after a NEXT-OLDER or NEXT-NEWER with a successful NEXT-OK
from the server.

    META_OK - Server returns the metadata for the current post (as returned by NEXT-OK).
        subject             longstr     Subject line
        timestamp           string      Post creation timestamp
        parent_id           string      Parent post ID, if any
        digest              string      Content SHA1 digest
        mime_type           string      Content MIME type
        content_size        number 8    Content size, octets

    CHUNK - Client fetches a chunk of content data from the server, for the current
post (as returned by NEXT-OK).
        offset              number 8    Chunk offset in file
        octets              number 4    Maximum chunk size to fetch

    CHUNK_OK - Return a chunk of post content.
        offset              number 8    Chunk offset in file
        content             chunk       Content data chunk

    PING - Client pings the server. Server replies with PING-OK, or ERROR with status
COMMAND-INVALID if the client is not recognized (e.g. after a server restart
or network recovery).

    PING_OK - Server replies to a client ping.

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
    public static final int NEXT_OLDER            = 3;
    public static final int NEXT_NEWER            = 4;
    public static final int NEXT_OK               = 5;
    public static final int NEXT_EMPTY            = 6;
    public static final int META                  = 7;
    public static final int META_OK               = 8;
    public static final int CHUNK                 = 9;
    public static final int CHUNK_OK              = 10;
    public static final int PING                  = 11;
    public static final int PING_OK               = 12;
    public static final int GOODBYE               = 13;
    public static final int GOODBYE_OK            = 14;
    public static final int ERROR                 = 15;

    //  Structure of our class
    private ZFrame routingId;           // Routing_id from ROUTER, if any
    private int id;                     //  HydraProto message ID
    private ByteBuffer needle;          //  Read/write pointer for serialization

    private String identity;
    private String nickname;
    private String ident;
    private String subject;
    private String timestamp;
    private String parent_id;
    private String digest;
    private String mime_type;
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

            case NEXT_OLDER:
                self.ident = self.getString ();
                break;

            case NEXT_NEWER:
                self.ident = self.getString ();
                break;

            case NEXT_OK:
                self.ident = self.getString ();
                break;

            case NEXT_EMPTY:
                break;

            case META:
                break;

            case META_OK:
                self.subject = self.getLongString ();
                self.timestamp = self.getString ();
                self.parent_id = self.getString ();
                self.digest = self.getString ();
                self.mime_type = self.getString ();
                self.content_size = self.getNumber8 ();
                break;

            case CHUNK:
                self.offset = self.getNumber8 ();
                self.octets = self.getNumber4 ();
                break;

            case CHUNK_OK:
                self.offset = self.getNumber8 ();
                self.content = self.getBlock((int) self.getNumber4());
                break;

            case PING:
                break;

            case PING_OK:
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

        case NEXT_OLDER:
            //  ident is a string with 1-byte length
            frameSize ++;
            frameSize += (ident != null) ? ident.length() : 0;
            break;

        case NEXT_NEWER:
            //  ident is a string with 1-byte length
            frameSize ++;
            frameSize += (ident != null) ? ident.length() : 0;
            break;

        case NEXT_OK:
            //  ident is a string with 1-byte length
            frameSize ++;
            frameSize += (ident != null) ? ident.length() : 0;
            break;

        case NEXT_EMPTY:
            break;

        case META:
            break;

        case META_OK:
            //  subject is a long string with 4-byte length
            frameSize += 4;
            frameSize += (subject != null) ? subject.length() : 0;
            //  timestamp is a string with 1-byte length
            frameSize ++;
            frameSize += (timestamp != null) ? timestamp.length() : 0;
            //  parent_id is a string with 1-byte length
            frameSize ++;
            frameSize += (parent_id != null) ? parent_id.length() : 0;
            //  digest is a string with 1-byte length
            frameSize ++;
            frameSize += (digest != null) ? digest.length() : 0;
            //  mime_type is a string with 1-byte length
            frameSize ++;
            frameSize += (mime_type != null) ? mime_type.length() : 0;
            //  content_size is a 8-byte integer
            frameSize += 8;
            break;

        case CHUNK:
            //  offset is a 8-byte integer
            frameSize += 8;
            //  octets is a 4-byte integer
            frameSize += 4;
            break;

        case CHUNK_OK:
            //  offset is a 8-byte integer
            frameSize += 8;
            //  content is a chunk with 4-byte length
            frameSize += 4;
            frameSize += (content != null) ? content.length : 0;
            break;

        case PING:
            break;

        case PING_OK:
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

        case NEXT_OLDER:
            if (ident != null)
                putString (ident);
            else
                putNumber1 ((byte) 0);      //  Empty string
            break;

        case NEXT_NEWER:
            if (ident != null)
                putString (ident);
            else
                putNumber1 ((byte) 0);      //  Empty string
            break;

        case NEXT_OK:
            if (ident != null)
                putString (ident);
            else
                putNumber1 ((byte) 0);      //  Empty string
            break;

        case NEXT_EMPTY:
            break;

        case META:
            break;

        case META_OK:
            if (subject != null)
                putLongString (subject);
            else
                putNumber4 (0);      //  Empty string
            if (timestamp != null)
                putString (timestamp);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (parent_id != null)
                putString (parent_id);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (digest != null)
                putString (digest);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (mime_type != null)
                putString (mime_type);
            else
                putNumber1 ((byte) 0);      //  Empty string
            putNumber8 (content_size);
            break;

        case CHUNK:
            putNumber8 (offset);
            putNumber4 (octets);
            break;

        case CHUNK_OK:
            putNumber8 (offset);
              if(content != null) {
                  putNumber4(content.length);
                  needle.put(content, 0, content.length);
              } else {
                  putNumber4(0);
              }
            break;

        case PING:
            break;

        case PING_OK:
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
//  Send the NEXT_OLDER to the socket in one step

    public static void sendNext_Older (
        Socket output,
        String ident)
    {
	sendNext_Older (
		    output,
		    null,
		    ident);
    }

//  --------------------------------------------------------------------------
//  Send the NEXT_OLDER to a router socket in one step

    public static void sendNext_Older (
        Socket output,
	ZFrame routingId,
        String ident)
    {
        HydraProto self = new HydraProto (HydraProto.NEXT_OLDER);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setIdent (ident);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the NEXT_NEWER to the socket in one step

    public static void sendNext_Newer (
        Socket output,
        String ident)
    {
	sendNext_Newer (
		    output,
		    null,
		    ident);
    }

//  --------------------------------------------------------------------------
//  Send the NEXT_NEWER to a router socket in one step

    public static void sendNext_Newer (
        Socket output,
	ZFrame routingId,
        String ident)
    {
        HydraProto self = new HydraProto (HydraProto.NEXT_NEWER);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setIdent (ident);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the NEXT_OK to the socket in one step

    public static void sendNext_Ok (
        Socket output,
        String ident)
    {
	sendNext_Ok (
		    output,
		    null,
		    ident);
    }

//  --------------------------------------------------------------------------
//  Send the NEXT_OK to a router socket in one step

    public static void sendNext_Ok (
        Socket output,
	ZFrame routingId,
        String ident)
    {
        HydraProto self = new HydraProto (HydraProto.NEXT_OK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setIdent (ident);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the NEXT_EMPTY to the socket in one step

    public static void sendNext_Empty (
        Socket output)
    {
	sendNext_Empty (
		    output,
		    null);
    }

//  --------------------------------------------------------------------------
//  Send the NEXT_EMPTY to a router socket in one step

    public static void sendNext_Empty (
        Socket output,
	ZFrame routingId)
    {
        HydraProto self = new HydraProto (HydraProto.NEXT_EMPTY);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the META to the socket in one step

    public static void sendMeta (
        Socket output)
    {
	sendMeta (
		    output,
		    null);
    }

//  --------------------------------------------------------------------------
//  Send the META to a router socket in one step

    public static void sendMeta (
        Socket output,
	ZFrame routingId)
    {
        HydraProto self = new HydraProto (HydraProto.META);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the META_OK to the socket in one step

    public static void sendMeta_Ok (
        Socket output,
        String subject,
        String timestamp,
        String parent_id,
        String digest,
        String mime_type,
        long content_size)
    {
	sendMeta_Ok (
		    output,
		    null,
		    subject,
		    timestamp,
		    parent_id,
		    digest,
		    mime_type,
		    content_size);
    }

//  --------------------------------------------------------------------------
//  Send the META_OK to a router socket in one step

    public static void sendMeta_Ok (
        Socket output,
	ZFrame routingId,
        String subject,
        String timestamp,
        String parent_id,
        String digest,
        String mime_type,
        long content_size)
    {
        HydraProto self = new HydraProto (HydraProto.META_OK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setSubject (subject);
        self.setTimestamp (timestamp);
        self.setParent_Id (parent_id);
        self.setDigest (digest);
        self.setMime_Type (mime_type);
        self.setContent_Size (content_size);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the CHUNK to the socket in one step

    public static void sendChunk (
        Socket output,
        long offset,
        long octets)
    {
	sendChunk (
		    output,
		    null,
		    offset,
		    octets);
    }

//  --------------------------------------------------------------------------
//  Send the CHUNK to a router socket in one step

    public static void sendChunk (
        Socket output,
	ZFrame routingId,
        long offset,
        long octets)
    {
        HydraProto self = new HydraProto (HydraProto.CHUNK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setOffset (offset);
        self.setOctets (octets);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the CHUNK_OK to the socket in one step

    public static void sendChunk_Ok (
        Socket output,
        long offset,
        byte[] content)
    {
	sendChunk_Ok (
		    output,
		    null,
		    offset,
		    content);
    }

//  --------------------------------------------------------------------------
//  Send the CHUNK_OK to a router socket in one step

    public static void sendChunk_Ok (
        Socket output,
	ZFrame routingId,
        long offset,
        byte[] content)
    {
        HydraProto self = new HydraProto (HydraProto.CHUNK_OK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setOffset (offset);
        self.setContent (content);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the PING to the socket in one step

    public static void sendPing (
        Socket output)
    {
	sendPing (
		    output,
		    null);
    }

//  --------------------------------------------------------------------------
//  Send the PING to a router socket in one step

    public static void sendPing (
        Socket output,
	ZFrame routingId)
    {
        HydraProto self = new HydraProto (HydraProto.PING);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the PING_OK to the socket in one step

    public static void sendPing_Ok (
        Socket output)
    {
	sendPing_Ok (
		    output,
		    null);
    }

//  --------------------------------------------------------------------------
//  Send the PING_OK to a router socket in one step

    public static void sendPing_Ok (
        Socket output,
	ZFrame routingId)
    {
        HydraProto self = new HydraProto (HydraProto.PING_OK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
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
        case NEXT_OLDER:
            copy.ident = this.ident;
        break;
        case NEXT_NEWER:
            copy.ident = this.ident;
        break;
        case NEXT_OK:
            copy.ident = this.ident;
        break;
        case NEXT_EMPTY:
        break;
        case META:
        break;
        case META_OK:
            copy.subject = this.subject;
            copy.timestamp = this.timestamp;
            copy.parent_id = this.parent_id;
            copy.digest = this.digest;
            copy.mime_type = this.mime_type;
            copy.content_size = this.content_size;
        break;
        case CHUNK:
            copy.offset = this.offset;
            copy.octets = this.octets;
        break;
        case CHUNK_OK:
            copy.offset = this.offset;
            copy.content = this.content;
        break;
        case PING:
        break;
        case PING_OK:
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

        case NEXT_OLDER:
            System.out.println ("NEXT_OLDER:");
            if (ident != null)
                System.out.printf ("    ident='%s'\n", ident);
            else
                System.out.printf ("    ident=\n");
            break;

        case NEXT_NEWER:
            System.out.println ("NEXT_NEWER:");
            if (ident != null)
                System.out.printf ("    ident='%s'\n", ident);
            else
                System.out.printf ("    ident=\n");
            break;

        case NEXT_OK:
            System.out.println ("NEXT_OK:");
            if (ident != null)
                System.out.printf ("    ident='%s'\n", ident);
            else
                System.out.printf ("    ident=\n");
            break;

        case NEXT_EMPTY:
            System.out.println ("NEXT_EMPTY:");
            break;

        case META:
            System.out.println ("META:");
            break;

        case META_OK:
            System.out.println ("META_OK:");
            if (subject != null)
                System.out.printf ("    subject='%s'\n", subject);
            else
                System.out.printf ("    subject=\n");
            if (timestamp != null)
                System.out.printf ("    timestamp='%s'\n", timestamp);
            else
                System.out.printf ("    timestamp=\n");
            if (parent_id != null)
                System.out.printf ("    parent_id='%s'\n", parent_id);
            else
                System.out.printf ("    parent_id=\n");
            if (digest != null)
                System.out.printf ("    digest='%s'\n", digest);
            else
                System.out.printf ("    digest=\n");
            if (mime_type != null)
                System.out.printf ("    mime_type='%s'\n", mime_type);
            else
                System.out.printf ("    mime_type=\n");
            System.out.printf ("    content_size=%d\n", (long)content_size);
            break;

        case CHUNK:
            System.out.println ("CHUNK:");
            System.out.printf ("    offset=%d\n", (long)offset);
            System.out.printf ("    octets=%d\n", (long)octets);
            break;

        case CHUNK_OK:
            System.out.println ("CHUNK_OK:");
            System.out.printf ("    offset=%d\n", (long)offset);
            break;

        case PING:
            System.out.println ("PING:");
            break;

        case PING_OK:
            System.out.println ("PING_OK:");
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
    //  Get/set the ident field

    public String ident ()
    {
        return ident;
    }

    public void setIdent (String format, Object ... args)
    {
        //  Format into newly allocated string
        ident = String.format (format, args);
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
    //  Get/set the parent_id field

    public String parent_id ()
    {
        return parent_id;
    }

    public void setParent_Id (String format, Object ... args)
    {
        //  Format into newly allocated string
        parent_id = String.format (format, args);
    }

    //  --------------------------------------------------------------------------
    //  Get/set the digest field

    public String digest ()
    {
        return digest;
    }

    public void setDigest (String format, Object ... args)
    {
        //  Format into newly allocated string
        digest = String.format (format, args);
    }

    //  --------------------------------------------------------------------------
    //  Get/set the mime_type field

    public String mime_type ()
    {
        return mime_type;
    }

    public void setMime_Type (String format, Object ... args)
    {
        //  Format into newly allocated string
        mime_type = String.format (format, args);
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

