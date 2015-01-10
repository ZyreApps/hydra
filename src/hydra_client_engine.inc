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
    expect_status_ok_state = 3,
    connected_state = 4,
    expect_header_ok_state = 5,
    expect_chunk_ok_state = 6,
    defaults_state = 7,
    expect_goodbye_ok_state = 8
} state_t;

typedef enum {
    NULL_event = 0,
    constructor_event = 1,
    bad_endpoint_event = 2,
    hello_ok_event = 3,
    status_ok_event = 4,
    fetch_event = 5,
    header_ok_event = 6,
    header_empty_event = 7,
    post_exists_event = 8,
    chunk_ok_event = 9,
    finished_event = 10,
    expired_event = 11,
    destructor_event = 12,
    error_event = 13,
    goodbye_ok_event = 14
} event_t;

//  Names for state machine logging and error reporting
static char *
s_state_name [] = {
    "(NONE)",
    "start",
    "expect hello ok",
    "expect status ok",
    "connected",
    "expect header ok",
    "expect chunk ok",
    "defaults",
    "expect goodbye ok"
};

static char *
s_event_name [] = {
    "(NONE)",
    "constructor",
    "bad_endpoint",
    "HELLO_OK",
    "STATUS_OK",
    "fetch",
    "HEADER_OK",
    "HEADER_EMPTY",
    "post_exists",
    "CHUNK_OK",
    "finished",
    "expired",
    "destructor",
    "ERROR",
    "GOODBYE_OK"
};
 

//  ---------------------------------------------------------------------------
//  Context for the client. This embeds the application-level client context
//  at its start (the entire structure, not a reference), so we can cast a
//  pointer between client_t and s_client_t arbitrarily.

//  These are the different method arguments we manage automatically
struct _client_args_t {
    char *peer_endpoint;
    char *sink_endpoint;
    uint32_t timeout;
    int which;
};

