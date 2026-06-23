#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>

#include "sisci_types.h"
#include "sisci_api.h"
#include "sisci_error.h"


/* Simplified error handling to keep the code clean. */
#define SISCI_ERROR_CHECK(func_name, error)                              \
if (error != SCI_ERR_OK) {                                               \
    fprintf(stderr, "%s failed - Error code: 0x%x\n", func_name, error); \
    fprintf(stderr, "   %s\n", SCIGetErrorString(error));                \
    return (int)error;                                                   \
}

#define NO_CALLBACK NULL
#define NO_OFFSET 0
#define NO_FLAGS 0
#define DEFAULT_ADAPTER_INDEX 0

#define EVENT_SEGMENT_ID 11

#define ENTER_DISAPPEAR_MS 3000ULL
#define RING_CAPACITY 1024
#define MAX_TEXT_LEN 8192

typedef enum {
    KEY_EVENT_CHAR = 1,
    KEY_EVENT_BACKSPACE = 2,
    KEY_EVENT_ENTER = 3
} key_event_kind_t;

typedef struct {
    uint32_t seq;
    uint32_t author_node_id;
    uint8_t kind;
    uint8_t ch;
    uint16_t reserved;
} key_event_t;

typedef struct {
    volatile uint32_t write_seq;
    key_event_t events[RING_CAPACITY];
} event_ring_t;

typedef struct {
    char ch;
    uint64_t expire_ms;
} visible_char_t;

typedef struct {
    visible_char_t chars[MAX_TEXT_LEN];
    size_t len;
} text_state_t;

typedef struct {
    unsigned int remote_node_id;
} app_config_t;

typedef struct {
    struct termios old_termios;
    bool enabled;
} terminal_mode_t;


//Prototypes
static int parse_args();
static int connect_remote_segment_with_retry();
static void run_diary_loop();
static uint64_t now_ms();
static bool apply_event();
static bool prune_expired();
static void publish_event();
static bool poll_ring();
static int enable_raw_mode();
static void disable_raw_mode();
static void render_ui();


/*
 * Program entry point.
 * Responsibilities:
 * - parse arguments,
 * - initialize SISCI and open descriptor,
 * - create/map local event segment and make it available,
 * - connect/map remote event segment,
 * - start the interactive diary loop,
 * - terminate SISCI on exit.
 */
int main(int argc, char *argv[])
{
    app_config_t cfg;
    int parse_rc = parse_args(argc, argv, &cfg);
    if (parse_rc != 0) {
        fprintf(stderr, "Usage: %s -rn <remote_node_id>\n", argv[0]);
        return EXIT_FAILURE;
    }

    sci_error_t sisci_error;
    sci_desc_t sd;
    sci_local_segment_t local_event_segment;
    sci_map_t local_event_map;
    void *local_event_ptr;
    sci_remote_segment_t remote_event_segment;
    sci_map_t remote_event_map;
    volatile void *remote_event_ptr;
    unsigned int local_node_id = 0;
    size_t segment_size = sizeof(event_ring_t);

    printf("Initializing SISCI...\n");
    SCIInitialize(NO_FLAGS, &sisci_error);
    SISCI_ERROR_CHECK("SCIInitialize", sisci_error);

    SCIOpen(&sd, NO_FLAGS, &sisci_error);
    SISCI_ERROR_CHECK("SCIOpen", sisci_error);

    SCIGetLocalNodeId(0, &local_node_id, NO_FLAGS, &sisci_error);
    SISCI_ERROR_CHECK("SCIGetLocalNodeId", sisci_error);

    SCICreateSegment(
        sd,
        &local_event_segment,
        EVENT_SEGMENT_ID,
        segment_size,
        NO_CALLBACK,
        NULL,
        NO_FLAGS,
        &sisci_error);
    SISCI_ERROR_CHECK("SCICreateSegment(event)", sisci_error);

    SCIPrepareSegment(local_event_segment, DEFAULT_ADAPTER_INDEX, NO_FLAGS, &sisci_error);
    SISCI_ERROR_CHECK("SCIPrepareSegment(event)", sisci_error);

    local_event_ptr = SCIMapLocalSegment(
        local_event_segment,
        &local_event_map,
        NO_OFFSET,
        segment_size,
        NULL,
        NO_FLAGS,
        &sisci_error);
    SISCI_ERROR_CHECK("SCIMapLocalSegment(event)", sisci_error);

    memset(local_event_ptr, 0, segment_size);

    SCISetSegmentAvailable(local_event_segment, DEFAULT_ADAPTER_INDEX, NO_FLAGS, &sisci_error);
    SISCI_ERROR_CHECK("SCISetSegmentAvailable(event)", sisci_error);

    if (connect_remote_segment_with_retry(
            sd,
            cfg.remote_node_id,
            EVENT_SEGMENT_ID,
            &remote_event_segment) != 0) {
        fprintf(stderr, "Could not connect to remote event segment\n");
        return EXIT_FAILURE;
    }

    remote_event_ptr = SCIMapRemoteSegment(
        remote_event_segment,
        &remote_event_map,
        NO_OFFSET,
        segment_size,
        NULL,
        NO_FLAGS,
        &sisci_error);
    SISCI_ERROR_CHECK("SCIMapRemoteSegment(event)", sisci_error);

    printf("Connected: local node %u <-> remote node %u\n", local_node_id, cfg.remote_node_id);
    run_diary_loop(
        local_node_id,
        (event_ring_t *)local_event_ptr,
        (volatile event_ring_t *)remote_event_ptr);

    printf("\nShutting down SISCI...\n");
    SCITerminate();
    return EXIT_SUCCESS;
}

