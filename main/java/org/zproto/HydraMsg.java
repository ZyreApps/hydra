/*  =========================================================================
    HydraMsg - The Hydra Protocol

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

    * The XML model used for this code generation: hydra_msg.xml
    * The code generation script that built this file: zproto_codec_c
    ************************************************************************
    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of zbroker, the ZeroMQ broker project.           
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/

/*  These are the HydraMsg messages:

    HELLO - Open new connection and ask for most recent post

    HELLO_OK - Return last post known for peer
        post_id             string      Post identifier

    GET_TAGS - Request list of tags known by peer

    GET_TAGS_OK - Return list of known tags
        tags                strings     List of known tags

    GET_TAG - Request last post for a given tag
        tag                 string      Name of tag

    GET_TAG_OK - Return last post for given tag
        post_id             string      Post identifier

    GET_POST - Fetch a given post
        post_id             string      Post identifier

    GET_POST_OK - Return a post details
        post_id             string      Post identifier
        reply_to            string      Parent post, if any
        previous            string      Previous post, if any
        tags                strings     Post tags
        timestamp           number 8    Post creation timestamp
        type                string      Content type
        content             msg         Content body

    GOODBYE - Close the connection politely

    GOODBYE_OK - Handshake a connection close

    INVALID - Command was invalid at this time

    FAILED - Command failed for some specific reason
        reason              string      Reason for failure
*/

package org.zproto;

import java.util.*;
import java.nio.ByteBuffer;

import org.zeromq.ZFrame;
import org.zeromq.ZMsg;
import org.zeromq.ZMQ;
import org.zeromq.ZMQ.Socket;

public class HydraMsg implements java.io.Closeable
{

    public static final int HELLO                 = 1;
    public static final int HELLO_OK              = 2;
    public static final int GET_TAGS              = 3;
    public static final int GET_TAGS_OK           = 4;
    public static final int GET_TAG               = 5;
    public static final int GET_TAG_OK            = 6;
    public static final int GET_POST              = 7;
    public static final int GET_POST_OK           = 8;
    public static final int GOODBYE               = 9;
    public static final int GOODBYE_OK            = 10;
    public static final int INVALID               = 11;
    public static final int FAILED                = 12;

    //  Structure of our class
    private ZFrame routingId;           // Routing_id from ROUTER, if any
    private int id;                     //  HydraMsg message ID
    private ByteBuffer needle;          //  Read/write pointer for serialization

    private String post_id;
    private List <String> tags;
    private String tag;
    private String reply_to;
    private String previous;
    private long timestamp;
    private String type;
    private ZMsg content;
    private String reason;

    public HydraMsg( int id )
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
    //  Receive and parse a HydraMsg from the socket. Returns new object or
    //  null if error. Will block if there's no message waiting.

