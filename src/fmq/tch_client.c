/*
 * Project:
 *  ___________      .___________ .__    .__ 
 * \__    ___/____  |__\_   ___ \|  |__ |__|
 *   |    |  \__  \ |  /    \  \/|  |  \|  |
 *   |    |   / __ \|  \     \___|   Y  \  |
 *   |____|  (____  /__|\______  /___|  /__|
 *                \/           \/     \/   
 *
 * Copyright (C) 2021 - 2022, Yan RuiBing, <772166784@qq.com>, et al.
 *
 */
#include <tch_client.h>

//  State machine constants

typedef enum {
    start_state = 1,
    connecting_state = 2,
    connected_state = 3,
    subscribing_state = 4,
    subscribed_state = 5,
    defaults_state = 6
} state_t;

typedef enum {
    NULL_event = 0,
    connect_event = 1,
    connect_error_event = 2,
    ohai_ok_event = 3,
    expired_event = 4,
    set_inbox_event = 5,
    subscribe_event = 6,
    destructor_event = 7,
    subscribe_error_event = 8,
    icanhaz_ok_event = 9,
    send_credit_event = 10,
    cheezburger_event = 11,
    finished_event = 12,
    srsly_event = 13,
    rtfm_event = 14,
    hugz_ok_event = 15,
    bombcmd_event = 16,
    bombmsg_event = 17
} event_t;

//  Names for state machine logging and error reporting
static char *
s_state_name [] = {
    "(NONE)",
    "start",
    "connecting",
    "connected",
    "subscribing",
    "subscribed",
    "defaults"
};

static char *
s_event_name [] = {
    "(NONE)",
    "connect",
    "connect_error",
    "OHAI_OK",
    "expired",
    "set_inbox",
    "subscribe",
    "destructor",
    "subscribe_error",
    "ICANHAZ_OK",
    "send_credit",
    "CHEEZBURGER",
    "finished",
    "SRSLY",
    "RTFM",
    "HUGZ_OK",
    "bombcmd",
    "bombmsg"
};

typedef struct tch_client_s         tch_client_t;
typedef struct tch_client_args_s    tch_client_args_t;
typedef struct tch_sub_s            tch_sub_t;
typedef struct tch_s_client_s       tch_s_client_t;
//typedef struct tch_fmq_client_s     tch_fmq_client_t;

struct tch_sub_s {
    tch_client_t    *client;        //  Pointer to parent client
    char            *inbox;         //  Inbox location
    char            *path;          //  Path we subscribe to
};

struct tch_client_s {
    zsock_t         *cmdpipe;       //  Command pipe to/from caller API
    zsock_t         *msgpipe;       //  Message pipe to/from caller API
    zsock_t         *dealer;        //  Socket to talk to server
    fmq_msg_t       *message;       //  Message to/from server
    tch_client_args_t *args;        //  Arguments from methods
    size_t          credit;         //  Current credit pending
    zfile_t         *file;          //  File we're currently writing
    char            *inbox;         //  Path where files will be stored
    zlist_t         *subs;          //  Our subscriptions
    tch_sub_t       *sub;           //  Subscription we're sending
    int             timeouts;       //  Count the timeouts
};
//  These are the different method arguments we manage automatically
struct tch_client_args_s {
    char        *endpoint;
    uint32_t    timeout;
    char        *path;
};

struct tch_s_client_s {
    tch_client_t client;            //  Application-level client context
    zsock_t     *cmdpipe;           //  Get/send commands from caller API
    zsock_t     *msgpipe;           //  Get/send messages from caller API
    zsock_t     *dealer;            //  Socket to talk to server
    zloop_t     *loop;              //  Listen to pipe and dealer
    fmq_msg_t   *message;           //  Message received or sent
    tch_client_args_t args;         //  Method arguments structure
    bool        connected;          //  True if client is connected
    bool        terminated;         //  True if client is shutdown
    bool        fsm_stopped;        //  "terminate" action called
    size_t      expiry;             //  Expiry timer, msecs
    size_t      heartbeat;          //  Heartbeat timer, msecs
    state_t     state;              //  Current state
    event_t     event;              //  Current event
    event_t     next_event;         //  The next event
    event_t     exception;          //  Exception event, if any
    int         expiry_timer;       //  zloop timer for expiry
    int         wakeup_timer;       //  zloop timer for alarms
    int         heartbeat_timer;    //  zloop timer for heartbeat
    event_t     wakeup_event;       //  Wake up with this event
    char        log_prefix [41];    //  Log prefix string
};

/* Class interface */
struct tch_fmq_client_s {
    zactor_t    *actor;             //  Client actor
    zsock_t     *msgpipe;           //  Pipe for async message flow
    bool        connected;          //  Client currently connected or not
    uint8_t     status;             //  Returned by actor reply
    char        *reason;            //  Returned by actor reply
};

/* Global tracing/animation indicator; we can't use a client method as
 * that only works after construction (which we often want to trace). */
volatile int fmq_client_verbose = false;

