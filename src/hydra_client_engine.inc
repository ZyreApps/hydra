/*  =========================================================================
    hydra_client_engine - Hydra Client engine

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: hydra_client.xml, or
     * The code generation script that built this file: zproto_client_c
    ************************************************************************
    Copyright (c) the Contributors as noted in the AUTHORS file.       
    This file is part of zbroker, the ZeroMQ broker project.           
                                                                       
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.           
    =========================================================================
*/


//  ---------------------------------------------------------------------------
//  State machine constants

typedef enum {
    start_state = 1,
    expect_hello_ok_state = 2,
    connected_state = 3,
    have_peer_status_state = 4,
    scanning_backwards_state = 5,
    scanning_forwards_state = 6,
    defaults_state = 7,
    have_error_state = 8,
    reconnecting_state = 9,
    expect_goodbye_ok_state = 10
} state_t;

typedef enum {
    NULL_event = 0,
    connect_event = 1,
    bad_endpoint_event = 2,
    destructor_event = 3,
    hello_ok_event = 4,
    expired_event = 5,
    sync_event = 6,
    new_peer_event = 7,
    known_peer_event = 8,
    next_ok_event = 9,
    next_empty_event = 10,
    meta_ok_event = 11,
    chunk_ok_event = 12,
    duplicate_event = 13,
    ping_ok_event = 14,
    error_event = 15,
    exception_event = 16,
    command_invalid_event = 17,
    other_event = 18,
    goodbye_ok_event = 19
} event_t;

//  Names for state machine logging and error reporting
static char *
s_state_name [] = {
    "(NONE)",
    "start",
    "expect hello ok",
    "connected",
    "have peer status",
    "scanning backwards",
    "scanning forwards",
    "defaults",
    "have error",
    "reconnecting",
    "expect goodbye ok"
};

static char *
s_event_name [] = {
    "(NONE)",
    "connect",
    "bad_endpoint",
    "destructor",
    "HELLO_OK",
    "expired",
    "sync",
    "new_peer",
    "known_peer",
    "NEXT_OK",
    "NEXT_EMPTY",
    "META_OK",
    "CHUNK_OK",
    "duplicate",
    "PING_OK",
    "ERROR",
    "exception",
    "command_invalid",
    "other",
    "GOODBYE_OK"
};


//  ---------------------------------------------------------------------------
//  Context for the client. This embeds the application-level client context
//  at its start (the entire structure, not a reference), so we can cast a
//  pointer between client_t and s_client_t arbitrarily.

//  These are the different method arguments we manage automatically
struct _client_args_t {
    char *endpoint;
    uint32_t timeout;
};

typedef struct {
    client_t client;            //  Application-level client context
    zsock_t *cmdpipe;           //  Get/send commands from caller API
    zsock_t *msgpipe;           //  Get/send messages from caller API
    zsock_t *dealer;            //  Socket to talk to server
    zloop_t *loop;              //  Listen to pipe and dealer
    hydra_proto_t *message;     //  Message received or sent
    client_args_t args;         //  Method arguments structure
    bool connected;             //  True if client is connected
    bool terminated;            //  True if client is shutdown
    bool fsm_stopped;           //  "terminate" action called
    size_t timeout;             //  inactivity timeout, msecs
    state_t state;              //  Current state
    event_t event;              //  Current event
    event_t next_event;         //  The next event
    event_t exception;          //  Exception event, if any
    int expiry_timer;           //  zloop timer for timeouts
    int wakeup_timer;           //  zloop timer for alarms
    event_t wakeup_event;       //  Wake up with this event
} s_client_t;

static int
    client_initialize (client_t *self);
static void
    client_terminate (client_t *self);
static void
    s_client_destroy (s_client_t **self_p);
static void
    s_client_execute (s_client_t *self, event_t event);
static int
    s_client_handle_wakeup (zloop_t *loop, int timer_id, void *argument);
static int
    s_client_handle_timeout (zloop_t *loop, int timer_id, void *argument);
static void
    s_satisfy_pedantic_compilers (void);
static void
    connect_to_server_endpoint (client_t *self);
static void
    set_client_identity (client_t *self);
static void
    use_connect_timeout (client_t *self);
static void
    signal_bad_endpoint (client_t *self);
static void
    signal_success (client_t *self);
static void
    signal_connected (client_t *self);
static void
    client_is_connected (client_t *self);
static void
    check_peer_status (client_t *self);
static void
    prepare_to_fetch_newest_post (client_t *self);
static void
    prepare_to_fetch_newer_post (client_t *self);
static void
    use_this_post_as_oldest (client_t *self);
static void
    skip_post_if_duplicate (client_t *self);
static void
    signal_sync_success (client_t *self);
static void
    store_post_metadata (client_t *self);
static void
    prepare_to_get_first_chunk (client_t *self);
static void
    store_post_content_chunk (client_t *self);
static void
    store_complete_post (client_t *self);
static void
    save_peer_configuration (client_t *self);
static void
    use_this_post_as_newest (client_t *self);
static void
    prepare_to_fetch_older_post (client_t *self);
static void
    check_if_connection_is_dead (client_t *self);
static void
    check_status_code (client_t *self);
static void
    signal_internal_error (client_t *self);
static void
    signal_unhandled_error (client_t *self);

//  Global tracing/animation indicator; we can't use a client method as
//  that only works after construction (which we often want to trace).
volatile int hydra_client_verbose = false;

