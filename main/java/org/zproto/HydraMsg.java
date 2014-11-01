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
        tags                string      List of known tags

    GET_TAG - Request last post for a given tag
        tag                 string      Name of tag

    GET_TAG_OK - Return last post for given tag
        tag                 string      Name of tag
        post_id             string      Post identifier

    GET_POST - Fetch a given post
        post_id             string      Post identifier

    GET_POST_OK - Return a post details
        post_id             string      Post identifier
        reply_to            string      Parent post, if any
        previous            string      Previous post, if any
        tags                string      Post tags
        timestamp           number 8    Post creation timestamp
        type                string      Content type
        content             string      Content body

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
    private String tags;
    private String tag;
    private String reply_to;
    private String previous;
    private long timestamp;
    private String type;
    private String content;
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
                self.tags = self.getString ();
                break;

            case GET_TAG:
                self.tag = self.getString ();
                break;

            case GET_TAG_OK:
                self.tag = self.getString ();
                self.post_id = self.getString ();
                break;

            case GET_POST:
                self.post_id = self.getString ();
                break;

            case GET_POST_OK:
                self.post_id = self.getString ();
                self.reply_to = self.getString ();
                self.previous = self.getString ();
                self.tags = self.getString ();
                self.timestamp = self.getNumber8 ();
                self.type = self.getString ();
                self.content = self.getString ();
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
            //  tags is a string with 1-byte length
            frameSize ++;
            frameSize += (tags != null) ? tags.length() : 0;
            break;

        case GET_TAG:
            //  tag is a string with 1-byte length
            frameSize ++;
            frameSize += (tag != null) ? tag.length() : 0;
            break;

        case GET_TAG_OK:
            //  tag is a string with 1-byte length
            frameSize ++;
            frameSize += (tag != null) ? tag.length() : 0;
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
            //  tags is a string with 1-byte length
            frameSize ++;
            frameSize += (tags != null) ? tags.length() : 0;
            //  timestamp is a 8-byte integer
            frameSize += 8;
            //  type is a string with 1-byte length
            frameSize ++;
            frameSize += (type != null) ? type.length() : 0;
            //  content is a string with 1-byte length
            frameSize ++;
            frameSize += (content != null) ? content.length() : 0;
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
            if (tags != null)
                putString (tags);
            else
                putNumber1 ((byte) 0);      //  Empty string
            break;

        case GET_TAG:
            if (tag != null)
                putString (tag);
            else
                putNumber1 ((byte) 0);      //  Empty string
            break;

        case GET_TAG_OK:
            if (tag != null)
                putString (tag);
            else
                putNumber1 ((byte) 0);      //  Empty string
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
            if (tags != null)
                putString (tags);
            else
                putNumber1 ((byte) 0);      //  Empty string
            putNumber8 (timestamp);
            if (type != null)
                putString (type);
            else
                putNumber1 ((byte) 0);      //  Empty string
            if (content != null)
                putString (content);
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
        HydraMsg self = new HydraMsg (HydraMsg.HELLO);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the HELLO_OK to the socket in one step

    public static void sendHello_Ok (
        Socket output,
        String post_id)
    {
        HydraMsg self = new HydraMsg (HydraMsg.HELLO_OK);
        self.setPost_Id (post_id);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the GET_TAGS to the socket in one step

    public static void sendGet_Tags (
        Socket output)
    {
        HydraMsg self = new HydraMsg (HydraMsg.GET_TAGS);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the GET_TAGS_OK to the socket in one step

    public static void sendGet_Tags_Ok (
        Socket output,
        String tags)
    {
        HydraMsg self = new HydraMsg (HydraMsg.GET_TAGS_OK);
        self.setTags (tags);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the GET_TAG to the socket in one step

    public static void sendGet_Tag (
        Socket output,
        String tag)
    {
        HydraMsg self = new HydraMsg (HydraMsg.GET_TAG);
        self.setTag (tag);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the GET_TAG_OK to the socket in one step

    public static void sendGet_Tag_Ok (
        Socket output,
        String tag,
        String post_id)
    {
        HydraMsg self = new HydraMsg (HydraMsg.GET_TAG_OK);
        self.setTag (tag);
        self.setPost_Id (post_id);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the GET_POST to the socket in one step

    public static void sendGet_Post (
        Socket output,
        String post_id)
    {
        HydraMsg self = new HydraMsg (HydraMsg.GET_POST);
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
        String tags,
        long timestamp,
        String type,
        String content)
    {
        HydraMsg self = new HydraMsg (HydraMsg.GET_POST_OK);
        self.setPost_Id (post_id);
        self.setReply_To (reply_to);
        self.setPrevious (previous);
        self.setTags (tags);
        self.setTimestamp (timestamp);
        self.setType (type);
        self.setContent (content);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the GOODBYE to the socket in one step

    public static void sendGoodbye (
        Socket output)
    {
        HydraMsg self = new HydraMsg (HydraMsg.GOODBYE);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the GOODBYE_OK to the socket in one step

    public static void sendGoodbye_Ok (
        Socket output)
    {
        HydraMsg self = new HydraMsg (HydraMsg.GOODBYE_OK);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the INVALID to the socket in one step

    public static void sendInvalid (
        Socket output)
    {
        HydraMsg self = new HydraMsg (HydraMsg.INVALID);
        self.send (output);
    }

//  --------------------------------------------------------------------------
//  Send the FAILED to the socket in one step

    public static void sendFailed (
        Socket output,
        String reason)
    {
        HydraMsg self = new HydraMsg (HydraMsg.FAILED);
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
            copy.tags = this.tags;
        break;
        case GET_TAG:
            copy.tag = this.tag;
        break;
        case GET_TAG_OK:
            copy.tag = this.tag;
            copy.post_id = this.post_id;
        break;
        case GET_POST:
            copy.post_id = this.post_id;
        break;
        case GET_POST_OK:
            copy.post_id = this.post_id;
            copy.reply_to = this.reply_to;
            copy.previous = this.previous;
            copy.tags = this.tags;
            copy.timestamp = this.timestamp;
            copy.type = this.type;
            copy.content = this.content;
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
            if (tags != null)
                System.out.printf ("    tags='%s'\n", tags);
            else
                System.out.printf ("    tags=\n");
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
            if (tag != null)
                System.out.printf ("    tag='%s'\n", tag);
            else
                System.out.printf ("    tag=\n");
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
            if (tags != null)
                System.out.printf ("    tags='%s'\n", tags);
            else
                System.out.printf ("    tags=\n");
            System.out.printf ("    timestamp=%d\n", (long)timestamp);
            if (type != null)
                System.out.printf ("    type='%s'\n", type);
            else
                System.out.printf ("    type=\n");
            if (content != null)
                System.out.printf ("    content='%s'\n", content);
            else
                System.out.printf ("    content=\n");
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
    //  Get/set the tags field

    public String tags ()
    {
        return tags;
    }

    public void setTags (String format, Object ... args)
    {
        //  Format into newly allocated string
        tags = String.format (format, args);
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

    public String content ()
    {
        return content;
    }

    public void setContent (String format, Object ... args)
    {
        //  Format into newly allocated string
        content = String.format (format, args);
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

