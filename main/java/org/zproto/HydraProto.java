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

    HELLO - Open new connection
        identity            string      Client identity
        nickname            string      Client nickname

    HELLO_OK - Accept new connection and return most recent post, if any.
        post_id             string      Post identifier
        identity            string      Server identity
        nickname            string      Server nickname

    GET_POST - Fetch a given post's content
        post_id             string      Post identifier

    GET_POST_OK - Return a post's metadata and content
        post_id             string      Post identifier
        reply_to            string      Parent post, if any
        previous            string      Previous post, if any
        timestamp           string      Content date/time
        digest              string      Content digest
        type                string      Content type
        content             chunk       Content body

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
    public static final int GET_POST              = 3;
    public static final int GET_POST_OK           = 4;
    public static final int GOODBYE               = 5;
    public static final int GOODBYE_OK            = 6;
    public static final int ERROR                 = 7;

    //  Structure of our class
    private ZFrame routingId;           // Routing_id from ROUTER, if any
    private int id;                     //  HydraProto message ID
    private ByteBuffer needle;          //  Read/write pointer for serialization

    private String identity;
    private String nickname;
    private String post_id;
    private String reply_to;
    private String previous;
    private String timestamp;
    private String digest;
    private String type;
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
                self.post_id = self.getString ();
                self.identity = self.getString ();
                self.nickname = self.getString ();
                break;

            case GET_POST:
                self.post_id = self.getString ();
                break;

            case GET_POST_OK:
                self.post_id = self.getString ();
                self.reply_to = self.getString ();
                self.previous = self.getString ();
                self.timestamp = self.getString ();
                self.digest = self.getString ();
                self.type = self.getString ();
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
            //  post_id is a string with 1-byte length
            frameSize ++;
            frameSize += (post_id != null) ? post_id.length() : 0;
            //  identity is a string with 1-byte length
            frameSize ++;
            frameSize += (identity != null) ? identity.length() : 0;
            //  nickname is a string with 1-byte length
            frameSize ++;
            frameSize += (nickname != null) ? nickname.length() : 0;
            break;

        case GET_POST:
            //  post_id is a string with 1-byte length
            frameSize ++;
            frameSize += (post_id != null) ? post_id.length() : 0;
            break;

        case GET_POST_OK:
            //  post_id is a string with 1-byte length
            frameSize ++;
            frameSize += (post_id != null) ? post_id.length() : 0;
            //  reply_to is a string with 1-byte length
            frameSize ++;
            frameSize += (reply_to != null) ? reply_to.length() : 0;
            //  previous is a string with 1-byte length
            frameSize ++;
            frameSize += (previous != null) ? previous.length() : 0;
            //  timestamp is a string with 1-byte length
            frameSize ++;
            frameSize += (timestamp != null) ? timestamp.length() : 0;
            //  digest is a string with 1-byte length
            frameSize ++;
            frameSize += (digest != null) ? digest.length() : 0;
            //  type is a string with 1-byte length
            frameSize ++;
            frameSize += (type != null) ? type.length() : 0;
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
            if (post_id != null)
                putString (post_id);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (identity != null)
                putString (identity);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (nickname != null)
                putString (nickname);
            else
                putNumber1 ((byte) 0);      //  Empty string
            break;

        case GET_POST:
            if (post_id != null)
                putString (post_id);
            else
                putNumber1 ((byte) 0);      //  Empty string
            break;

        case GET_POST_OK:
            if (post_id != null)
                putString (post_id);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (reply_to != null)
                putString (reply_to);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (previous != null)
                putString (previous);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (timestamp != null)
                putString (timestamp);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (digest != null)
                putString (digest);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (type != null)
                putString (type);
            else
                putNumber1 ((byte) 0);      //  Empty string
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
        String post_id,
        String identity,
        String nickname)
    {
	sendHello_Ok (
		    output,
		    null,
		    post_id,
		    identity,
		    nickname);
    }