//  Create a new client connection

static s_client_t *
s_client_new (zsock_t *cmdpipe, zsock_t *msgpipe)
{
    s_client_t *self = (s_client_t *) zmalloc (sizeof (s_client_t));
    if (self) {
        assert ((s_client_t *) &self->client == self);
        self->cmdpipe = cmdpipe;
        self->msgpipe = msgpipe;
        self->dealer = zsock_new (ZMQ_DEALER);
        if (self->dealer)
            self->message = hydra_proto_new ();
        if (self->message)
            self->loop = zloop_new ();
        if (self->loop) {
            //  Give application chance to initialize and set next event
            self->state = start_state;
            self->event = NULL_event;
            self->client.cmdpipe = self->cmdpipe;
            self->client.msgpipe = self->msgpipe;
            self->client.dealer = self->dealer;
            self->client.message = self->message;
            self->client.args = &self->args;
            if (client_initialize (&self->client))
                s_client_destroy (&self);
        }
        else
            s_client_destroy (&self);
    }
    s_satisfy_pedantic_compilers ();
    return self;
}

//  Destroy the client connection

static void
s_client_destroy (s_client_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        s_client_t *self = *self_p;
        zstr_free (&self->args.endpoint);
        client_terminate (&self->client);
        hydra_proto_destroy (&self->message);
        zsock_destroy (&self->msgpipe);
        zsock_destroy (&self->dealer);
        zloop_destroy (&self->loop);
        free (self);
        *self_p = NULL;
    }
}

//  ---------------------------------------------------------------------------
//  These methods are an internal API for actions

//  Set the next event, needed in at least one action in an internal
//  state; otherwise the state machine will wait for a message on the
//  dealer socket and treat that as the event.

static void
engine_set_next_event (client_t *client, event_t event)
{
    if (client) {
        s_client_t *self = (s_client_t *) client;
        self->next_event = event;
    }
}

//  Raise an exception with 'event', halting any actions in progress.
//  Continues execution of actions defined for the exception event.

static void
engine_set_exception (client_t *client, event_t event)
{
    if (client) {
        s_client_t *self = (s_client_t *) client;
        self->exception = event;
    }
}

//  Set wakeup alarm after 'delay' msecs. The next state should handle the
//  wakeup event. The alarm is cancelled on any other event.

static void
engine_set_wakeup_event (client_t *client, size_t delay, event_t event)
{
    if (client) {
        s_client_t *self = (s_client_t *) client;
        if (self->wakeup_timer) {
            zloop_timer_end (self->loop, self->wakeup_timer);
            self->wakeup_timer = 0;
        }
        self->wakeup_timer = zloop_timer (
            self->loop, delay, 1, s_client_handle_wakeup, self);
        self->wakeup_event = event;
    }
}

//  Set heartbeat timeout. By default, the timeout is zero, meaning
//  infinite. Setting a non-zero timeout causes the state machine to
//  receive an "expired" event if is no incoming traffic for that many
//  milliseconds. This cycles over and over until/unless the code sets
//  a zero timeout. The state machine must handle the "expired" event.

static void
engine_set_timeout (client_t *client, size_t timeout)
{
    if (client) {
        s_client_t *self = (s_client_t *) client;
        self->timeout = timeout;
        if (self->expiry_timer) {
            zloop_timer_end (self->loop, self->expiry_timer);
            self->expiry_timer = 0;
        }
        if (self->timeout)
            self->expiry_timer = zloop_timer (
                self->loop, self->timeout, 1, s_client_handle_timeout, self);
    }
}

//  Poll socket for activity, invoke handler on any received message.
//  Handler must be a CZMQ zloop_fn function; receives client as arg.

static void
engine_handle_socket (client_t *client, zsock_t *sock, zloop_reader_fn handler)
{
    if (client && sock) {
        s_client_t *self = (s_client_t *) client;
        if (handler != NULL) {
            int rc = zloop_reader (self->loop, sock, handler, self);
            assert (rc == 0);
            zloop_reader_set_tolerant (self->loop, sock);
        }
        else
            zloop_reader_end (self->loop, sock);
    }
}

//  Set connected to true/false. The client must call this if it wants
//  to provide the API with the connected status.

static void
engine_set_connected (client_t *client, bool connected)
{
    if (client) {
        s_client_t *self = (s_client_t *) client;
        self->connected = connected;
    }
}

//  Pedantic compilers don't like unused functions, so we call the whole
//  API, passing null references. It's nasty and horrid and sufficient.

static void
s_satisfy_pedantic_compilers (void)
{
    engine_set_next_event (NULL, NULL_event);
    engine_set_exception (NULL, NULL_event);
    engine_set_timeout (NULL, 0);
    engine_set_wakeup_event (NULL, 0, NULL_event);
    engine_handle_socket (NULL, 0, NULL);
}


//  ---------------------------------------------------------------------------
//  Generic methods on protocol messages
//  TODO: replace with lookup table, since ID is one byte