typedef struct {
    client_t client;            //  Application-level client context
    zsock_t *cmdpipe;           //  Get/send commands from caller API
    zsock_t *msgpipe;           //  Get/send messages from caller API
    zsock_t *dealer;            //  Socket to talk to server
    zloop_t *loop;              //  Listen to pipe and dealer
    hydra_proto_t *message;     //  Message received or sent
    client_args_t args;         //  Method arguments structure
    bool terminated;            //  True if client is shutdown
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
    use_response_timeout (client_t *self);
static void
    store_peer_id_and_nickname (client_t *self);
static void
    prepare_to_request_status (client_t *self);
static void
    signal_connected (client_t *self);
static void
    prepare_to_fetch_header (client_t *self);
static void
    store_post_metadata (client_t *self);
static void
    prepare_to_get_first_chunk (client_t *self);
static void
    signal_no_data (client_t *self);
static void
    signal_post_skipped (client_t *self);
static void
    store_post_data_chunk (client_t *self);
static void
    prepare_to_get_next_chunk (client_t *self);
static void
    signal_post_fetched (client_t *self);
static void
    store_the_post_in_ledger (client_t *self);
static void
    save_peer_configuration (client_t *self);
static void
    signal_server_not_present (client_t *self);
static void
    signal_unexpected_server_reply (client_t *self);
static void
    signal_internal_error (client_t *self);
static void
    signal_success (client_t *self);
static void
    signal_incomplete (client_t *self);

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
        zstr_free (&self->args.peer_endpoint);
        zstr_free (&self->args.sink_endpoint);
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

//  Set timeout for next protocol read. By default, will wait forever
//  or until the process is interrupted. The timeout is in milliseconds.
//  The state machine must handle the "expired" event.

static void
engine_set_timeout (client_t *client, size_t timeout)
{
    if (client) {
        s_client_t *self = (s_client_t *) client;
        self->timeout = timeout;
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
        case HYDRA_PROTO_STATUS_OK:
            return status_ok_event;
            break;
        case HYDRA_PROTO_HEADER_OK:
            return header_ok_event;
            break;
        case HYDRA_PROTO_HEADER_EMPTY:
            return header_empty_event;
            break;
        case HYDRA_PROTO_CHUNK_OK:
            return chunk_ok_event;
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
//  or state machine is terminated, do nothing.

static void
s_client_execute (s_client_t *self, event_t event)
{
    self->next_event = event;
    //  Cancel wakeup timer, if any was pending
    if (self->wakeup_timer) {
        zloop_timer_end (self->loop, self->wakeup_timer);
        self->wakeup_timer = 0;
    }
    while (!self->terminated && self->next_event != NULL_event) {
        self->event = self->next_event;
        self->next_event = NULL_event;
        self->exception = NULL_event;
        if (hydra_client_verbose) {
            zsys_debug ("hydra_client: %s:", s_state_name [self->state]);
            zsys_debug ("hydra_client:      %s", s_event_name [self->event]);
        }
        switch (self->state) {
            case start_state:
                if (self->event == constructor_event) {
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
                        self->terminated = true;
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
                        //  use response timeout
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ use response timeout");
                        use_response_timeout (&self->client);
                    }
                    if (!self->exception) {
                        //  store peer id and nickname
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ store peer id and nickname");
                        store_peer_id_and_nickname (&self->client);
                    }
                    if (!self->exception) {
                        //  prepare to request status
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ prepare to request status");
                        prepare_to_request_status (&self->client);
                    }
                    if (!self->exception) {
                        //  send STATUS
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send STATUS");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_STATUS);
                        hydra_proto_send (self->message, self->dealer);
                    }
                    if (!self->exception)
                        self->state = expect_status_ok_state;
                }
                else
                if (self->event == expired_event) {
                    if (!self->exception) {
                        //  signal server not present
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal server not present");
                        signal_server_not_present (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->terminated = true;
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
                if (self->event == error_event) {
                    if (!self->exception) {
                        //  signal unexpected server reply
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal unexpected server reply");
                        signal_unexpected_server_reply (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->terminated = true;
                    }
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
                        self->terminated = true;
                    }
                }
                break;

            case expect_status_ok_state:
                if (self->event == status_ok_event) {
                    if (!self->exception) {
                        //  signal connected
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal connected");
                        signal_connected (&self->client);
                    }
                    if (!self->exception)
                        self->state = connected_state;
                }
                else
                if (self->event == expired_event) {
                    if (!self->exception) {
                        //  signal server not present
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal server not present");
                        signal_server_not_present (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->terminated = true;
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
                if (self->event == error_event) {
                    if (!self->exception) {
                        //  signal unexpected server reply
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal unexpected server reply");
                        signal_unexpected_server_reply (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->terminated = true;
                    }
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
                        self->terminated = true;
                    }
                }
                break;

            case connected_state:
                if (self->event == fetch_event) {
                    if (!self->exception) {
                        //  prepare to fetch header
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ prepare to fetch header");
                        prepare_to_fetch_header (&self->client);
                    }
                    if (!self->exception) {
                        //  send HEADER
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send HEADER");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_HEADER);
                        hydra_proto_send (self->message, self->dealer);
                    }
                    if (!self->exception)
                        self->state = expect_header_ok_state;
                }
                else
                if (self->event == expired_event) {
                    if (!self->exception) {
                        //  signal server not present
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal server not present");
                        signal_server_not_present (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->terminated = true;
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
                if (self->event == error_event) {
                    if (!self->exception) {
                        //  signal unexpected server reply
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal unexpected server reply");
                        signal_unexpected_server_reply (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->terminated = true;
                    }
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
                        self->terminated = true;
                    }
                }
                break;

            case expect_header_ok_state:
                if (self->event == header_ok_event) {
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
                    if (!self->exception)
                        self->state = expect_chunk_ok_state;
                }
                else
                if (self->event == header_empty_event) {
                    if (!self->exception) {
                        //  signal no data
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal no data");
                        signal_no_data (&self->client);
                    }
                    if (!self->exception)
                        self->state = connected_state;
                }
                else
                if (self->event == post_exists_event) {
                    if (!self->exception) {
                        //  signal post skipped
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal post skipped");
                        signal_post_skipped (&self->client);
                    }
                    if (!self->exception)
                        self->state = connected_state;
                }
                else
                if (self->event == expired_event) {
                    if (!self->exception) {
                        //  signal server not present
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal server not present");
                        signal_server_not_present (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->terminated = true;
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
                if (self->event == error_event) {
                    if (!self->exception) {
                        //  signal unexpected server reply
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal unexpected server reply");
                        signal_unexpected_server_reply (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->terminated = true;
                    }
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
                        self->terminated = true;
                    }
                }
                break;

            case expect_chunk_ok_state:
                if (self->event == chunk_ok_event) {
                    if (!self->exception) {
                        //  store post data chunk
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ store post data chunk");
                        store_post_data_chunk (&self->client);
                    }
                    if (!self->exception) {
                        //  prepare to get next chunk
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ prepare to get next chunk");
                        prepare_to_get_next_chunk (&self->client);
                    }
                    if (!self->exception) {
                        //  send CHUNK
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ send CHUNK");
                        hydra_proto_set_id (self->message, HYDRA_PROTO_CHUNK);
                        hydra_proto_send (self->message, self->dealer);
                    }
                    if (!self->exception)
                        self->state = expect_chunk_ok_state;
                }
                else
                if (self->event == finished_event) {
                    if (!self->exception) {
                        //  signal post fetched
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal post fetched");
                        signal_post_fetched (&self->client);
                    }
                    if (!self->exception) {
                        //  store the post in ledger
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ store the post in ledger");
                        store_the_post_in_ledger (&self->client);
                    }
                    if (!self->exception) {
                        //  save peer configuration
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ save peer configuration");
                        save_peer_configuration (&self->client);
                    }
                    if (!self->exception)
                        self->state = connected_state;
                }
                else
                if (self->event == expired_event) {
                    if (!self->exception) {
                        //  signal server not present
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal server not present");
                        signal_server_not_present (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->terminated = true;
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
                if (self->event == error_event) {
                    if (!self->exception) {
                        //  signal unexpected server reply
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal unexpected server reply");
                        signal_unexpected_server_reply (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->terminated = true;
                    }
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
                        self->terminated = true;
                    }
                }
                break;

            case defaults_state:
                if (self->event == expired_event) {
                    if (!self->exception) {
                        //  signal server not present
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal server not present");
                        signal_server_not_present (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->terminated = true;
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
                if (self->event == error_event) {
                    if (!self->exception) {
                        //  signal unexpected server reply
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal unexpected server reply");
                        signal_unexpected_server_reply (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->terminated = true;
                    }
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
                        self->terminated = true;
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
                        //  save peer configuration
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ save peer configuration");
                        save_peer_configuration (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->terminated = true;
                    }
                }
                else
                if (self->event == expired_event) {
                    if (!self->exception) {
                        //  signal incomplete
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal incomplete");
                        signal_incomplete (&self->client);
                    }
                    if (!self->exception) {
                        //  save peer configuration
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ save peer configuration");
                        save_peer_configuration (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->terminated = true;
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
                if (self->event == error_event) {
                    if (!self->exception) {
                        //  signal unexpected server reply
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ signal unexpected server reply");
                        signal_unexpected_server_reply (&self->client);
                    }
                    if (!self->exception) {
                        //  terminate
                        if (hydra_client_verbose)
                            zsys_debug ("hydra_client:          $ terminate");
                        self->terminated = true;
                    }
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
                        self->terminated = true;
                    }
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
    return self->terminated? -1: 0;
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
    if (streq (method, "CONSTRUCTOR")) {
        zstr_free (&self->args.peer_endpoint);
        zstr_free (&self->args.sink_endpoint);
        zsock_recv (self->cmdpipe, "ss4", &self->args.peer_endpoint, &self->args.sink_endpoint, &self->args.timeout);
        s_client_execute (self, constructor_event);
    }
    else
    if (streq (method, "DESTRUCTOR")) {
        s_client_execute (self, destructor_event);
    }
    else
    if (streq (method, "FETCH")) {
        zsock_recv (self->cmdpipe, "i", &self->args.which);
        s_client_execute (self, fetch_event);
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

        //  Front-end shuts down msgpipe before cmdpipe
        if (streq (method, "$TERM"))
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
    int status;                 //  Returned by actor reply
    uint32_t before;            //  Returned by actor reply
    uint32_t after;             //  Returned by actor reply
    char *reason;               //  Returned by actor reply
    char *ident;                //  Returned by actor reply
    char *subject;              //  Returned by actor reply
    char *timestamp;            //  Returned by actor reply
    char *parent_id;            //  Returned by actor reply
    char *mime_type;            //  Returned by actor reply
    uint64_t content_size;      //  Returned by actor reply
};


//  ---------------------------------------------------------------------------
//  Create a new hydra_client
//  Connect to server endpoint, with specified timeout in msecs (zero means wait    
//  forever). Constructor succeeds if connection is successful. The sink endpoint is
//  provided by the node's own server, for storing received posts.                  

static int 
hydra_client_constructor (hydra_client_t *self, const char *peer_endpoint, const char *sink_endpoint, uint32_t timeout);

hydra_client_t *
hydra_client_new (const char *peer_endpoint, const char *sink_endpoint, uint32_t timeout)
{
    hydra_client_t *self = (hydra_client_t *) zmalloc (sizeof (hydra_client_t));
    if (self) {
        zsock_t *backend;
        self->msgpipe = zsys_create_pipe (&backend);
        self->actor = zactor_new (hydra_client, backend);
        if (self->actor)
            self->status = hydra_client_constructor (self, peer_endpoint, sink_endpoint, timeout);
        if (self->status == -1)
            zactor_destroy (&self->actor);
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
            //  Shut down msgpipe first so that client can do clean shutdown,
            //  sending any pending messages and handshaking goodbye to server
            zstr_send (self->msgpipe, "$TERM");
            zsock_wait (self->actor);
            hydra_client_destructor (self);
        }
        zactor_destroy (&self->actor);
        zsock_destroy (&self->msgpipe);
        zstr_free (&self->reason);
        zstr_free (&self->ident);
        zstr_free (&self->subject);
        zstr_free (&self->timestamp);
        zstr_free (&self->parent_id);
        zstr_free (&self->mime_type);
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
                    zsock_recv (self->actor, "i44", &self->status, &self->before, &self->after);
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
                else
                if (streq (reply, "FETCHED")) {
                    zstr_free (&self->ident);
                    zstr_free (&self->subject);
                    zstr_free (&self->timestamp);
                    zstr_free (&self->parent_id);
                    zstr_free (&self->mime_type);
                    zsock_recv (self->actor, "isssss8", &self->status, &self->ident, &self->subject, &self->timestamp, &self->parent_id, &self->mime_type, &self->content_size);
                }
                else
                if (streq (reply, "SKIPPED")) {
                    zsock_recv (self->actor, "i", &self->status);
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

static int 
hydra_client_constructor (hydra_client_t *self, const char *peer_endpoint, const char *sink_endpoint, uint32_t timeout)
{
    assert (self);
    zsock_send (self->actor, "sss4", "CONSTRUCTOR", peer_endpoint, sink_endpoint, timeout);
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
//  Fetch a post from the server. Fetches either a newer post, an older post, or a  
//  fresh post (from newest backwards). Returns 0 if OK, -1 if there was no post to 
//  fetch, or another error. disappears.                                            
//  Returns >= 0 if successful, -1 if interrupted.

int 
hydra_client_fetch (hydra_client_t *self, int which)
{
    assert (self);
    zsock_send (self->actor, "si", "FETCH", which);
    if (s_accept_reply (self, "FETCHED", "SKIPPED", "FAILURE", NULL))
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
//  Return last received before

uint32_t 
hydra_client_before (hydra_client_t *self)
{
    assert (self);
    return self->before;
}


//  ---------------------------------------------------------------------------
//  Return last received after

uint32_t 
hydra_client_after (hydra_client_t *self)
{
    assert (self);
    return self->after;
}


//  ---------------------------------------------------------------------------
//  Return last received reason

const char *
hydra_client_reason (hydra_client_t *self)
{
    assert (self);
    return self->reason;
}


//  ---------------------------------------------------------------------------
//  Return last received ident

const char *
hydra_client_ident (hydra_client_t *self)
{
    assert (self);
    return self->ident;
}


//  ---------------------------------------------------------------------------
//  Return last received subject

const char *
hydra_client_subject (hydra_client_t *self)
{
    assert (self);
    return self->subject;
}


//  ---------------------------------------------------------------------------
//  Return last received timestamp

const char *
hydra_client_timestamp (hydra_client_t *self)
{
    assert (self);
    return self->timestamp;
}


//  ---------------------------------------------------------------------------
//  Return last received parent_id

const char *
hydra_client_parent_id (hydra_client_t *self)
{
    assert (self);
    return self->parent_id;
}


//  ---------------------------------------------------------------------------
//  Return last received mime_type

const char *
hydra_client_mime_type (hydra_client_t *self)
{
    assert (self);
    return self->mime_type;
}


//  ---------------------------------------------------------------------------
//  Return last received content_size

uint64_t 
hydra_client_content_size (hydra_client_t *self)
{
    assert (self);
    return self->content_size;
}
