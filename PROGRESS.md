# PROGRESS

One line per phase. Update the status and date when a gate passes.
See ROADMAP.md for the full spec and gate condition of each phase.

| Phase | Status | Gate passed on | Notes |
|---|---|---|---|
| Part 1: C ramp | done | — | Beej's selected chapters + bridge exercises 1-2 |
| Part 1: cstack tutorial | done | — | Through Part 14 |
| Phase 1: Disk manager + pages | done | 2026-09-13 | Slotted pages / heap file deferred, see log |
| Phase 2: Buffer pool | in progress | — | |
| Phase 3: B+tree index | in progress | — | |
| Phase 4: Catalog + executors | not started | — | |
| Phase 5: SQL frontend | not started | — | |
| Phase 6: Concurrency | not started | — | |
| Phase 7: WAL + recovery | not started | — | |
| Phase 8: Stretch | not started | — | |

## Current focus

Phase 3 — B+tree index.

Gate: point lookups and range scans both work correctly, including after
several splits.

Phase 2 (buffer pool) is functionally in place (`buffer_pool_get_page`,
eviction, error translation) but its own gate test was skipped for now —
no `buffer_pool_create`/`destroy`, no dirty-flag/unpin/flush-on-evict yet.
Picking up B+tree in parallel rather than closing that out first; revisit
Phase 2's gate when those pieces are actually needed.

## Log

- Use this space for short dated notes on design decisions or things to
  revisit — not a full diary, just what a future session (yours or
  Claude's) would need to avoid re-deriving context.

- 2026-09-15: `buffer_pool_get_page` takes one single, coarse-grained
  `static pthread_mutex_t lock` for its entire body — every fetch is fully
  serialized pool-wide, and the lock isn't even tied to a specific
  `BufferPool` instance. Fine for now (no concurrency yet), but once
  Phase 6 lands this should become per-frame/per-bucket latching instead
  of one pool-wide latch, so unrelated fetches don't block each other.

- 2026-09-13: Phase 1 gate (round-trip + out-of-range handling) passed.
  Slotted pages for variable-length tuples and the heap file abstraction
  ((page id, slot) addressing), both listed in Phase 1's "What" section,
  were deliberately deferred rather than built now — moving to the buffer
  pool next, since it sits directly on raw pages via the disk manager.
  Revisit slotted pages/heap file before or during Phase 4 (catalog +
  executors), which needs them.

- 2026-09-08: `disk_manager_allocate_page` does `dm->num_pages++` before
  writing the zeroed page, then decrements it back on write failure. Fine
  for now (single-threaded), but once Phase 6 concurrency lands, another
  thread could observe the incremented `num_pages` — and treat that page
  id as valid — during the window before the write is confirmed or rolled
  back. Will need a lock/atomic update around this increment-then-maybe-
  rollback sequence at that point.