//  --------------------------------------------------------------------------
//  Send the HELLO_OK to a router socket in one step

    public static void sendHello_Ok (
        Socket output,
	ZFrame routingId,
        String post_id,
        String identity,
        String nickname)
    {
        HydraProto self = new HydraProto (HydraProto.HELLO_OK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setPost_Id (post_id);
        self.setIdentity (identity);
        self.setNickname (nickname);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the GET_POST to the socket in one step

    public static void sendGet_Post (
        Socket output,
        String post_id)
    {
	sendGet_Post (
		    output,
		    null,
		    post_id);
    }

//  --------------------------------------------------------------------------
//  Send the GET_POST to a router socket in one step

    public static void sendGet_Post (
        Socket output,
	ZFrame routingId,
        String post_id)
    {
        HydraProto self = new HydraProto (HydraProto.GET_POST);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setPost_Id (post_id);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the GET_POST_OK to the socket in one step

    public static void sendGet_Post_Ok (
        Socket output,
        String post_id,
        String reply_to,
        String previous,
        String timestamp,
        String digest,
        String type,
        byte[] content)
    {
	sendGet_Post_Ok (
		    output,
		    null,
		    post_id,
		    reply_to,
		    previous,
		    timestamp,
		    digest,
		    type,
		    content);
    }

//  --------------------------------------------------------------------------
//  Send the GET_POST_OK to a router socket in one step

    public static void sendGet_Post_Ok (
        Socket output,
	ZFrame routingId,
        String post_id,
        String reply_to,
        String previous,
        String timestamp,
        String digest,
        String type,
        byte[] content)
    {
        HydraProto self = new HydraProto (HydraProto.GET_POST_OK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setPost_Id (post_id);
        self.setReply_To (reply_to);
        self.setPrevious (previous);
        self.setTimestamp (timestamp);
        self.setDigest (digest);
        self.setType (type);
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
            copy.post_id = this.post_id;
            copy.identity = this.identity;
            copy.nickname = this.nickname;
        break;
        case GET_POST:
            copy.post_id = this.post_id;
        break;
        case GET_POST_OK:
            copy.post_id = this.post_id;
            copy.reply_to = this.reply_to;
            copy.previous = this.previous;
            copy.timestamp = this.timestamp;
            copy.digest = this.digest;
            copy.type = this.type;
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
            if (post_id != null)
                System.out.printf ("    post_id='%s'\n", post_id);
            else
                System.out.printf ("    post_id=\n");
            if (identity != null)
                System.out.printf ("    identity='%s'\n", identity);
            else
                System.out.printf ("    identity=\n");
            if (nickname != null)
                System.out.printf ("    nickname='%s'\n", nickname);
            else
                System.out.printf ("    nickname=\n");
            break;

        case GET_POST:
            System.out.println ("GET_POST:");
            if (post_id != null)
                System.out.printf ("    post_id='%s'\n", post_id);
            else
                System.out.printf ("    post_id=\n");
            break;

        case GET_POST_OK:
            System.out.println ("GET_POST_OK:");
            if (post_id != null)
                System.out.printf ("    post_id='%s'\n", post_id);
            else
                System.out.printf ("    post_id=\n");
            if (reply_to != null)
                System.out.printf ("    reply_to='%s'\n", reply_to);
            else
                System.out.printf ("    reply_to=\n");
            if (previous != null)
                System.out.printf ("    previous='%s'\n", previous);
            else
                System.out.printf ("    previous=\n");
            if (timestamp != null)
                System.out.printf ("    timestamp='%s'\n", timestamp);
            else
                System.out.printf ("    timestamp=\n");
            if (digest != null)
                System.out.printf ("    digest='%s'\n", digest);
            else
                System.out.printf ("    digest=\n");
            if (type != null)
                System.out.printf ("    type='%s'\n", type);
            else
                System.out.printf ("    type=\n");
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
    //  Get/set the post_id field

    public String post_id ()
    {
        return post_id;
    }

    public void setPost_Id (String format, Object ... args)
    {
        //  Format into newly allocated string
        post_id = String.format (format, args);
    }

    //  --------------------------------------------------------------------------
    //  Get/set the reply_to field

    public String reply_to ()
    {
        return reply_to;
    }

    public void setReply_To (String format, Object ... args)
    {
        //  Format into newly allocated string
        reply_to = String.format (format, args);
    }

    //  --------------------------------------------------------------------------
    //  Get/set the previous field

    public String previous ()
    {
        return previous;
    }

    public void setPrevious (String format, Object ... args)
    {
        //  Format into newly allocated string
        previous = String.format (format, args);
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
    //  Get/set the type field

    public String type ()
    {
        return type;
    }

    public void setType (String format, Object ... args)
    {
        //  Format into newly allocated string
        type = String.format (format, args);
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

