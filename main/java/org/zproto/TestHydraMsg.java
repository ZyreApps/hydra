package org.zproto;

import static org.junit.Assert.*;
import org.junit.Test;
import org.zeromq.ZMQ;
import org.zeromq.ZMQ.Socket;
import org.zeromq.ZFrame;
import org.zeromq.ZContext;

public class TestHydraMsg
{
    @Test
    public void testHydraMsg ()
    {
        System.out.printf (" * hydra_msg: ");

        //  Simple create/destroy test
        HydraMsg self = new HydraMsg (0);
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

        self = new HydraMsg (HydraMsg.HELLO);
        self.setAddress ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        assertEquals (self.address (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraMsg (HydraMsg.HELLO_OK);
        self.setPost_Id ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        assertEquals (self.post_id (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraMsg (HydraMsg.GET_POST);
        self.setPost_Id ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        assertEquals (self.post_id (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraMsg (HydraMsg.GET_POST_OK);
        self.setPost_Id ("Life is short but Now lasts for ever");
        self.setReply_To ("Life is short but Now lasts for ever");
        self.setPrevious ("Life is short but Now lasts for ever");
        self.appendTags ("Name: %s", "Brutus");
        self.appendTags ("Age: %d", 43);
        self.setTimestamp ("Life is short but Now lasts for ever");
        byte [] digestData = new byte [HydraMsg.DIGEST_SIZE];
        for (int i=0; i < HydraMsg.DIGEST_SIZE; i++)
            digestData [i] = 123;
        self.setDigest (digestData);
        self.setType ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        assertEquals (self.post_id (), "Life is short but Now lasts for ever");
        assertEquals (self.reply_to (), "Life is short but Now lasts for ever");
        assertEquals (self.previous (), "Life is short but Now lasts for ever");
        assertEquals (self.tags ().size (), 2);
        assertEquals (self.tags ().get (0), "Name: Brutus");
        assertEquals (self.tags ().get (1), "Age: 43");
        assertEquals (self.timestamp (), "Life is short but Now lasts for ever");
        assertEquals (self.digest () [0], 123);
        assertEquals (self.digest () [HydraMsg.DIGEST_SIZE - 1], 123);
        assertEquals (self.type (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraMsg (HydraMsg.GET_TAGS);
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        self.destroy ();

        self = new HydraMsg (HydraMsg.GET_TAGS_OK);
        self.appendTags ("Name: %s", "Brutus");
        self.appendTags ("Age: %d", 43);
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        assertEquals (self.tags ().size (), 2);
        assertEquals (self.tags ().get (0), "Name: Brutus");
        assertEquals (self.tags ().get (1), "Age: 43");
        self.destroy ();

        self = new HydraMsg (HydraMsg.GET_TAG);
        self.setTag ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        assertEquals (self.tag (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraMsg (HydraMsg.GET_TAG_OK);
        self.setTag ("Life is short but Now lasts for ever");
        self.setPost_Id ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        assertEquals (self.tag (), "Life is short but Now lasts for ever");
        assertEquals (self.post_id (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraMsg (HydraMsg.GOODBYE);
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        self.destroy ();

        self = new HydraMsg (HydraMsg.GOODBYE_OK);
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        self.destroy ();

        self = new HydraMsg (HydraMsg.INVALID);
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        self.destroy ();

        self = new HydraMsg (HydraMsg.FAILED);
        self.setReason ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        assertEquals (self.reason (), "Life is short but Now lasts for ever");
        self.destroy ();

        ctx.destroy ();
        System.out.printf ("OK\n");
    }
}