/*
 * Main interactive loop:
 * - polls remote events and local expiry,
 * - redraws only when content changes,
 * - reads local raw keystrokes and publishes each as a SISCI event,
 * - applies local events immediately for responsive local echo.
 * Exits on Ctrl-C/Ctrl-D.
 */
static void run_diary_loop(
    unsigned int local_node_id,
    event_ring_t *local_events,
    volatile event_ring_t *remote_events)
{
    terminal_mode_t tm = {0};
    text_state_t text = {0};
    uint32_t local_seq = local_events->write_seq;
    uint32_t remote_seq = remote_events->write_seq;
    int esc_state = 0;
    bool needs_redraw = true;
    bool did_initial_clear = false;

    if (enable_raw_mode(&tm) != 0) {
        return;
    }

    while (true) {
        fd_set readfds;
        struct timeval tv;
        uint64_t tnow = now_ms();

        if (poll_ring(remote_events, &remote_seq, &text, local_node_id, tnow)) {
            needs_redraw = true;
        }
        if (prune_expired(&text, tnow)) {
            needs_redraw = true;
        }

        if (needs_redraw) {
            render_ui(&text, !did_initial_clear);
            did_initial_clear = true;
            needs_redraw = false;
        }

        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 20000;

        int rv = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
        if (rv <= 0) {
            continue;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds) != 0) {
            unsigned char ch = 0;
            ssize_t nread = read(STDIN_FILENO, &ch, 1);
            if (nread <= 0) {
                continue;
            }

            if (ch == 3 || ch == 4) {
                break;
            }

            key_event_t ev;
            memset(&ev, 0, sizeof(ev));
            ev.author_node_id = local_node_id;

            if (!decode_key(ch, &esc_state, &ev)) {
                continue;
            }

            publish_event(local_events, ev, &local_seq);
            if (apply_event(&text, ev.kind, ev.ch, tnow)) {
                needs_redraw = true;
            }
        }
    }

    disable_raw_mode(&tm);
    printf("\nExiting.\n");
}








/*
 * Parses runtime options into app_config_t.
 * Returns 0 on success and -1 for invalid/missing arguments.
 */
static int parse_args(int argc, char *argv[], app_config_t *cfg)
{
    cfg->remote_node_id = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp("-rn", argv[i]) == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Missing value for -rn\n");
                return -1;
            }
            cfg->remote_node_id = (unsigned int)strtoul(argv[++i], NULL, 10);
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            return -1;
        }
    }

    if (cfg->remote_node_id == 0) {
        fprintf(stderr, "remote_node_id must be non-zero\n");
        return -1;
    }

    return 0;
}

