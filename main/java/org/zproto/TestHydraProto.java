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
        self.setIdentity ("Life is short but Now lasts for ever");
        self.setNickname ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        assertEquals (self.identity (), "Life is short but Now lasts for ever");
        assertEquals (self.nickname (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraProto (HydraProto.NEXT_OLDER);
        self.setIdent ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        assertEquals (self.ident (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraProto (HydraProto.NEXT_NEWER);
        self.setIdent ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        assertEquals (self.ident (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraProto (HydraProto.NEXT_OK);
        self.setIdent ("Life is short but Now lasts for ever");
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        assertEquals (self.ident (), "Life is short but Now lasts for ever");
        self.destroy ();

        self = new HydraProto (HydraProto.NEXT_EMPTY);
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        self.destroy ();

        self = new HydraProto (HydraProto.META);
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        self.destroy ();

        self = new HydraProto (HydraProto.META_OK);
        self.setSubject ("Life is short but Now lasts for ever");
        self.setTimestamp ("Life is short but Now lasts for ever");
        self.setParent_Id ("Life is short but Now lasts for ever");
        self.setDigest ("Life is short but Now lasts for ever");
        self.setMime_Type ("Life is short but Now lasts for ever");
        self.setContent_Size ((byte) 123);
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        assertEquals (self.subject (), "Life is short but Now lasts for ever");
        assertEquals (self.timestamp (), "Life is short but Now lasts for ever");
        assertEquals (self.parent_id (), "Life is short but Now lasts for ever");
        assertEquals (self.digest (), "Life is short but Now lasts for ever");
        assertEquals (self.mime_type (), "Life is short but Now lasts for ever");
        assertEquals (self.content_size (), 123);
        self.destroy ();

        self = new HydraProto (HydraProto.CHUNK);
        self.setOffset ((byte) 123);
        self.setOctets ((byte) 123);
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        assertEquals (self.offset (), 123);
        assertEquals (self.octets (), 123);
        self.destroy ();

        self = new HydraProto (HydraProto.CHUNK_OK);
        self.setOffset ((byte) 123);
        self.setContent ("Captcha Diem".getBytes());
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        assertEquals (self.offset (), 123);
        assertTrue (java.util.Arrays.equals("Captcha Diem".getBytes(), self.content ()));
        self.destroy ();

        self = new HydraProto (HydraProto.PING);
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
        self.destroy ();

        self = new HydraProto (HydraProto.PING_OK);
        self.send (output);

        self = HydraProto.recv (input);
        assert (self != null);
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