static event_t
s_protocol_event (s_client_t *self, hydra_proto_t *message)
{
    assert (message);
    switch (hydra_proto_id (message)) {
        case HYDRA_PROTO_HELLO_OK:
            return hello_ok_event;
            break;
        case HYDRA_PROTO_NEXT_OK:
            return next_ok_event;
            break;
        case HYDRA_PROTO_NEXT_EMPTY:
            return next_empty_event;
            break;
        case HYDRA_PROTO_META_OK:
            return meta_ok_event;
            break;
        case HYDRA_PROTO_CHUNK_OK:
            return chunk_ok_event;
            break;
        case HYDRA_PROTO_PING_OK:
            return ping_ok_event;
            break;
        case HYDRA_PROTO_GOODBYE_OK:
            return goodbye_ok_event;
            break;
        case HYDRA_PROTO_ERROR:
            return error_event;
            break;
        default:
            zsys_error ("hydra_client: unknown command %s, halting", hydra_proto_command (message));
            self->terminated = true;
            return NULL_event;
    }
}


//  Execute state machine as long as we have events; if event is NULL_event,
//  or state machine is stopped, do nothing.

static void
s_client_execute (s_client_t *self, event_t event)
{
    self->next_event = event;
    //  Cancel wakeup timer, if any was pending
    if (self->wakeup_timer) {
        zloop_timer_end (self->loop, self->wakeup_timer);
        self->wakeup_timer = 0;
    }
    while (!self->terminated                    //  Actor is dying
        && !self->fsm_stopped                   //  FSM has finished
        && self->next_event != NULL_event) {
        self->event = self->next_event;
        self->next_event = NULL_event;
        self->exception = NULL_event;
        if (hydra_client_verbose) {
            zsys_debug ("hydra_client: %s:", s_state_name [self->state]);
            zsys_debug ("hydra_client:      %s", s_event_name [self->event]);
        }
        switch (self->state) {
            case start_state:
                if (self->event == connect_event) {
                    if (!self->exception) {
                        //  connect to server endpoint
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ connect to server endpoint");
                        connect_to_server_endpoint (&self->client);
                    }
                    if (!self->exception) {
                        //  set client identity
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ set client identity");
                        set_client_identity (&self->client);
                    }
                    if (!self->exception) {
                        //  use connect timeout
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ use connect timeout");
                        use_connect_timeout (&self->client);
                    }
                    if (!self->exception) {
                        //  send HELLO
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send HELLO");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_HELLO);
                        hydra_proto_send (self->message, self->dealer);
                    }
                    if (!self->exception)
                        self->state = expect_hello_ok_state;
                }
                else
                if (self->event == bad_endpoint_event) {
                    if (!self->exception) {
                        //  signal bad endpoint
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal bad endpoint");
                        signal_bad_endpoint (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->fsm_stopped = true;
                    }
                }
                else
                if (self->event == destructor_event) {
                    if (!self->exception) {
                        //  signal success
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal success");
                        signal_success (&self->client);
                    }
                }
                else {
                    //  Handle unexpected internal events
                    zsys_warning ("hydra_client: unhandled event %s in %s",
                        s_event_name [self->event], s_state_name [self->state]);
                    assert (false);
                }
                break;

            case expect_hello_ok_state:
                if (self->event == hello_ok_event) {
                    if (!self->exception) {
                        //  signal connected
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal connected");
                        signal_connected (&self->client);
                    }
                    if (!self->exception) {
                        //  client is connected
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ client is connected");
                        client_is_connected (&self->client);
                    }
                    if (!self->exception)
                        self->state = connected_state;
                }
                else
                if (self->event == expired_event) {
                        //  No action - just logging
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ expired");
                    if (!self->exception)
                        self->state = start_state;
                }
                else
                if (self->event == destructor_event) {
                    if (!self->exception) {
                        //  send GOODBYE
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send GOODBYE");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_GOODBYE);
                        hydra_proto_send (self->message, self->dealer);
                    }
                    if (!self->exception)
                        self->state = expect_goodbye_ok_state;
                }
                else
                if (self->event == ping_ok_event) {
                    if (!self->exception) {
                        //  client is connected
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ client is connected");
                        client_is_connected (&self->client);
                    }
                }
                else
                if (self->event == error_event) {
                    if (!self->exception) {
                        //  check status code
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ check status code");
                        check_status_code (&self->client);
                    }
                    if (!self->exception)
                        self->state = have_error_state;
                }
                else
                if (self->event == exception_event) {
                        //  No action - just logging
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ exception");
                }
                else {
                    //  Handle unexpected protocol events
                    if (!self->exception) {
                        //  signal internal error
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal internal error");
                        signal_internal_error (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->fsm_stopped = true;
                    }
                }
                break;

            case connected_state:
                if (self->event == sync_event) {
                    if (!self->exception) {
                        //  signal success
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal success");
                        signal_success (&self->client);
                    }
                    if (!self->exception) {
                        //  check peer status
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ check peer status");
                        check_peer_status (&self->client);
                    }
                    if (!self->exception)
                        self->state = have_peer_status_state;
                }
                else
                if (self->event == destructor_event) {
                    if (!self->exception) {
                        //  send GOODBYE
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send GOODBYE");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_GOODBYE);
                        hydra_proto_send (self->message, self->dealer);
                    }
                    if (!self->exception)
                        self->state = expect_goodbye_ok_state;
                }
                else
                if (self->event == expired_event) {
                    if (!self->exception) {
                        //  check if connection is dead
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ check if connection is dead");
                        check_if_connection_is_dead (&self->client);
                    }
                    if (!self->exception) {
                        //  send PING
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send PING");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_PING);
                        hydra_proto_send (self->message, self->dealer);
                    }
                }
                else
                if (self->event == ping_ok_event) {
                    if (!self->exception) {
                        //  client is connected
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ client is connected");
                        client_is_connected (&self->client);
                    }
                }
                else
                if (self->event == error_event) {
                    if (!self->exception) {
                        //  check status code
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ check status code");
                        check_status_code (&self->client);
                    }
                    if (!self->exception)
                        self->state = have_error_state;
                }
                else
                if (self->event == exception_event) {
                        //  No action - just logging
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ exception");
                }
                else {
                    //  Handle unexpected protocol events
                    if (!self->exception) {
                        //  signal internal error
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal internal error");
                        signal_internal_error (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->fsm_stopped = true;
                    }
                }
                break;

            case have_peer_status_state:
                if (self->event == new_peer_event) {
                    if (!self->exception) {
                        //  prepare to fetch newest post
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ prepare to fetch newest post");
                        prepare_to_fetch_newest_post (&self->client);
                    }
                    if (!self->exception) {
                        //  send NEXT_OLDER
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send NEXT_OLDER");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_NEXT_OLDER);
                        hydra_proto_send (self->message, self->dealer);
                    }
                    if (!self->exception)
                        self->state = scanning_backwards_state;
                }
                else
                if (self->event == known_peer_event) {
                    if (!self->exception) {
                        //  prepare to fetch newer post
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ prepare to fetch newer post");
                        prepare_to_fetch_newer_post (&self->client);
                    }
                    if (!self->exception) {
                        //  send NEXT_NEWER
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send NEXT_NEWER");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_NEXT_NEWER);
                        hydra_proto_send (self->message, self->dealer);
                    }
                    if (!self->exception)
                        self->state = scanning_forwards_state;
                }
                else {
                    //  Handle unexpected internal events
                    zsys_warning ("hydra_client: unhandled event %s in %s",
                        s_event_name [self->event], s_state_name [self->state]);
                    assert (false);
                }
                break;

            case scanning_backwards_state:
                if (self->event == next_ok_event) {
                    if (!self->exception) {
                        //  use this post as oldest
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ use this post as oldest");
                        use_this_post_as_oldest (&self->client);
                    }
                    if (!self->exception) {
                        //  skip post if duplicate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ skip post if duplicate");
                        skip_post_if_duplicate (&self->client);
                    }
                    if (!self->exception) {
                        //  send META
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send META");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_META);
                        hydra_proto_send (self->message, self->dealer);
                    }
                }
                else
                if (self->event == next_empty_event) {
                    if (!self->exception) {
                        //  signal sync success
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal sync success");
                        signal_sync_success (&self->client);
                    }
                    if (!self->exception)
                        self->state = connected_state;
                }
                else
                if (self->event == meta_ok_event) {
                    if (!self->exception) {
                        //  store post metadata
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ store post metadata");
                        store_post_metadata (&self->client);
                    }
                    if (!self->exception) {
                        //  prepare to get first chunk
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ prepare to get first chunk");
                        prepare_to_get_first_chunk (&self->client);
                    }
                    if (!self->exception) {
                        //  send CHUNK
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send CHUNK");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_CHUNK);
                        hydra_proto_send (self->message, self->dealer);
                    }
                }
                else
                if (self->event == chunk_ok_event) {
                    if (!self->exception) {
                        //  store post content chunk
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ store post content chunk");
                        store_post_content_chunk (&self->client);
                    }
                    if (!self->exception) {
                        //  store complete post
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ store complete post");
                        store_complete_post (&self->client);
                    }
                    if (!self->exception) {
                        //  save peer configuration
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ save peer configuration");
                        save_peer_configuration (&self->client);
                    }
                    if (!self->exception) {
                        //  send NEXT_OLDER
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send NEXT_OLDER");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_NEXT_OLDER);
                        hydra_proto_send (self->message, self->dealer);
                    }
                }
                else
                if (self->event == duplicate_event) {
                    if (!self->exception) {
                        //  send NEXT_OLDER
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send NEXT_OLDER");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_NEXT_OLDER);
                        hydra_proto_send (self->message, self->dealer);
                    }
                }
                else
                if (self->event == destructor_event) {
                    if (!self->exception) {
                        //  send GOODBYE
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send GOODBYE");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_GOODBYE);
                        hydra_proto_send (self->message, self->dealer);
                    }
                    if (!self->exception)
                        self->state = expect_goodbye_ok_state;
                }
                else
                if (self->event == expired_event) {
                    if (!self->exception) {
                        //  check if connection is dead
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ check if connection is dead");
                        check_if_connection_is_dead (&self->client);
                    }
                    if (!self->exception) {
                        //  send PING
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send PING");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_PING);
                        hydra_proto_send (self->message, self->dealer);
                    }
                }
                else
                if (self->event == ping_ok_event) {
                    if (!self->exception) {
                        //  client is connected
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ client is connected");
                        client_is_connected (&self->client);
                    }
                }
                else
                if (self->event == error_event) {
                    if (!self->exception) {
                        //  check status code
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ check status code");
                        check_status_code (&self->client);
                    }
                    if (!self->exception)
                        self->state = have_error_state;
                }
                else
                if (self->event == exception_event) {
                        //  No action - just logging
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ exception");
                }
                else {
                    //  Handle unexpected protocol events
                    if (!self->exception) {
                        //  signal internal error
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal internal error");
                        signal_internal_error (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->fsm_stopped = true;
                    }
                }
                break;

            case scanning_forwards_state:
                if (self->event == next_ok_event) {
                    if (!self->exception) {
                        //  use this post as newest
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ use this post as newest");
                        use_this_post_as_newest (&self->client);
                    }
                    if (!self->exception) {
                        //  skip post if duplicate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ skip post if duplicate");
                        skip_post_if_duplicate (&self->client);
                    }
                    if (!self->exception) {
                        //  send META
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send META");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_META);
                        hydra_proto_send (self->message, self->dealer);
                    }
                }
                else
                if (self->event == next_empty_event) {
                    if (!self->exception) {
                        //  prepare to fetch older post
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ prepare to fetch older post");
                        prepare_to_fetch_older_post (&self->client);
                    }
                    if (!self->exception) {
                        //  send NEXT_OLDER
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send NEXT_OLDER");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_NEXT_OLDER);
                        hydra_proto_send (self->message, self->dealer);
                    }
                    if (!self->exception)
                        self->state = scanning_backwards_state;
                }
                else
                if (self->event == meta_ok_event) {
                    if (!self->exception) {
                        //  store post metadata
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ store post metadata");
                        store_post_metadata (&self->client);
                    }
                    if (!self->exception) {
                        //  prepare to get first chunk
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ prepare to get first chunk");
                        prepare_to_get_first_chunk (&self->client);
                    }
                    if (!self->exception) {
                        //  send CHUNK
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send CHUNK");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_CHUNK);
                        hydra_proto_send (self->message, self->dealer);
                    }
                }
                else
                if (self->event == chunk_ok_event) {
                    if (!self->exception) {
                        //  store post content chunk
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ store post content chunk");
                        store_post_content_chunk (&self->client);
                    }
                    if (!self->exception) {
                        //  store complete post
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ store complete post");
                        store_complete_post (&self->client);
                    }
                    if (!self->exception) {
                        //  save peer configuration
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ save peer configuration");
                        save_peer_configuration (&self->client);
                    }
                    if (!self->exception) {
                        //  send NEXT_NEWER
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send NEXT_NEWER");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_NEXT_NEWER);
                        hydra_proto_send (self->message, self->dealer);
                    }
                }
                else
                if (self->event == duplicate_event) {
                    if (!self->exception) {
                        //  send NEXT_NEWER
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send NEXT_NEWER");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_NEXT_NEWER);
                        hydra_proto_send (self->message, self->dealer);
                    }
                }
                else
                if (self->event == destructor_event) {
                    if (!self->exception) {
                        //  send GOODBYE
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send GOODBYE");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_GOODBYE);
                        hydra_proto_send (self->message, self->dealer);
                    }
                    if (!self->exception)
                        self->state = expect_goodbye_ok_state;
                }
                else
                if (self->event == expired_event) {
                    if (!self->exception) {
                        //  check if connection is dead
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ check if connection is dead");
                        check_if_connection_is_dead (&self->client);
                    }
                    if (!self->exception) {
                        //  send PING
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send PING");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_PING);
                        hydra_proto_send (self->message, self->dealer);
                    }
                }
                else
                if (self->event == ping_ok_event) {
                    if (!self->exception) {
                        //  client is connected
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ client is connected");
                        client_is_connected (&self->client);
                    }
                }
                else
                if (self->event == error_event) {
                    if (!self->exception) {
                        //  check status code
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ check status code");
                        check_status_code (&self->client);
                    }
                    if (!self->exception)
                        self->state = have_error_state;
                }
                else
                if (self->event == exception_event) {
                        //  No action - just logging
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ exception");
                }
                else {
                    //  Handle unexpected protocol events
                    if (!self->exception) {
                        //  signal internal error
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal internal error");
                        signal_internal_error (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->fsm_stopped = true;
                    }
                }
                break;

            case defaults_state:
                if (self->event == destructor_event) {
                    if (!self->exception) {
                        //  send GOODBYE
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send GOODBYE");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_GOODBYE);
                        hydra_proto_send (self->message, self->dealer);
                    }
                    if (!self->exception)
                        self->state = expect_goodbye_ok_state;
                }
                else
                if (self->event == expired_event) {
                    if (!self->exception) {
                        //  check if connection is dead
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ check if connection is dead");
                        check_if_connection_is_dead (&self->client);
                    }
                    if (!self->exception) {
                        //  send PING
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send PING");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_PING);
                        hydra_proto_send (self->message, self->dealer);
                    }
                }
                else
                if (self->event == ping_ok_event) {
                    if (!self->exception) {
                        //  client is connected
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ client is connected");
                        client_is_connected (&self->client);
                    }
                }
                else
                if (self->event == error_event) {
                    if (!self->exception) {
                        //  check status code
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ check status code");
                        check_status_code (&self->client);
                    }
                    if (!self->exception)
                        self->state = have_error_state;
                }
                else
                if (self->event == exception_event) {
                        //  No action - just logging
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ exception");
                }
                else {
                    //  Handle unexpected protocol events
                    if (!self->exception) {
                        //  signal internal error
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal internal error");
                        signal_internal_error (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->fsm_stopped = true;
                    }
                }
                break;

            case have_error_state:
                if (self->event == command_invalid_event) {
                    if (!self->exception) {
                        //  use connect timeout
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ use connect timeout");
                        use_connect_timeout (&self->client);
                    }
                    if (!self->exception) {
                        //  send HELLO
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send HELLO");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_HELLO);
                        hydra_proto_send (self->message, self->dealer);
                    }
                    if (!self->exception)
                        self->state = reconnecting_state;
                }
                else
                if (self->event == other_event) {
                    if (!self->exception) {
                        //  signal unhandled error
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal unhandled error");
                        signal_unhandled_error (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->fsm_stopped = true;
                    }
                }
                else {
                    //  Handle unexpected internal events
                    zsys_warning ("hydra_client: unhandled event %s in %s",
                        s_event_name [self->event], s_state_name [self->state]);
                    assert (false);
                }
                break;

            case reconnecting_state:
                if (self->event == hello_ok_event) {
                    if (!self->exception) {
                        //  client is connected
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ client is connected");
                        client_is_connected (&self->client);
                    }
                    if (!self->exception)
                        self->state = connected_state;
                }
                else
                if (self->event == destructor_event) {
                    if (!self->exception) {
                        //  send GOODBYE
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send GOODBYE");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_GOODBYE);
                        hydra_proto_send (self->message, self->dealer);
                    }
                    if (!self->exception)
                        self->state = expect_goodbye_ok_state;
                }
                else
                if (self->event == expired_event) {
                    if (!self->exception) {
                        //  check if connection is dead
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ check if connection is dead");
                        check_if_connection_is_dead (&self->client);
                    }
                    if (!self->exception) {
                        //  send PING
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send PING");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_PING);
                        hydra_proto_send (self->message, self->dealer);
                    }
                }
                else
                if (self->event == ping_ok_event) {
                    if (!self->exception) {
                        //  client is connected
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ client is connected");
                        client_is_connected (&self->client);
                    }
                }
                else
                if (self->event == error_event) {
                    if (!self->exception) {
                        //  check status code
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ check status code");
                        check_status_code (&self->client);
                    }
                    if (!self->exception)
                        self->state = have_error_state;
                }
                else
                if (self->event == exception_event) {
                        //  No action - just logging
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ exception");
                }
                else {
                    //  Handle unexpected protocol events
                    if (!self->exception) {
                        //  signal internal error
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal internal error");
                        signal_internal_error (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->fsm_stopped = true;
                    }
                }
                break;

            case expect_goodbye_ok_state:
                if (self->event == goodbye_ok_event) {
                    if (!self->exception) {
                        //  signal success
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal success");
                        signal_success (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->fsm_stopped = true;
                    }
                }
                else
                if (self->event == destructor_event) {
                    if (!self->exception) {
                        //  send GOODBYE
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send GOODBYE");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_GOODBYE);
                        hydra_proto_send (self->message, self->dealer);
                    }
                    if (!self->exception)
                        self->state = expect_goodbye_ok_state;
                }
                else
                if (self->event == expired_event) {
                    if (!self->exception) {
                        //  check if connection is dead
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ check if connection is dead");
                        check_if_connection_is_dead (&self->client);
                    }
                    if (!self->exception) {
                        //  send PING
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send PING");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_PING);
                        hydra_proto_send (self->message, self->dealer);
                    }
                }
                else
                if (self->event == ping_ok_event) {
                    if (!self->exception) {
                        //  client is connected
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ client is connected");
                        client_is_connected (&self->client);
                    }
                }
                else
                if (self->event == error_event) {
                    if (!self->exception) {
                        //  check status code
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ check status code");
                        check_status_code (&self->client);
                    }
                    if (!self->exception)
                        self->state = have_error_state;
                }
                else
                if (self->event == exception_event) {
                        //  No action - just logging
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ exception");
                }
                else {
                    //  Handle unexpected protocol events
                        //  No action - just logging
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ *");
                }
                break;
        }
        //  If we had an exception event, interrupt normal programming
        if (self->exception) {
            if (hydra_client_verbose)
                zsys_debug ("hydra_client:          ! %s", s_event_name [self->exception]);
            self->next_event = self->exception;
        }
        else
        if (hydra_client_verbose)
            zsys_debug ("hydra_client:          > %s", s_state_name [self->state]);
    }
}

//  zloop callback when client inactivity timer expires

static int
s_client_handle_timeout (zloop_t *loop, int timer_id, void *argument)
{
    s_client_t *self = (s_client_t *) argument;
    s_client_execute (self, expired_event);
    if (self->terminated)
        return -1;

    if (self->timeout > 0)
        self->expiry_timer = zloop_timer (
            loop, self->timeout, 1, s_client_handle_timeout, self);
    return 0;
}

//  zloop callback when client wakeup timer expires

static int
s_client_handle_wakeup (zloop_t *loop, int timer_id, void *argument)
{
    s_client_t *self = (s_client_t *) argument;
    s_client_execute (self, self->wakeup_event);
    return 0;
}


//  Handle command pipe to/from calling API

static int
s_client_handle_cmdpipe (zloop_t *loop, zsock_t *reader, void *argument)
{
    s_client_t *self = (s_client_t *) argument;
    char *method = zstr_recv (self->cmdpipe);
    if (!method)
        return -1;                  //  Interrupted; exit zloop
    if (hydra_client_verbose)
        zsys_debug ("hydra_client:      API command=%s", method);

    if (streq (method, "$TERM"))
        self->terminated = true;    //  Shutdown the engine
    else
    if (streq (method, "$CONNECTED"))
        zsock_send (self->cmdpipe, "i", self->connected);
    else
    if (streq (method, "CONNECT")) {
        zstr_free (&self->args.endpoint);
        zsock_recv (self->cmdpipe, "s4", &self->args.endpoint, &self->args.timeout);
        s_client_execute (self, connect_event);
    }
    else
    if (streq (method, "DESTRUCTOR")) {
        s_client_execute (self, destructor_event);
    }
    else
    if (streq (method, "SYNC")) {
        s_client_execute (self, sync_event);
    }
    //  Cleanup pipe if any argument frames are still waiting to be eaten
    if (zsock_rcvmore (self->cmdpipe)) {
        zsys_error ("hydra_client: trailing API command frames (%s)", method);
        zmsg_t *more = zmsg_recv (self->cmdpipe);
        zmsg_print (more);
        zmsg_destroy (&more);
    }
    zstr_free (&method);
    return self->terminated? -1: 0;
}


//  Handle message pipe to/from calling API

static int
s_client_handle_msgpipe (zloop_t *loop, zsock_t *reader, void *argument)
{
    s_client_t *self = (s_client_t *) argument;

    //  We will process as many messages as we can, to reduce the overhead
    //  of polling and the reactor:
    while (zsock_events (self->msgpipe) & ZMQ_POLLIN) {
        char *method = zstr_recv (self->msgpipe);
        if (!method)
            return -1;              //  Interrupted; exit zloop
        if (hydra_client_verbose)
            zsys_debug ("hydra_client:      API message=%s", method);

        //  Front-end shuts down msgpipe before cmdpipe, this little
        //  handshake just ensures all traffic on the msgpipe has been
        //  flushed before the calling thread continues with destroying
        //  the actor.
        if (streq (method, "$FLUSH"))
            zsock_signal (self->cmdpipe, 0);
        //  Cleanup pipe if any argument frames are still waiting to be eaten
        if (zsock_rcvmore (self->msgpipe)) {
            zsys_error ("hydra_client: trailing API message frames (%s)", method);
            zmsg_t *more = zmsg_recv (self->msgpipe);
            zmsg_print (more);
            zmsg_destroy (&more);
        }
        zstr_free (&method);
    }
    return 0;
}


//  Handle a message (a protocol reply) from the server

static int
s_client_handle_protocol (zloop_t *loop, zsock_t *reader, void *argument)
{
    s_client_t *self = (s_client_t *) argument;

    //  We will process as many messages as we can, to reduce the overhead
    //  of polling and the reactor:
    while (zsock_events (self->dealer) & ZMQ_POLLIN) {
        if (hydra_proto_recv (self->message, self->dealer))
            return -1;              //  Interrupted; exit zloop

        //  Any input from server counts as activity
        if (self->expiry_timer) {
            zloop_timer_end (self->loop, self->expiry_timer);
            self->expiry_timer = 0;
        }
        //  Reset expiry timer if timeout is not zero
        if (self->timeout)
            self->expiry_timer = zloop_timer (
                self->loop, self->timeout, 1, s_client_handle_timeout, self);
        s_client_execute (self, s_protocol_event (self, self->message));
        if (self->terminated)
            return -1;
    }
    return 0;
}


//  ---------------------------------------------------------------------------
//  This is the client actor, which polls its two sockets and processes
//  incoming messages

void
hydra_client (zsock_t *cmdpipe, void *msgpipe)
{
    //  Initialize
    s_client_t *self = s_client_new (cmdpipe, (zsock_t *) msgpipe);
    if (self) {
        zsock_signal (cmdpipe, 0);

        //  Set up handler for the sockets the client uses
        engine_handle_socket ((client_t *) self, self->cmdpipe, s_client_handle_cmdpipe);
        engine_handle_socket ((client_t *) self, self->msgpipe, s_client_handle_msgpipe);
        engine_handle_socket ((client_t *) self, self->dealer, s_client_handle_protocol);

        //  Run reactor until there's a termination signal
        zloop_start (self->loop);

        //  Reactor has ended
        s_client_destroy (&self);
    }
    else
        zsock_signal (cmdpipe, -1);
}


//  ---------------------------------------------------------------------------
//  Class interface

struct _hydra_client_t {
    zactor_t *actor;            //  Client actor
    zsock_t *msgpipe;           //  Pipe for async message flow
    bool connected;             //  Client currently connected or not
    int status;                 //  Returned by actor reply
    char *nickname;             //  Returned by actor reply
    char *reason;               //  Returned by actor reply
};


//  ---------------------------------------------------------------------------
//  Create a new hydra_client

hydra_client_t *
hydra_client_new (void)
{
    hydra_client_t *self = (hydra_client_t *) zmalloc (sizeof (hydra_client_t));
    if (self) {
        zsock_t *backend;
        self->msgpipe = zsys_create_pipe (&backend);
        if (self->msgpipe)
            self->actor = zactor_new (hydra_client, backend);
        if (!self->actor)
            hydra_client_destroy (&self);
    }
    return self;
}


//  ---------------------------------------------------------------------------
//  Destroy the hydra_client
//  Disconnect from server. Waits for a short timeout for confirmation from the     
//  server, then disconnects anyhow.                                                

static int 
hydra_client_destructor (hydra_client_t *self);

void
hydra_client_destroy (hydra_client_t **self_p)
{
    assert (self_p);
    if (*self_p) {
        hydra_client_t *self = *self_p;
        if (self->actor && !zsys_interrupted) {
            //  Before destroying the actor we have to flush any pending
            //  traffic on the msgpipe, otherwise it gets lost in a fire and
            //  forget scenario. We do this by sending $FLUSH to the msgpipe
            //  and waiting for a signal back on the cmdpipe.
            if (zstr_send (self->msgpipe, "$FLUSH") == 0)
                zsock_wait (self->actor);
            hydra_client_destructor (self);
        }
        zactor_destroy (&self->actor);
        zsock_destroy (&self->msgpipe);
        zstr_free (&self->nickname);
        zstr_free (&self->reason);
        free (self);
        *self_p = NULL;
    }
}


//  ---------------------------------------------------------------------------
//  Return actor, when caller wants to work with multiple actors and/or
//  input sockets asynchronously.

zactor_t *
hydra_client_actor (hydra_client_t *self)
{
    assert (self);
    return self->actor;
}


//  ---------------------------------------------------------------------------
//  Return message pipe for asynchronous message I/O. In the high-volume case,
//  we send methods and get replies to the actor, in a synchronous manner, and
//  we send/recv high volume message data to a second pipe, the msgpipe. In
//  the low-volume case we can do everything over the actor pipe, if traffic
//  is never ambiguous.

zsock_t *
hydra_client_msgpipe (hydra_client_t *self)
{
    assert (self);
    return self->msgpipe;
}


//  ---------------------------------------------------------------------------
//  Return true if client is currently connected, else false. Note that the
//  client will automatically re-connect if the server dies and restarts after
//  a successful first connection.

bool
hydra_client_connected (hydra_client_t *self)
{
    assert (self);
    bool connected;
    zsock_send (self->actor, "s", "$CONNECTED");
    zsock_recv (self->actor, "i", &connected);
    return connected;
}


//  ---------------------------------------------------------------------------
//  Get valid reply from actor; discard replies that does not match. Current
//  implementation filters on first frame of message. Blocks until a valid
//  reply is received, and properties can be loaded from it. Returns 0 if
//  matched, -1 if interrupted or timed-out.

static int
s_accept_reply (hydra_client_t *self, ...)
{
    assert (self);
    while (!zsys_interrupted) {
        char *reply = zstr_recv (self->actor);
        if (!reply)
            break;              //  Interrupted or timed-out

        va_list args;
        va_start (args, self);
        char *filter = va_arg (args, char *);
        while (filter) {
            if (streq (reply, filter)) {
                if (streq (reply, "CONNECTED")) {
                    zstr_free (&self->nickname);
                    zsock_recv (self->actor, "is", &self->status, &self->nickname);
                }
                else
                if (streq (reply, "SUCCESS")) {
                    zsock_recv (self->actor, "i", &self->status);
                }
                else
                if (streq (reply, "FAILURE")) {
                    zstr_free (&self->reason);
                    zsock_recv (self->actor, "is", &self->status, &self->reason);
                }
                break;
            }
            filter = va_arg (args, char *);
        }
        va_end (args);
        //  If anything was remaining on pipe, flush it
        zsock_flush (self->actor);
        if (filter) {
            zstr_free (&reply);
            return 0;           //  We matched one of the filters
        }
    }
    return -1;          //  Interrupted or timed-out
}


//  ---------------------------------------------------------------------------
//  Connect to server endpoint, with specified timeout in msecs (zero means wait    
//  forever). Constructor succeeds if connection is successful. The sink endpoint is
//  provided by the node's own server, for storing received posts.                  
//  Returns >= 0 if successful, -1 if interrupted.

int 
hydra_client_connect (hydra_client_t *self, const char *endpoint, uint32_t timeout)
{
    assert (self);

    zsock_send (self->actor, "ss4", "CONNECT", endpoint, timeout);
    if (s_accept_reply (self, "CONNECTED", "FAILURE", NULL))
        return -1;              //  Interrupted or timed-out
    return self->status;
}


//  ---------------------------------------------------------------------------
//  Disconnect from server. Waits for a short timeout for confirmation from the     
//  server, then disconnects anyhow.                                                
//  Returns >= 0 if successful, -1 if interrupted.

int 
hydra_client_destructor (hydra_client_t *self)
{
    assert (self);

    zsock_send (self->actor, "s", "DESTRUCTOR");
    if (s_accept_reply (self, "SUCCESS", "FAILURE", NULL))
        return -1;              //  Interrupted or timed-out
    return self->status;
}


//  ---------------------------------------------------------------------------
//  Start synchronization with server. This method returns immediately, and then    
//  signals progress via the msgpipe socket, with POST, SUCCESS, and FAILED         
//  commands.                                                                       
//  Returns >= 0 if successful, -1 if interrupted.

int 
hydra_client_sync (hydra_client_t *self)
{
    assert (self);

    zsock_send (self->actor, "s", "SYNC");
    if (s_accept_reply (self, "SUCCESS", NULL))
        return -1;              //  Interrupted or timed-out
    return self->status;
}


//  ---------------------------------------------------------------------------
//  Return last received status

int 
hydra_client_status (hydra_client_t *self)
{
    assert (self);
    return self->status;
}


//  ---------------------------------------------------------------------------
//  Return last received nickname

const char *
hydra_client_nickname (hydra_client_t *self)
{
    assert (self);
    return self->nickname;
}


//  ---------------------------------------------------------------------------
//  Return last received reason

const char *
hydra_client_reason (hydra_client_t *self)
{
    assert (self);
    return self->reason;
}