    public static HydraMsg recv (Socket input)
    {
        assert (input != null);
        HydraMsg self = new HydraMsg (0);
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
                break;

            case HELLO_OK:
                self.post_id = self.getString ();
                break;

            case GET_TAGS:
                break;

            case GET_TAGS_OK:
                listSize = (int) self.getNumber4 ();
                self.tags = new ArrayList<String> ();
                while (listSize-- > 0) {
                    String string = self.getLongString ();
                    self.tags.add (string);
                }
                break;

            case GET_TAG:
                self.tag = self.getString ();
                break;

            case GET_TAG_OK:
                self.post_id = self.getString ();
                break;

            case GET_POST:
                self.post_id = self.getString ();
                break;

            case GET_POST_OK:
                self.post_id = self.getString ();
                self.reply_to = self.getString ();
                self.previous = self.getString ();
                listSize = (int) self.getNumber4 ();
                self.tags = new ArrayList<String> ();
                while (listSize-- > 0) {
                    String string = self.getLongString ();
                    self.tags.add (string);
                }
                self.timestamp = self.getNumber8 ();
                self.type = self.getString ();
                self.content = new ZMsg();
                if (input.hasReceiveMore ())
                    self.content.add(ZFrame.recvFrame (input));
                break;

            case GOODBYE:
                break;

            case GOODBYE_OK:
                break;

            case INVALID:
                break;

            case FAILED:
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
    //  Send the HydraMsg to the socket, and destroy it

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
            break;

        case HELLO_OK:
            //  post_id is a string with 1-byte length
            frameSize ++;
            frameSize += (post_id != null) ? post_id.length() : 0;
            break;

        case GET_TAGS:
            break;

        case GET_TAGS_OK:
            //  tags is an array of strings
            frameSize += 4;
            if (tags != null) {
                for (String value : tags) {
                    frameSize += 4;
                    frameSize += value.length ();
                }
            }
            break;

        case GET_TAG:
            //  tag is a string with 1-byte length
            frameSize ++;
            frameSize += (tag != null) ? tag.length() : 0;
            break;

        case GET_TAG_OK:
            //  post_id is a string with 1-byte length
            frameSize ++;
            frameSize += (post_id != null) ? post_id.length() : 0;
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
            //  tags is an array of strings
            frameSize += 4;
            if (tags != null) {
                for (String value : tags) {
                    frameSize += 4;
                    frameSize += value.length ();
                }
            }
            //  timestamp is a 8-byte integer
            frameSize += 8;
            //  type is a string with 1-byte length
            frameSize ++;
            frameSize += (type != null) ? type.length() : 0;
            break;

        case GOODBYE:
            break;

        case GOODBYE_OK:
            break;

        case INVALID:
            break;

        case FAILED:
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
            break;

        case HELLO_OK:
            if (post_id != null)
                putString (post_id);
            else
                putNumber1 ((byte) 0);      //  Empty string
            break;

        case GET_TAGS:
            break;

        case GET_TAGS_OK:
            if (tags != null) {
                putNumber4 (tags.size ());
                for (String value : tags) {
                    putLongString (value);
                }
            }
            else
                putNumber4 (0);      //  Empty string array
            break;

        case GET_TAG:
            if (tag != null)
                putString (tag);
            else
                putNumber1 ((byte) 0);      //  Empty string
            break;

        case GET_TAG_OK:
            if (post_id != null)
                putString (post_id);
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
            if (tags != null) {
                putNumber4 (tags.size ());
                for (String value : tags) {
                    putLongString (value);
                }
            }
            else
                putNumber4 (0);      //  Empty string array
            putNumber8 (timestamp);
            if (type != null)
                putString (type);
            else
                putNumber1 ((byte) 0);      //  Empty string
            break;

        case GOODBYE:
            break;

        case GOODBYE_OK:
            break;

        case INVALID:
            break;

        case FAILED:
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
        case GET_POST_OK:
            if( content == null )
                content = new ZMsg();
            for (ZFrame contentPart : content) {
                msg.add(contentPart);
            }
            break;
        }
        //  Destroy HydraMsg object
        msg.send(socket);
        destroy ();
        return true;
    }


//  --------------------------------------------------------------------------
//  Send the HELLO to the socket in one step

    public static void sendHello (
        Socket output)
    {
	sendHello (
		    output,
		    null);
    }