/*
 * Connects to a remote SISCI segment and retries until it becomes available.
 * This allows either node to start first while still establishing the link automatically.
 */
static int connect_remote_segment_with_retry(
    sci_desc_t sd,
    unsigned int remote_node_id,
    unsigned int segment_id,
    sci_remote_segment_t *remote_segment)
{
    sci_error_t sisci_error;
    unsigned int attempt = 0;

    do {
        SCIConnectSegment(
            sd,
            remote_segment,
            remote_node_id,
            segment_id,
            DEFAULT_ADAPTER_INDEX,
            NO_CALLBACK,
            NULL,
            SCI_INFINITE_TIMEOUT,
            NO_FLAGS,
            &sisci_error);

        if (sisci_error == SCI_ERR_OK) {
            return 0;
        }

        if ((attempt % 1000U) == 0U) {
            printf("Waiting for remote segment %u on node %u...\n", segment_id, remote_node_id);
        }

        attempt++;
        usleep(1000);
    } while (true);
}

/*
 * Returns monotonic time in milliseconds.
 * Monotonic time avoids problems from wall-clock changes and is used for message expiry logic.
 */
static uint64_t now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000ULL) + ((uint64_t)ts.tv_nsec / 1000000ULL);
}

/*
 * Applies one logical key event to the local visible text buffer.
 * Behavior:
 * - character: append and keep indefinitely until Enter commits the line,
 * - backspace: remove last visible character,
 * - enter: mark current line (and the newline) to hard-cut after ENTER_DISAPPEAR_MS.
 * Returns true if visible text changed.
 */
static bool apply_event(text_state_t *text, uint8_t kind, uint8_t ch, uint64_t tnow)
{
    if (kind == KEY_EVENT_BACKSPACE) {
        if (text->len > 0) {
            text->len--;
            return true;
        }
        return false;
    }

    if (text->len >= MAX_TEXT_LEN) {
        memmove(&text->chars[0], &text->chars[1], sizeof(text->chars[0]) * (MAX_TEXT_LEN - 1));
        text->len = MAX_TEXT_LEN - 1;
    }

    if (kind == KEY_EVENT_ENTER) {
        uint64_t line_expire_ms = tnow + ENTER_DISAPPEAR_MS;
        size_t i = text->len;

        while (i > 0) {
            i--;
            if (text->chars[i].ch == '\n') {
                i++;
                break;
            }
            text->chars[i].expire_ms = line_expire_ms;
        }

        text->chars[text->len].ch = '\n';
        text->chars[text->len].expire_ms = line_expire_ms;
        text->len++;
        return true;
    }

    if (kind == KEY_EVENT_CHAR) {
        text->chars[text->len].ch = (char)ch;
        text->chars[text->len].expire_ms = UINT64_MAX;
        text->len++;
        return true;
    }

    return false;
}

/*
 * Removes characters from the front while they are expired.
 * This is the hard-cut phase: once the committed line reaches its timeout, it is deleted.
 * Returns true when any character was removed.
 */
static bool prune_expired(text_state_t *text, uint64_t tnow)
{
    bool changed = false;

    while (text->len > 0) {
        if (text->chars[0].expire_ms > tnow) {
            break;
        }

        memmove(&text->chars[0], &text->chars[1], sizeof(text->chars[0]) * (text->len - 1));
        text->len--;
        changed = true;
    }

    return changed;
}

/*
 * Publishes one key event into the local shared ring buffer.
 * Sequence numbers provide ordering for the peer, and a memory barrier ensures the event data
 * is visible before write_seq is updated.
 */
static void publish_event(event_ring_t *ring, key_event_t ev, uint32_t *local_seq)
{
    uint32_t next_seq = (*local_seq) + 1U;
    uint32_t idx = next_seq % RING_CAPACITY;

    ev.seq = next_seq;
    ring->events[idx] = ev;
    __sync_synchronize();
    ring->write_seq = next_seq;

    *local_seq = next_seq;
}