static int client_initialize(tch_client_t *self);
static void client_terminate(tch_client_t *self);
static void sub_destroy(tch_sub_t **self_p);
static void s_client_destroy(tch_s_client_t **self_p);
static void s_client_execute(tch_s_client_t *self, event_t event);
static int s_client_handle_wakeup(zloop_t *loop, int timer_id, void *argument);
static int s_client_handle_expiry(zloop_t *loop, int timer_id, void *argument);
static void s_satisfy_pedantic_compilers(void);
static void engine_set_next_event(tch_client_t *client, event_t event);
static void engine_set_exception(tch_client_t *client, event_t event);
static void engine_set_heartbeat(tch_client_t *client, size_t heartbeat);
static void engine_set_expiry (tch_client_t *client, size_t expiry);
static void engine_set_wakeup_event(tch_client_t *client, size_t delay, event_t event);
static void engine_handle_socket(tch_client_t *client, zsock_t *sock, zloop_reader_fn handler);
static void engine_set_connected(tch_client_t *client, bool connected);
static event_t s_protocol_event (tch_s_client_t *self, fmq_msg_t *message);
static void connect_to_server_endpoint(tch_client_t *self);
static void use_connect_timeout(tch_client_t *self);
static void handle_connect_error(tch_client_t *self);
static void stayin_alive(tch_client_t *self);
static void log_access_denied(tch_client_t *self);
static void sync_server_not_present(tch_client_t *self);
static void async_server_not_present(tch_client_t *self);
static void log_protocol_error(tch_client_t *self);
static void connected_to_server(tch_client_t *self);
static void handle_connect_timeout(tch_client_t *self);
static void setup_inbox(tch_client_t *self);
static tch_sub_t *sub_new(tch_client_t *client, char *inbox, char *path);
static void format_icanhaz_command(tch_client_t *self);
static void signal_success(tch_client_t *self);
static void subscribe_failed(tch_client_t *self);
static void handle_connected_timeout(tch_client_t *self);
static void signal_subscribe_success(tch_client_t *self);
static void handle_subscribe_timeout(tch_client_t *self);
static void process_the_patch(tch_client_t *self);
static void refill_credit_as_needed (tch_client_t *self);
static int s_client_handle_cmdpipe(zloop_t *loop, zsock_t *reader, void *argument);
static int s_client_handle_msgpipe(zloop_t *loop, zsock_t *reader, void *argument);
static int s_client_handle_protocol (zloop_t *loop, zsock_t *reader, void *argument);
static void log_invalid_message(tch_client_t *self);
//static uint8_t fmq_client_destructor (tch_fmq_client_t *self);
static int s_accept_reply (tch_fmq_client_t *self, ...);

//  There's no point making these configurable
#define CREDIT_SLICE        1000000
#define CREDIT_MINIMUM      (CREDIT_SLICE * 4) + 1
#define engine_set_timeout  engine_set_expiry

static int 
client_initialize (tch_client_t *self)
{
    //zsys_info("client is initializing");
    self->subs = zlist_new();
    self->credit = 0;
    self->inbox = NULL;
    self->timeouts = 0;

    return 0;
}

/* Free properties and structures for a client instance */
static void
client_terminate(tch_client_t *self)
{
    zsys_info("client_terminate: client is terminating");
    while (zlist_size(self->subs)) {
        tch_sub_t *sub = (tch_sub_t *) zlist_pop(self->subs);
        zsys_debug("destroy sub %s", sub->path);
        sub_destroy(&sub);
    }
    zlist_destroy(&self->subs);
    zsys_debug("client_terminate: subscription list destroyed");
    if (self->inbox) {
        free(self->inbox);
        zsys_debug("client_terminate: inbox freed");
    }
}

static void
sub_destroy(tch_sub_t **self_p)
{
    assert(self_p);

    if (*self_p) {
        tch_sub_t *self = *self_p;
        free(self->inbox);
        free(self->path);
        free(self);
        *self_p = NULL;
    }
}

static void
s_client_destroy(tch_s_client_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        tch_s_client_t *self = *self_p;
        zstr_free(&self->args.endpoint);
        zstr_free(&self->args.path);
        client_terminate(&self->client);
        fmq_msg_destroy(&self->message);
        zsock_destroy(&self->msgpipe);
        zsock_destroy(&self->dealer);
        zloop_destroy(&self->loop);
        free(self);
        *self_p = NULL;
    }
}