//  --------------------------------------------------------------------------
//  Send the HELLO to a router socket in one step

    public static void sendHello (
        Socket output,
	ZFrame routingId)
    {
        HydraMsg self = new HydraMsg (HydraMsg.HELLO);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the HELLO_OK to the socket in one step

    public static void sendHello_Ok (
        Socket output,
        String post_id)
    {
	sendHello_Ok (
		    output,
		    null,
		    post_id);
    }

//  --------------------------------------------------------------------------
//  Send the HELLO_OK to a router socket in one step

    public static void sendHello_Ok (
        Socket output,
	ZFrame routingId,
        String post_id)
    {
        HydraMsg self = new HydraMsg (HydraMsg.HELLO_OK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setPost_Id (post_id);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the GET_TAGS to the socket in one step

    public static void sendGet_Tags (
        Socket output)
    {
	sendGet_Tags (
		    output,
		    null);
    }

//  --------------------------------------------------------------------------
//  Send the GET_TAGS to a router socket in one step

    public static void sendGet_Tags (
        Socket output,
	ZFrame routingId)
    {
        HydraMsg self = new HydraMsg (HydraMsg.GET_TAGS);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the GET_TAGS_OK to the socket in one step

    public static void sendGet_Tags_Ok (
        Socket output,
        List <String> tags)
    {
	sendGet_Tags_Ok (
		    output,
		    null,
		    tags);
    }

//  --------------------------------------------------------------------------
//  Send the GET_TAGS_OK to a router socket in one step

    public static void sendGet_Tags_Ok (
        Socket output,
	ZFrame routingId,
        List <String> tags)
    {
        HydraMsg self = new HydraMsg (HydraMsg.GET_TAGS_OK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setTags (new ArrayList <String> (tags));
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the GET_TAG to the socket in one step

    public static void sendGet_Tag (
        Socket output,
        String tag)
    {
	sendGet_Tag (
		    output,
		    null,
		    tag);
    }

//  --------------------------------------------------------------------------
//  Send the GET_TAG to a router socket in one step

    public static void sendGet_Tag (
        Socket output,
	ZFrame routingId,
        String tag)
    {
        HydraMsg self = new HydraMsg (HydraMsg.GET_TAG);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setTag (tag);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the GET_TAG_OK to the socket in one step

    public static void sendGet_Tag_Ok (
        Socket output,
        String post_id)
    {
	sendGet_Tag_Ok (
		    output,
		    null,
		    post_id);
    }

//  --------------------------------------------------------------------------
//  Send the GET_TAG_OK to a router socket in one step

    public static void sendGet_Tag_Ok (
        Socket output,
	ZFrame routingId,
        String post_id)
    {
        HydraMsg self = new HydraMsg (HydraMsg.GET_TAG_OK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setPost_Id (post_id);
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
        HydraMsg self = new HydraMsg (HydraMsg.GET_POST);
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
        List <String> tags,
        long timestamp,
        String type,
        ZMsg content)
    {
	sendGet_Post_Ok (
		    output,
		    null,
		    post_id,
		    reply_to,
		    previous,
		    tags,
		    timestamp,
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
        List <String> tags,
        long timestamp,
        String type,
        ZMsg content)
    {
        HydraMsg self = new HydraMsg (HydraMsg.GET_POST_OK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setPost_Id (post_id);
        self.setReply_To (reply_to);
        self.setPrevious (previous);
        self.setTags (new ArrayList <String> (tags));
        self.setTimestamp (timestamp);
        self.setType (type);
        self.setContent (content.duplicate ());
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
        HydraMsg self = new HydraMsg (HydraMsg.GOODBYE);
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
        HydraMsg self = new HydraMsg (HydraMsg.GOODBYE_OK);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the INVALID to the socket in one step

    public static void sendInvalid (
        Socket output)
    {
	sendInvalid (
		    output,
		    null);
    }

//  --------------------------------------------------------------------------
//  Send the INVALID to a router socket in one step

    public static void sendInvalid (
        Socket output,
	ZFrame routingId)
    {
        HydraMsg self = new HydraMsg (HydraMsg.INVALID);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the FAILED to the socket in one step

    public static void sendFailed (
        Socket output,
        String reason)
    {
	sendFailed (
		    output,
		    null,
		    reason);
    }

//  --------------------------------------------------------------------------
//  Send the FAILED to a router socket in one step

    public static void sendFailed (
        Socket output,
	ZFrame routingId,
        String reason)
    {
        HydraMsg self = new HydraMsg (HydraMsg.FAILED);
        if (routingId != null)
        {
	        self.setRoutingId (routingId);
        }
        self.setReason (reason);
        self.send (output);
    }


    //  --------------------------------------------------------------------------
    //  Duplicate the HydraMsg message

    public HydraMsg dup ()
    {
        HydraMsg copy = new HydraMsg (this.id);
        if (this.routingId != null)
            copy.routingId = this.routingId.duplicate ();
        switch (this.id) {
        case HELLO:
        break;
        case HELLO_OK:
            copy.post_id = this.post_id;
        break;
        case GET_TAGS:
        break;
        case GET_TAGS_OK:
            copy.tags = new ArrayList <String> (this.tags);
        break;
        case GET_TAG:
            copy.tag = this.tag;
        break;
        case GET_TAG_OK:
            copy.post_id = this.post_id;
        break;
        case GET_POST:
            copy.post_id = this.post_id;
        break;
        case GET_POST_OK:
            copy.post_id = this.post_id;
            copy.reply_to = this.reply_to;
            copy.previous = this.previous;
            copy.tags = new ArrayList <String> (this.tags);
            copy.timestamp = this.timestamp;
            copy.type = this.type;
        break;
        case GOODBYE:
        break;
        case GOODBYE_OK:
        break;
        case INVALID:
        break;
        case FAILED:
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
            break;

        case HELLO_OK:
            System.out.println ("HELLO_OK:");
            if (post_id != null)
                System.out.printf ("    post_id='%s'\n", post_id);
            else
                System.out.printf ("    post_id=\n");
            break;

        case GET_TAGS:
            System.out.println ("GET_TAGS:");
            break;

        case GET_TAGS_OK:
            System.out.println ("GET_TAGS_OK:");
            System.out.printf ("    tags={");
            if (tags != null) {
                for (String value : tags) {
                    System.out.printf (" '%s'", value);
                }
            }
            System.out.printf (" }\n");
            break;

        case GET_TAG:
            System.out.println ("GET_TAG:");
            if (tag != null)
                System.out.printf ("    tag='%s'\n", tag);
            else
                System.out.printf ("    tag=\n");
            break;

        case GET_TAG_OK:
            System.out.println ("GET_TAG_OK:");
            if (post_id != null)
                System.out.printf ("    post_id='%s'\n", post_id);
            else
                System.out.printf ("    post_id=\n");
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
            System.out.printf ("    tags={");
            if (tags != null) {
                for (String value : tags) {
                    System.out.printf (" '%s'", value);
                }
            }
            System.out.printf (" }\n");
            System.out.printf ("    timestamp=%d\n", (long)timestamp);
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

        case INVALID:
            System.out.println ("INVALID:");
            break;

        case FAILED:
            System.out.println ("FAILED:");
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
    //  Get/set the hydra_msg id

    public int id ()
    {
        return id;
    }

    public void setId (int id)
    {
        this.id = id;
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
    //  Iterate through the tags field, and append a tags value

    public List <String> tags ()
    {
        return tags;
    }

    public void appendTags (String format, Object ... args)
    {
        //  Format into newly allocated string

        String string = String.format (format, args);
        //  Attach string to list
        if (tags == null)
            tags = new ArrayList <String> ();
        tags.add (string);
    }

    public void setTags (List <String> value)
    {
        tags = new ArrayList (value);
    }

    //  --------------------------------------------------------------------------
    //  Get/set the tag field

    public String tag ()
    {
        return tag;
    }

    public void setTag (String format, Object ... args)
    {
        //  Format into newly allocated string
        tag = String.format (format, args);
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

    public long timestamp ()
    {
        return timestamp;
    }

    public void setTimestamp (long timestamp)
    {
        this.timestamp = timestamp;
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

    public ZMsg content ()
    {
        return content;
    }

    //  Takes ownership of supplied frame
    public void setContent (ZMsg frame)
    {
        if (content != null)
            content.destroy ();
        content = frame;
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

