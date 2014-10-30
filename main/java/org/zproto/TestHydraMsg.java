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
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        self.destroy ();

        self = new HydraMsg (HydraMsg.HELLO_OK);
        self.setPost_Id ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        assertEquals (self.post_id (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraMsg (HydraMsg.QUERY);
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        self.destroy ();

        self = new HydraMsg (HydraMsg.QUERY_OK);
        self.appendTags ("Name: %s", "Brutus");
        self.appendTags ("Age: %d", 43);
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        assertEquals (self.tags ().size (), 2);
        assertEquals (self.tags ().get (0), "Name: Brutus");
        assertEquals (self.tags ().get (1), "Age: 43");
        self.destroy ();

        self = new HydraMsg (HydraMsg.STATUS);
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        self.destroy ();

        self = new HydraMsg (HydraMsg.STATUS_OK);
        self.setPost_Id ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        assertEquals (self.post_id (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraMsg (HydraMsg.FETCH);
        self.setPost_Id ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraMsg.recv (input);
        assert (self != null);
        assertEquals (self.post_id (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraMsg (HydraMsg.FETCH_OK);
        self.setPost_Id ("Life is short but Now lasts for ever");
        self.setReply_To ("Life is short but Now lasts for ever");
        self.setPrevious ("Life is short but Now lasts for ever");
        self.appendTags ("Name: %s", "Brutus");
        self.appendTags ("Age: %d", 43);
        self.setTimestamp ((byte) 123);
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
        assertEquals (self.timestamp (), 123);
        assertEquals (self.type (), "Life is short but Now lasts for ever");
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