static void
s_client_execute(tch_s_client_t *self, event_t event)
{
    self->next_event = event;
    /* Cancel wakeup timer, if any was pending */
    if (self->wakeup_timer) {
        zloop_timer_end(self->loop, self->wakeup_timer);
        self->wakeup_timer = 0;
    }

    while (!self->terminated                    //  Actor is dying
        && !self->fsm_stopped                   //  FSM has finished
        && self->next_event != NULL_event) {
        self->event = self->next_event;
        self->next_event = NULL_event;
        self->exception = NULL_event;

        if (fmq_client_verbose) {
            zsys_debug("%s: %s:", self->log_prefix, s_state_name[self->state]);
            zsys_debug("%s:     %s", self->log_prefix, s_event_name[self->event]);
        }

        switch (self->state) {
        case start_state:
            if (self->event == connect_event) {
                if (!self->exception) {
                    //  connect to server endpoint
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ connect to server endpoint", self->log_prefix);
                    connect_to_server_endpoint(&self->client);
                }
                if (!self->exception) {
                    //  use connect timeout
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ use connect timeout", self->log_prefix);
                    use_connect_timeout(&self->client);
                }
                if (!self->exception) {
                    //  send OHAI
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ send OHAI", self->log_prefix);
                    fmq_msg_set_id(self->message, FMQ_MSG_OHAI);
                    fmq_msg_send(self->message, self->dealer);
                }
                if (!self->exception)
                    self->state = connecting_state;
            } else if (self->event == connect_error_event) {
                if (!self->exception) {
                    //  handle connect error
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ handle connect error", self->log_prefix);
                    handle_connect_error(&self->client);
                }
            } else if (self->event == srsly_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive(&self->client);
                }
                if (!self->exception) {
                    //  log access denied
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ log access denied", self->log_prefix);
                    log_access_denied(&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == rtfm_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive(&self->client);
                }
                if (!self->exception) {
                    //  log invalid message
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ log invalid message", self->log_prefix);
                    log_invalid_message(&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == hugz_ok_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive(&self->client);
                }
            } else if (self->event == bombcmd_event) {
                if (!self->exception) {
                    //  sync server not present
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ sync server not present", self->log_prefix);
                    sync_server_not_present(&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == bombmsg_event) {
                if (!self->exception) {
                    //  async server not present
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ async server not present", self->log_prefix);
                    async_server_not_present(&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else {
                //  Handle unexpected protocol events
                if (!self->exception) {
                    //  log protocol error
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ log protocol error", self->log_prefix);
                    log_protocol_error(&self->client);
                }
            }
            break;
        case connecting_state:
            if (self->event == ohai_ok_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive(&self->client);
                }
                if (!self->exception) {
                    //  connected to server
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ connected to server", self->log_prefix);
                    connected_to_server(&self->client);
                }
                if (!self->exception)
                    self->state = connected_state;
            } else if (self->event == expired_event) {
                if (!self->exception) {
                    //  handle connect timeout
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ handle connect timeout", self->log_prefix);
                    handle_connect_timeout(&self->client);
                }
            } else if (self->event == srsly_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive (&self->client);
                }
                if (!self->exception) {
                    //  log access denied
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ log access denied", self->log_prefix);
                    log_access_denied(&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == rtfm_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive(&self->client);
                }
                if (!self->exception) {
                    //  log invalid message
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ log invalid message", self->log_prefix);
                    log_invalid_message(&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == hugz_ok_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive(&self->client);
                }
            } else if (self->event == bombcmd_event) {
                if (!self->exception) {
                    //  sync server not present
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ sync server not present", self->log_prefix);
                    sync_server_not_present(&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == bombmsg_event) {
                if (!self->exception) {
                    //  async server not present
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ async server not present", self->log_prefix);
                    async_server_not_present(&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else {
                //  Handle unexpected protocol events
                if (!self->exception) {
                    //  log protocol error
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ log protocol error", self->log_prefix);
                    log_protocol_error(&self->client);
                }
            }
            break;
        case connected_state:
            if (self->event == set_inbox_event) {
                if (!self->exception) {
                    //  setup inbox
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ setup inbox", self->log_prefix);
                    setup_inbox(&self->client);
                }
            } else if (self->event == subscribe_event) {
                if (!self->exception) {
                    //  format icanhaz command
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ format icanhaz command", self->log_prefix);
                    format_icanhaz_command(&self->client);
                }
                if (!self->exception) {
                    //  send ICANHAZ
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ send ICANHAZ", self->log_prefix);
                    fmq_msg_set_id(self->message, FMQ_MSG_ICANHAZ);
                    fmq_msg_send(self->message, self->dealer);
                }
                if (!self->exception)
                    self->state = subscribing_state;
            } else if (self->event == destructor_event) {
                if (!self->exception) {
                    //  send KTHXBAI
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ send KTHXBAI", self->log_prefix);
                    fmq_msg_set_id (self->message, FMQ_MSG_KTHXBAI);
                    fmq_msg_send (self->message, self->dealer);
                }
                if (!self->exception) {
                    //  signal success
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ signal success", self->log_prefix);
                    signal_success (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == subscribe_error_event) {
                if (!self->exception) {
                    //  subscribe failed
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ subscribe failed", self->log_prefix);
                    subscribe_failed (&self->client);
                }
            } else if (self->event == expired_event) {
                if (!self->exception) {
                    //  handle connected timeout
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ handle connected timeout", self->log_prefix);
                    handle_connected_timeout (&self->client);
                }
                if (!self->exception) {
                    //  send HUGZ
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ send HUGZ", self->log_prefix);
                    fmq_msg_set_id (self->message, FMQ_MSG_HUGZ);
                    fmq_msg_send (self->message, self->dealer);
                }
            } else if (self->event == srsly_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive (&self->client);
                }
                if (!self->exception) {
                    //  log access denied
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ log access denied", self->log_prefix);
                    log_access_denied (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == rtfm_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive (&self->client);
                }
                if (!self->exception) {
                    //  log invalid message
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ log invalid message", self->log_prefix);
                    log_invalid_message (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == hugz_ok_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive (&self->client);
                }
            } else if (self->event == bombcmd_event) {
                if (!self->exception) {
                    //  sync server not present
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ sync server not present", self->log_prefix);
                    sync_server_not_present (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == bombmsg_event) {
                if (!self->exception) {
                    //  async server not present
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ async server not present", self->log_prefix);
                    async_server_not_present (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else {
                //  Handle unexpected protocol events
                if (!self->exception) {
                    //  log protocol error
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ log protocol error", self->log_prefix);
                    log_protocol_error (&self->client);
                }
            }
            break;
        case subscribing_state:
            if (self->event == icanhaz_ok_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive (&self->client);
                }
                if (!self->exception) {
                    //  signal subscribe success
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ signal subscribe success", self->log_prefix);
                    signal_subscribe_success (&self->client);
                }
                if (!self->exception)
                    self->state = subscribed_state;
            } else if (self->event == expired_event) {
                if (!self->exception) {
                    //  handle subscribe timeout
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ handle subscribe timeout", self->log_prefix);
                    handle_subscribe_timeout (&self->client);
                }
            } else if (self->event == srsly_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive (&self->client);
                }
                if (!self->exception) {
                    //  log access denied
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ log access denied", self->log_prefix);
                    log_access_denied (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == rtfm_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive (&self->client);
                }
                if (!self->exception) {
                    //  log invalid message
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ log invalid message", self->log_prefix);
                    log_invalid_message (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == hugz_ok_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive (&self->client);
                }
            } else if (self->event == bombcmd_event) {
                if (!self->exception) {
                    //  sync server not present
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ sync server not present", self->log_prefix);
                    sync_server_not_present (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == bombmsg_event) {
                if (!self->exception) {
                    //  async server not present
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ async server not present", self->log_prefix);
                    async_server_not_present (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else {
                //  Handle unexpected protocol events
                if (!self->exception) {
                    //  log protocol error
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ log protocol error", self->log_prefix);
                    log_protocol_error (&self->client);
                }
            }
            break;
        case subscribed_state:
            if (self->event == send_credit_event) {
                if (!self->exception) {
                    //  send NOM
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ send NOM", self->log_prefix);
                    fmq_msg_set_id (self->message, FMQ_MSG_NOM);
                    fmq_msg_send (self->message, self->dealer);
                }
            } else if (self->event == cheezburger_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive (&self->client);
                }
                if (!self->exception) {
                    //  process the patch
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ process the patch", self->log_prefix);
                    process_the_patch (&self->client);
                }
                if (!self->exception) {
                    //  refill credit as needed
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ refill credit as needed", self->log_prefix);
                    refill_credit_as_needed (&self->client);
                }
            } else if (self->event == finished_event) {
                if (!self->exception) {
                    //  refill credit as needed
                    if (fmq_client_verbose)
                        zsys_debug("%s:         $ refill credit as needed", self->log_prefix);
                    refill_credit_as_needed(&self->client);
                }
            } else if (self->event == destructor_event) {
                if (!self->exception) {
                    //  send KTHXBAI
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ send KTHXBAI", self->log_prefix);
                    fmq_msg_set_id (self->message, FMQ_MSG_KTHXBAI);
                    fmq_msg_send (self->message, self->dealer);
                }
                if (!self->exception) {
                    //  signal success
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ signal success", self->log_prefix);
                    signal_success (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == expired_event) {
                if (!self->exception) {
                    //  handle connected timeout
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ handle connected timeout", self->log_prefix);
                    handle_connected_timeout (&self->client);
                }
                if (!self->exception) {
                    //  send HUGZ
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ send HUGZ",
                            self->log_prefix);
                    fmq_msg_set_id (self->message, FMQ_MSG_HUGZ);
                    fmq_msg_send (self->message, self->dealer);
                }
            } else if (self->event == srsly_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive (&self->client);
                }
                if (!self->exception) {
                    //  log access denied
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ log access denied", self->log_prefix);
                    log_access_denied (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == rtfm_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive (&self->client);
                }
                if (!self->exception) {
                    //  log invalid message
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ log invalid message", self->log_prefix);
                    log_invalid_message (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == hugz_ok_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive (&self->client);
                }
            } else if (self->event == bombcmd_event) {
                if (!self->exception) {
                    //  sync server not present
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ sync server not present", self->log_prefix);
                    sync_server_not_present (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == bombmsg_event) {
                if (!self->exception) {
                    //  async server not present
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ async server not present", self->log_prefix);
                    async_server_not_present (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else {
                //  Handle unexpected protocol events
                if (!self->exception) {
                    //  log protocol error
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ log protocol error", self->log_prefix);
                    log_protocol_error (&self->client);
                }
            }
            break;
        case defaults_state:
            if (self->event == srsly_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive (&self->client);
                }
                if (!self->exception) {
                    //  log access denied
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ log access denied", self->log_prefix);
                    log_access_denied (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == rtfm_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive (&self->client);
                }
                if (!self->exception) {
                    //  log invalid message
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ log invalid message", self->log_prefix);
                    log_invalid_message (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == hugz_ok_event) {
                if (!self->exception) {
                    //  stayin alive
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ stayin alive", self->log_prefix);
                    stayin_alive (&self->client);
                }
            } else if (self->event == bombcmd_event) {
                if (!self->exception) {
                    //  sync server not present
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ sync server not present", self->log_prefix);
                    sync_server_not_present (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else if (self->event == bombmsg_event) {
                if (!self->exception) {
                    //  async server not present
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ async server not present", self->log_prefix);
                    async_server_not_present (&self->client);
                }
                if (!self->exception) {
                    //  terminate
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ terminate", self->log_prefix);
                    self->fsm_stopped = true;
                }
            } else {
                //  Handle unexpected protocol events
                if (!self->exception) {
                    //  log protocol error
                    if (fmq_client_verbose)
                        zsys_debug ("%s:         $ log protocol error", self->log_prefix);
                    log_protocol_error (&self->client);
                }
            }
            break;
        }
        //  If we had an exception event, interrupt normal programming
        if (self->exception) {
            if (fmq_client_verbose)
                zsys_debug("%s:         ! %s", self->log_prefix, s_event_name [self->exception]);
            self->next_event = self->exception;
        } else if (fmq_client_verbose) {
            zsys_debug("%s:         > %s", self->log_prefix, s_state_name [self->state]);
        }
    }
}

/* zloop callback when client expiry timeout expires */
static int 
s_client_handle_expiry(zloop_t *loop, int timer_id, void *argument)
{
    tch_s_client_t *self = (tch_s_client_t *)argument;

    s_client_execute(self, expired_event);
    if (self->terminated)
        return -1;

    if (self->expiry > 0)
        self->expiry_timer = zloop_timer(loop, self->expiry, 1, s_client_handle_expiry, self);
    
    return 0;
}

/* Pedantic compilers don't like unused functions, so we call the whole
 * API, passing null references. It's nasty and horrid and sufficient.*/
static void 
s_satisfy_pedantic_compilers(void)
{
    engine_set_next_event(NULL, NULL_event);
    engine_set_exception(NULL, NULL_event);
    engine_set_heartbeat(NULL, 0);
    engine_set_expiry(NULL, 0);
    engine_set_wakeup_event(NULL, 0, NULL_event);
    engine_handle_socket(NULL, 0, NULL);
    engine_set_connected(NULL, 0);
}

/* Set the next event, needed in at least one action in an internal
 * state; otherwise the state machine will wait for a message on the
 * dealer socket and treat that as the event.*/
static void
engine_set_next_event(tch_client_t *client, event_t event)
{
    if (client) {
        tch_s_client_t *self = (tch_s_client_t *) client;
        self->next_event = event;
    }
}

/* Raise an exception with 'event', halting any actions in progress.
 * Continues execution of actions defined for the exception event.*/

static void
engine_set_exception(tch_client_t *client, event_t event)
{
    if (client) {
        tch_s_client_t *self = (tch_s_client_t *)client;
        self->exception = event;
    }
}

/* Set a heartbeat timer. The interval is in msecs and must be
 * non-zero. The state machine must handle the "heartbeat" event.
 * The heartbeat happens every interval no matter what traffic the
 * client is sending or receiving.*/

static void
engine_set_heartbeat(tch_client_t *client, size_t heartbeat)
{
    if (client) {
        tch_s_client_t *self = (tch_s_client_t *)client;
        self->heartbeat = heartbeat;
    }
}

/* Set expiry timer. Setting a non-zero expiry causes the state machine
 * to receive an "expired" event if is no incoming traffic for that many
 * milliseconds. This cycles over and over until/unless the code sets a
 * zero expiry. The state machine must handle the "expired" event.*/
static void
engine_set_expiry(tch_client_t *client, size_t expiry)
{
    if (client) {
        tch_s_client_t *self = (tch_s_client_t *) client;
        self->expiry = expiry;
        if (self->expiry_timer) {
            zloop_timer_end(self->loop, self->expiry_timer);
            self->expiry_timer = 0;
        }
        if (self->expiry)
            self->expiry_timer = zloop_timer(self->loop, self->expiry, 1, s_client_handle_expiry, self);
    }
}

/* Set wakeup alarm after 'delay' msecs. The next state should handle the
 * wakeup event. The alarm is cancelled on any other event.*/
static void
engine_set_wakeup_event(tch_client_t *client, size_t delay, event_t event)
{
    if (client) {
        tch_s_client_t *self = (tch_s_client_t *)client;
        if (self->wakeup_timer) {
            zloop_timer_end(self->loop, self->wakeup_timer);
            self->wakeup_timer = 0;
        }
        self->wakeup_timer = zloop_timer(self->loop, delay, 1, s_client_handle_wakeup, self);
        self->wakeup_event = event;
    }
}

/* zloop callback when client wakeup timer expires */
static int
s_client_handle_wakeup(zloop_t *loop, int timer_id, void *argument)
{
    tch_s_client_t *self = (tch_s_client_t *)argument;
    s_client_execute(self, self->wakeup_event);
    return 0;
}

/* Poll socket for activity, invoke handler on any received message.
 * Handler must be a CZMQ zloop_fn function; receives client as arg.*/

static void
engine_handle_socket(tch_client_t *client, zsock_t *sock, zloop_reader_fn handler)
{
    if (client && sock) {
        tch_s_client_t *self = (tch_s_client_t *)client;
        if (handler != NULL) {
            int rc = zloop_reader(self->loop, sock, handler, self);
            assert (rc == 0);
            zloop_reader_set_tolerant(self->loop, sock);
        } else {
            zloop_reader_end(self->loop, sock);
        }
    }
}

/* Set connected to true/false. The client must call this if it wants
 * to provide the API with the connected status.*/
static void
engine_set_connected(tch_client_t *client, bool connected)
{
    if (client) {
        tch_s_client_t *self = (tch_s_client_t *) client;
        self->connected = connected;
    }
}

/* Generic methods on protocol messages
 * TODO: replace with lookup table, since ID is one byte */
static event_t
s_protocol_event (tch_s_client_t *self, fmq_msg_t *message)
{
    assert(message);
    switch (fmq_msg_id (message)) {
    case FMQ_MSG_OHAI_OK:
        return ohai_ok_event;
        break;
    case FMQ_MSG_ICANHAZ_OK:
        return icanhaz_ok_event;
        break;
    case FMQ_MSG_CHEEZBURGER:
        return cheezburger_event;
        break;
    case FMQ_MSG_HUGZ_OK:
        return hugz_ok_event;
        break;
    case FMQ_MSG_SRSLY:
        return srsly_event;
        break;
    case FMQ_MSG_RTFM:
        return rtfm_event;
        break;
    default:
        zsys_error("%s: unknown command %s, halting", self->log_prefix, fmq_msg_command(message));
        self->terminated = true;
        return NULL_event;
    }
}

/* connect_to_server_endpoint */
static void
connect_to_server_endpoint(tch_client_t *self)
{
    if (zsock_connect(self->dealer, "%s", self->args->endpoint)) {
        engine_set_exception(self, connect_error_event);
        zsys_warning("could not connect to %s", self->args->endpoint);
    }
}

/* use_connect_timeout */
static void
use_connect_timeout(tch_client_t *self)
{
    engine_set_timeout(self, self->args->timeout);
}

/* handle_connect_error */
static void
handle_connect_error(tch_client_t *self)
{
    zsys_warning("unable to connect to the server");
    engine_set_next_event(self, bombcmd_event);
}

/* stayin_alive */
static void
stayin_alive(tch_client_t *self)
{
    self->timeouts = 0;
}

/* log_access_denied */
static void
log_access_denied(tch_client_t *self)
{
    zsys_warning("##### access was denied #####");
}

/* log_invalid_message */
static void
log_invalid_message(tch_client_t *self)
{
    zsys_error("????? invalid message ?????");
    fmq_msg_print(self->message);
}

/* sync_server_not_present */
static void
sync_server_not_present(tch_client_t *self)
{
    zsock_send(self->cmdpipe, "sis", "FAILURE", -1, "server is not reachable");
}

/* async_server_not_present */
static void
async_server_not_present(tch_client_t *self)
{
    zsock_send(self->msgpipe, "ss", "DISCONNECT", "server is not reachable");
}

static void
log_protocol_error(tch_client_t *self)
{
    zsys_error("***** protocol error *****");
    fmq_msg_print(self->message);
}

/* connected_to_server */
static void
connected_to_server(tch_client_t *self)
{
    //zsys_debug("connected to server");
    zsock_send(self->cmdpipe, "si", "SUCCESS", 0);
}

/* handle_connect_timeout */
static void
handle_connect_timeout(tch_client_t *self)
{
    if (self->timeouts <= 3)
        self->timeouts++;
    else {
        zsys_warning("server did not respond to the request to communicate");
        engine_set_next_event(self, bombcmd_event);
    }
}

/* setup_inbox */
static void
setup_inbox(tch_client_t *self)
{
    if (!self->inbox) {
        self->inbox = strdup(self->args->path);
        zsock_send(self->cmdpipe, "si", "SUCCESS", 0);
    } else
        zsock_send(self->cmdpipe, "sis", "FAILURE", -1, "inbox already set");
}

/* format_icanhaz_command */
static void
format_icanhaz_command(tch_client_t *self)
{
    if (!self->inbox) {
        engine_set_exception(self, subscribe_error_event);
        zsys_error("can't subscribe without inbox set");
        return;
    }

    char *path = strdup(self->args->path);
    if (*path != '/') {
        engine_set_exception(self, subscribe_error_event);
        zsys_error("unable to subscribe path %s, must start with /", path);
        free(path);
        return;
    }

    self->sub = (tch_sub_t *) zlist_first (self->subs);
    while (self->sub) {
        if (streq (path, self->sub->path)) {
            zsys_warning ("already subscribed to %s", path);
            free (path);
            return;
        }
        self->sub = (tch_sub_t *) zlist_next (self->subs);
    }
    self->sub = sub_new (self, self->inbox, path);
    zlist_append (self->subs, self->sub);
    //zsys_debug ("%s added to subscription list", path);
    free (path);

    fmq_msg_set_path(self->message, self->sub->path);
}

static tch_sub_t *
sub_new(tch_client_t *client, char *inbox, char *path)
{
    tch_sub_t *self = (tch_sub_t *)zmalloc(sizeof(tch_sub_t));
    self->client = client;
    self->inbox = strdup(inbox);
    self->path = strdup(path);
    return self;
}

/* signal_success */
static void
signal_success(tch_client_t *self)
{
    zsock_send(self->cmdpipe, "si", "SUCCESS", 0);
}

/* subscribe_failed */
static void
subscribe_failed(tch_client_t *self)
{
    zsock_send(self->cmdpipe, "sis", "FAILURE", -1, "subscription failed");
}

/* handle_connected_timeout */
static void
handle_connected_timeout(tch_client_t *self)
{
    if (self->timeouts <= 3)
        self->timeouts++;
    else
        engine_set_next_event(self, bombmsg_event);
}

/* signal_subscribe_success */
static void
signal_subscribe_success(tch_client_t *self)
{
    zsock_send (self->cmdpipe, "si", "SUCCESS", 0);
    size_t credit_to_send = 0;
    while (self->credit < CREDIT_MINIMUM) {
        credit_to_send += CREDIT_SLICE;
        self->credit += CREDIT_SLICE;
    }
    if (credit_to_send) {
        fmq_msg_set_credit (self->message, credit_to_send);
        engine_set_next_event (self, send_credit_event);
    }
}

/* handle_subscribe_timeout */
static void
handle_subscribe_timeout(tch_client_t *self)
{
    if (self->timeouts <= 3)
        self->timeouts++;
    else {
        zsys_warning ("server did not respond to subscription request");
        engine_set_next_event (self, bombcmd_event);
    }
}

/* process_the_patch */
static void
process_the_patch(tch_client_t *self)
{
    const char *filename = fmq_msg_filename(self->message);

    if (*filename != '/') {
        zsys_error("filename did not start with a \'/\'");
        return;
    }

    tch_sub_t *substr = (tch_sub_t *)zlist_first(self->subs);
    int found = 0;
    while (substr) {
        if (!strncmp(filename, substr->path, strlen(substr->path))) {
            filename += strlen(substr->path);
            //zsys_debug("subscription found for %s", filename);
            found = 1;
            break;
        }
        substr = (tch_sub_t *)zlist_next(self->subs);
    }
    if (!found) {
        zsys_debug("subscription not found for %s", filename);
        return;
    }

    if ('/' == *filename) filename++;

    if (fmq_msg_operation(self->message) == FMQ_MSG_FILE_CREATE) {
        if (self->file == NULL) {
            //zsys_debug("creating file object for %s/%s", self->inbox, filename);
            self->file = zfile_new(self->inbox, filename);
            if (zfile_output(self->file)) {
                zsys_warning("unable to write to file %s/%s", self->inbox, filename);
                //  File not writeable, skip patch
                zfile_destroy(&self->file);
                return;
            }
        }
        //  Try to write, ignore errors in this version
        zchunk_t *chunk = fmq_msg_chunk(self->message);
        if (zchunk_size(chunk) > 0) {
            //zsys_debug("writing chunk at offset %u of %s/%s",fmq_msg_offset(self->message), self->inbox, filename);
            zfile_write(self->file, chunk, fmq_msg_offset(self->message));
            self->credit -= zchunk_size(chunk);
        } else {
            //  Zero-sized chunk means end of file, so report back to caller
            //  Communicate back to caller via the msgpipe
            //zsys_debug("file complete %s/%s", self->inbox, filename);
            zsock_send(self->msgpipe, "sss", "FILE UPDATED", self->inbox, filename);
            zfile_destroy(&self->file);
        }
    } else if (fmq_msg_operation(self->message) == FMQ_MSG_FILE_DELETE) {
        zsys_debug("delete %s/%s", self->inbox, filename);
        zfile_t *file = zfile_new(self->inbox, filename);
        zfile_remove(file);
        zfile_destroy(&file);

        //  Report file deletion back to caller
        //  Notify the caller of deletion
        zsock_send(self->msgpipe, "sss", "FILE DELETED", self->inbox, filename);
    }
}

/* refill_credit_as_needed */
static void
refill_credit_as_needed(tch_client_t *self)
{
    //zsys_debug("refill credit as needed");
    size_t credit_to_send = 0;
    while (self->credit < CREDIT_MINIMUM) {
        credit_to_send += CREDIT_SLICE;
        self->credit += CREDIT_SLICE;
    }
    if (credit_to_send) {
        fmq_msg_set_credit(self->message, credit_to_send);
        engine_set_next_event(self, send_credit_event);
    }
}

/* Handle command pipe to/from calling API */
static int
s_client_handle_cmdpipe(zloop_t *loop, zsock_t *reader, void *argument)
{
    tch_s_client_t *self = (tch_s_client_t *) argument;

    char *method = zstr_recv(self->cmdpipe);
    if (!method)
        return -1;
    
    if (fmq_client_verbose)
        zsys_debug("%s:     API command=%s", self->log_prefix, method);

    if (streq(method, "$TERM")) {
        self->terminated = true;    //  Shutdown the engine
    } else if (streq(method, "$CONNECTED")) {
        zsock_send(self->cmdpipe, "i", self->connected);
    } else if (streq(method, "CONNECT")) {
        zstr_free(&self->args.endpoint);
        zsock_recv(self->cmdpipe, "s4", &self->args.endpoint, &self->args.timeout);
        s_client_execute(self, connect_event);
    } else if (streq(method, "DESTRUCTOR")) {
        s_client_execute(self, destructor_event);
    } else if (streq(method, "SUBSCRIBE")) {
        zstr_free(&self->args.path);
        zsock_recv(self->cmdpipe, "s", &self->args.path);
        s_client_execute(self, subscribe_event);
    } else if (streq(method, "SET INBOX")) {
        zstr_free(&self->args.path);
        zsock_recv(self->cmdpipe, "s", &self->args.path);
        s_client_execute(self, set_inbox_event);
    }
    //  Cleanup pipe if any argument frames are still waiting to be eaten
    if (zsock_rcvmore(self->cmdpipe)) {
        zsys_error("%s: trailing API command frames (%s)", self->log_prefix, method);
        zmsg_t *more = zmsg_recv(self->cmdpipe);
        zmsg_print(more);
        zmsg_destroy(&more);
    }
    zstr_free(&method);
    return self->terminated? -1: 0;
}

/* Handle message pipe to/from calling API */
static int
s_client_handle_msgpipe (zloop_t *loop, zsock_t *reader, void *argument)
{
    tch_s_client_t *self = (tch_s_client_t *) argument;

    //  We will process as many messages as we can, to reduce the overhead
    //  of polling and the reactor:
    while (zsock_events(self->msgpipe) & ZMQ_POLLIN) {
        char *method = zstr_recv(self->msgpipe);
        if (!method)
            return -1;              //  Interrupted; exit zloop
        if (fmq_client_verbose)
            zsys_debug("%s:     API message=%s", self->log_prefix, method);

        //  Front-end shuts down msgpipe before cmdpipe, this little
        //  handshake just ensures all traffic on the msgpipe has been
        //  flushed before the calling thread continues with destroying
        //  the actor.
        if (streq(method, "$FLUSH"))
            zsock_signal (self->cmdpipe, 0);
        //  Cleanup pipe if any argument frames are still waiting to be eaten
        if (zsock_rcvmore(self->msgpipe)) {
            zsys_error("%s: trailing API message frames (%s)", self->log_prefix, method);
            zmsg_t *more = zmsg_recv(self->msgpipe);
            zmsg_print(more);
            zmsg_destroy(&more);
        }
        zstr_free(&method);
    }
    return 0;
}

/* Handle a message (a protocol reply) from the server */
static int
s_client_handle_protocol (zloop_t *loop, zsock_t *reader, void *argument)
{
    tch_s_client_t *self = (tch_s_client_t *) argument;

    //  We will process as many messages as we can, to reduce the overhead
    //  of polling and the reactor:
    while (zsock_events (self->dealer) & ZMQ_POLLIN) {
        if (fmq_msg_recv (self->message, self->dealer))
            return -1;              //  Interrupted; exit zloop

        //  Any input from server counts as activity
        if (self->expiry_timer) {
            zloop_timer_end(self->loop, self->expiry_timer);
            self->expiry_timer = 0;
        }
        //  Reset expiry timer if expiry timeout not zero
        if (self->expiry)
            self->expiry_timer = zloop_timer (self->loop, self->expiry, 1, s_client_handle_expiry, self);
        s_client_execute(self, s_protocol_event (self, self->message));
        if (self->terminated)
            return -1;
    }
    return 0;
}

//  Create a new client connection

static tch_s_client_t *
s_client_new(zsock_t *cmdpipe, zsock_t *msgpipe)
{
    tch_s_client_t *self = (tch_s_client_t *) zmalloc(sizeof(tch_s_client_t));
    if (self) {
        assert((tch_s_client_t *) &self->client == self);
        self->cmdpipe = cmdpipe;
        self->msgpipe = msgpipe;
        self->state = start_state;
        self->event = NULL_event;
        snprintf (self->log_prefix, sizeof (self->log_prefix),
            "%6d:%-33s", randof (1000000), "fmq_client");
        self->dealer = zsock_new(ZMQ_DEALER);
        if (self->dealer)
            self->message = fmq_msg_new();
        if (self->message)
            self->loop = zloop_new();
        if (self->loop) {
            //  Give application chance to initialize and set next event
            self->client.cmdpipe = self->cmdpipe;
            self->client.msgpipe = self->msgpipe;
            self->client.dealer = self->dealer;
            self->client.message = self->message;
            self->client.args = &self->args;
            if (client_initialize (&self->client))
                s_client_destroy(&self);
        }
        else
            s_client_destroy(&self);
    }
    s_satisfy_pedantic_compilers();
    return self;
}

//  Get valid reply from actor; discard replies that does not match. Current
//  implementation filters on first frame of message. Blocks until a valid
//  reply is received, and properties can be loaded from it. Returns 0 if
//  matched, -1 if interrupted or timed-out.
static int
s_accept_reply (tch_fmq_client_t *self, ...)
{
    assert(self);
    while (!zsys_interrupted) {
        char *reply = zstr_recv(self->actor);
        if (!reply)
            break;              //  Interrupted or timed-out

        va_list args;
        va_start (args, self);
        char *filter = va_arg(args, char *);
        while (filter) {
            if (streq(reply, filter)) {
                if (streq (reply, "SUCCESS")) {
                    zsock_recv(self->actor, "1", &self->status);
                } else if (streq(reply, "FAILURE")) {
                    zstr_free (&self->reason);
                    zsock_recv (self->actor, "1s", &self->status, &self->reason);
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

/* This is the client actor, which polls its two sockets and processes
 *incoming messages */
void
fmq_client(zsock_t *cmdpipe, void *msgpipe)
{
    //  Initialize
    tch_s_client_t *self = s_client_new(cmdpipe, (zsock_t *) msgpipe);
    if (self) {
        zsock_signal(cmdpipe, 0);

        //  Set up handler for the sockets the client uses
        engine_handle_socket((tch_client_t *) self, self->cmdpipe, s_client_handle_cmdpipe);
        engine_handle_socket((tch_client_t *) self, self->msgpipe, s_client_handle_msgpipe);
        engine_handle_socket((tch_client_t *) self, self->dealer, s_client_handle_protocol);

        //  Run reactor until there's a termination signal
        zloop_start(self->loop);

        //  Reactor has ended
        s_client_destroy(&self);
    } else
        zsock_signal(cmdpipe, -1);
}

/* Create a new fmq_client */
tch_fmq_client_t *
fmq_client_new(void)
{
    tch_fmq_client_t *self = (tch_fmq_client_t *) zmalloc(sizeof(tch_fmq_client_t));
    if (self) {
        zsock_t *backend;
        self->msgpipe = zsys_create_pipe(&backend);
        if (self->msgpipe)
            self->actor = zactor_new(fmq_client, backend);
        if (!self->actor)
            fmq_client_destroy(&self);
    }
    return self;
}

void
fmq_client_destroy(tch_fmq_client_t **self_p)
{
    assert(self_p);
    if (*self_p) {
        tch_fmq_client_t *self = *self_p;
        if (self->actor && !zsys_interrupted) {
            //  Before destroying the actor we have to flush any pending
            //  traffic on the msgpipe, otherwise it gets lost in a fire and
            //  forget scenario. We do this by sending $FLUSH to the msgpipe
            //  and waiting for a signal back on the cmdpipe.
            if (zstr_send(self->msgpipe, "$FLUSH") == 0)
                zsock_wait(self->actor);
            fmq_client_destructor(self);
        }
        zactor_destroy(&self->actor);
        zsock_destroy(&self->msgpipe);
        zstr_free(&self->reason);
        free(self);
        *self_p = NULL;
    }
}

/* Disconnect from server.                                                         
 * Returns >= 0 if successful, -1 if interrupted.*/
uint8_t 
fmq_client_destructor(tch_fmq_client_t *self)
{
    assert(self);

    zsock_send(self->actor, "s", "DESTRUCTOR");
    if (s_accept_reply(self, "SUCCESS", "FAILURE", NULL))
        return -1;              //  Interrupted or timed-out
    return self->status;
}

/* Connect to server endpoint, with specified timeout in msecs (zero means wait    
 * forever). Connect succeeds if connection is successful.                         
 * Returns >= 0 if successful, -1 if interrupted. */
uint8_t 
fmq_client_connect (tch_fmq_client_t *self, const char *endpoint, uint32_t timeout)
{
    assert (self);

    zsock_send (self->actor, "ss4", "CONNECT", endpoint, timeout);
    if (s_accept_reply (self, "SUCCESS", "FAILURE", NULL))
        return -1;              //  Interrupted or timed-out
    return self->status;
}

/* Subscribe to a directory on the server, directory specified by path.            
 * Returns >= 0 if successful, -1 if interrupted.*/
uint8_t 
fmq_client_subscribe (tch_fmq_client_t *self, const char *path)
{
    assert (self);

    zsock_send (self->actor, "ss", "SUBSCRIBE", path);
    if (s_accept_reply (self, "SUCCESS", "FAILURE", NULL))
        return -1;              //  Interrupted or timed-out
    return self->status;
}

/* Tell the api where to store files. This should be done before subscribing to    
 * anything.                                                                       
 * Returns >= 0 if successful, -1 if interrupted.*/
uint8_t 
fmq_client_set_inbox(tch_fmq_client_t *self, const char *path)
{
    assert (self);

    zsock_send (self->actor, "ss", "SET INBOX", path);
    if (s_accept_reply (self, "SUCCESS", "FAILURE", NULL))
        return -1;              //  Interrupted or timed-out
    return self->status;
}

/* Return last received status */
uint8_t 
fmq_client_status (tch_fmq_client_t *self)
{
    assert (self);
    return self->status;
}

/* Return last received reason */
const char *
fmq_client_reason (tch_fmq_client_t *self)
{
    assert (self);
    return self->reason;
}

/*  Return true if client is currently connected, else false. Note that the
 *  client will automatically re-connect if the server dies and restarts after
 *  a successful first connection. */
bool
fmq_client_connected(tch_fmq_client_t *self)
{
    assert (self);
    int connected;
    zsock_send (self->actor, "s", "$CONNECTED");
    zsock_recv (self->actor, "i", &connected);
    return connected == 1;
}

//  Return actor, when caller wants to work with multiple actors and/or
//  input sockets asynchronously.

zactor_t *
fmq_client_actor(tch_fmq_client_t *self)
{
    assert (self);
    return self->actor;
}

//  Return message pipe for asynchronous message I/O. In the high-volume case,
//  we send methods and get replies to the actor, in a synchronous manner, and
//  we send/recv high volume message data to a second pipe, the msgpipe. In
//  the low-volume case we can do everything over the actor pipe, if traffic
//  is never ambiguous.

zsock_t *
fmq_client_msgpipe(tch_fmq_client_t *self)
{
    assert (self);
    return self->msgpipe;
}