/*
 * Consumes newly written events from the peer ring in sequence order.
 * Events authored by this node are skipped to avoid double-application.
 * If ring consistency is lost (seq mismatch), the reader jumps to writer tail to recover.
 * Returns true if remote events changed the local visible text.
 */
static bool poll_ring(
    const volatile event_ring_t *ring,
    uint32_t *last_seq,
    text_state_t *text,
    uint32_t local_node_id,
    uint64_t tnow)
{
    bool changed = false;

    uint32_t write_seq = ring->write_seq;
    if (write_seq == *last_seq) {
        return false;
    }

    while (*last_seq < write_seq) {
        uint32_t next = (*last_seq) + 1U;
        uint32_t idx = next % RING_CAPACITY;
        key_event_t ev = ring->events[idx];

        if (ev.seq != next) {
            *last_seq = write_seq;
            return changed;
        }

        if (ev.author_node_id == local_node_id) {
            *last_seq = next;
            continue;
        }

        if (apply_event(text, ev.kind, ev.ch, tnow)) {
            changed = true;
        }
        *last_seq = next;
    }

    return changed;
}

/*
 * Puts stdin into raw, non-echo mode so each keystroke is read immediately.
 * This is required for real-time per-keystroke replication.
 */
static int enable_raw_mode(terminal_mode_t *tm)
{
    struct termios raw;

    if (tcgetattr(STDIN_FILENO, &tm->old_termios) != 0) {
        perror("tcgetattr");
        return -1;
    }

    raw = tm->old_termios;
    raw.c_lflag &= (tcflag_t) ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
        perror("tcsetattr");
        return -1;
    }

    tm->enabled = true;
    return 0;
}

/*
 * Restores terminal settings captured by enable_raw_mode.
 * Called during shutdown so the shell is returned to normal behavior.
 */
static void disable_raw_mode(terminal_mode_t *tm)
{
    if (tm->enabled) {
        (void)tcsetattr(STDIN_FILENO, TCSANOW, &tm->old_termios);
        tm->enabled = false;
    }
}

/*
 * Renders only the chat content area using ANSI cursor control.
 * On first draw it clears the full screen; later draws repaint from the top and clear trailing
 * content to avoid visual leftovers from longer prior frames.
 */
static void render_ui(
    const text_state_t *text,
    bool clear_screen)
{
    bool at_line_start = true;

    if (clear_screen) {
        printf("\033[2J\033[H");
    } else {
        printf("\033[H");
    }

    for (size_t i = 0; i < text->len; i++) {
        char ch = text->chars[i].ch;

        if (at_line_start) {
            printf("\033[2K");
            at_line_start = false;
        }

        if (ch == '\n') {
            putchar('\n');
            at_line_start = true;
        } else if (isprint((unsigned char)ch) != 0) {
            putchar(ch);
        }
    }

    if (at_line_start) {
        printf("\033[2K");
    }

    printf("\033[J");
    fflush(stdout);
}

/*
 * Converts a raw input byte stream into a logical key event.
 * Escape sequences (arrows/home/etc.) are swallowed so their bytes do not appear in chat.
 * Returns true only when a usable key event was decoded.
 */
static bool decode_key(unsigned char ch, int *esc_state, key_event_t *ev)
{
    if (*esc_state == 1) {
        if (ch == '[') {
            *esc_state = 2;
            return false;
        }
        *esc_state = 0;
        return false;
    }

    if (*esc_state == 2) {
        if (ch >= '@' && ch <= '~') {
            *esc_state = 0;
        }
        return false;
    }

    if (ch == 27) {
        *esc_state = 1;
        return false;
    }

    if (ch == '\r' || ch == '\n') {
        ev->kind = KEY_EVENT_ENTER;
        ev->ch = '\n';
        return true;
    }

    if (ch == 127 || ch == 8) {
        ev->kind = KEY_EVENT_BACKSPACE;
        ev->ch = 0;
        return true;
    }

    if (isprint(ch) != 0) {
        ev->kind = KEY_EVENT_CHAR;
        ev->ch = ch;
        return true;
    }

    return false;
}

