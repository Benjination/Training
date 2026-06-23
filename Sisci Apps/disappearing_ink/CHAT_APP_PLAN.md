# SISCI Vanishing-Text Chat Plan

This document is a working plan for building the chat app from `rn_check.c`.

## Goal
Build a two-node SISCI chat where:
- active nodes are visible in the UI,
- each keystroke appears on the other node immediately,
- characters disappear after a time window in the same order they arrived.

## Architecture (MVP)
1. Presence segment:
- fixed-size table of `node_id`, `last_heartbeat_ns`, `display_name`.
- each node updates its own slot every 100-250 ms.
- nodes are "online" if heartbeat age is under a timeout.

2. Event segment:
- append-only ring buffer of key events.
- each event stores `seq`, `author_node_id`, `char_or_key`, `created_ns`, `expire_ns`.
- each node tracks a read cursor per peer.

3. Local renderer:
- reconstruct output text by applying events in sequence order.
- remove expired events using `expire_ns`.
- show node list at the top and fading text below.

## Data Types To Add In C
- `presence_slot_t`
- `key_event_t`
- `shared_event_ring_t`
- `local_state_t`

## Phase Plan
1. Stabilize base scaffold (done):
- SISCI init/open/get local node ID.
- create/map local presence segment.
- connect/map remote presence segment.
- keep terminal command loop for iteration.

2. Add presence protocol:
- write heartbeat loop.
- command `nodes` prints online peers with age.

3. Add key event transport:
- non-blocking keyboard capture.
- push one event per key into local ring.
- poll remote ring and merge into local timeline.

4. Add vanish behavior:
- assign `expire_ns = created_ns + ttl_ns`.
- periodic cleanup by sequence order.

5. Add terminal UI:
- split screen: peers + live text + input line.
- redraw at fixed frame rate (20-30 Hz).

## Suggested Initial Constants
- `PRESENCE_SEGMENT_ID = 10`
- `EVENT_SEGMENT_ID = 11`
- `HEARTBEAT_MS = 150`
- `NODE_TIMEOUT_MS = 1200`
- `KEY_TTL_MS = 8000`
- `RING_CAPACITY = 4096`

## Notes
- Keep wire structs fixed-size and packed consistently on all nodes.
- Use monotonic clock for TTL and heartbeat age.
- Start with ASCII input and add special keys (backspace/enter) after basic sync works.
