package org.zproto;

import static org.junit.Assert.*;
import org.junit.Test;
import org.zeromq.ZMQ;
import org.zeromq.ZMQ.Socket;
import org.zeromq.ZFrame;
import org.zeromq.ZContext;

public class TestHydraProto
{
    @Test
    public void testHydraProto ()
    {
        System.out.printf (" * hydra_proto: ");

        //  Simple create/destroy test
        HydraProto self = new HydraProto (0);
        assert (self != null);
        self.destroy ();

        //  Create pair of sockets we can send through
        ZContext ctx = new ZContext ();
        assert (ctx != null);

        Socket output = ctx.createSocket (ZMQ.DEALER);
        assert (output != null);
        output.bind ("inproc://selftest");
        Socket input = ctx.createSocket (ZMQ.ROUTER);
        assert (input != null);
        input.connect ("inproc://selftest");

        //  Encode/send/decode and verify each message type

        self = new HydraProto (HydraProto.HELLO);
        self.setIdentity ("Life is short but Now lasts for ever");
        self.setNickname ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        assertEquals (self.identity (), "Life is short but Now lasts for ever");
        assertEquals (self.nickname (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraProto (HydraProto.HELLO_OK);
        self.setPost_Id ("Life is short but Now lasts for ever");
        self.setIdentity ("Life is short but Now lasts for ever");
        self.setNickname ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        assertEquals (self.post_id (), "Life is short but Now lasts for ever");
        assertEquals (self.identity (), "Life is short but Now lasts for ever");
        assertEquals (self.nickname (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraProto (HydraProto.GET_POST);
        self.setPost_Id ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        assertEquals (self.post_id (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraProto (HydraProto.GET_POST_OK);
        self.setPost_Id ("Life is short but Now lasts for ever");
        self.setReply_To ("Life is short but Now lasts for ever");
        self.setPrevious ("Life is short but Now lasts for ever");
        self.setTags ("Life is short but Now lasts for ever");
        self.setTimestamp ("Life is short but Now lasts for ever");
        self.setDigest ("Life is short but Now lasts for ever");
        self.setType ("Life is short but Now lasts for ever");
        self.setContent ("Captcha Diem".getBytes());
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        assertEquals (self.post_id (), "Life is short but Now lasts for ever");
        assertEquals (self.reply_to (), "Life is short but Now lasts for ever");
        assertEquals (self.previous (), "Life is short but Now lasts for ever");
        assertEquals (self.tags (), "Life is short but Now lasts for ever");
        assertEquals (self.timestamp (), "Life is short but Now lasts for ever");
        assertEquals (self.digest (), "Life is short but Now lasts for ever");
        assertEquals (self.type (), "Life is short but Now lasts for ever");
        assertTrue (java.util.Arrays.equals("Captcha Diem".getBytes(), self.content ()));
        self.destroy ();

        self = new HydraProto (HydraProto.GET_TAGS);
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        self.destroy ();

        self = new HydraProto (HydraProto.GET_TAGS_OK);
        self.setTags ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        assertEquals (self.tags (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraProto (HydraProto.GET_TAG);
        self.setTag ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        assertEquals (self.tag (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraProto (HydraProto.GET_TAG_OK);
        self.setTag ("Life is short but Now lasts for ever");
        self.setPost_Id ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        assertEquals (self.tag (), "Life is short but Now lasts for ever");
        assertEquals (self.post_id (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraProto (HydraProto.GOODBYE);
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        self.destroy ();

        self = new HydraProto (HydraProto.GOODBYE_OK);
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        self.destroy ();

        self = new HydraProto (HydraProto.ERROR);
        self.setStatus ((byte) 123);
        self.setReason ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        assertEquals (self.status (), 123);
        assertEquals (self.reason (), "Life is short but Now lasts for ever");
        self.destroy ();

        ctx.destroy ();
        System.out.printf ("OK\n");
    }
}
