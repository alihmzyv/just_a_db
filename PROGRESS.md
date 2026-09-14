# PROGRESS

One line per phase. Update the status and date when a gate passes.
See ROADMAP.md for the full spec and gate condition of each phase.

| Phase | Status | Gate passed on | Notes |
|---|---|---|---|
| Part 1: C ramp | done | — | Beej's selected chapters + bridge exercises 1-2 |
| Part 1: cstack tutorial | done | — | Through Part 14 |
| Phase 1: Disk manager + pages | done | 2026-09-13 | Slotted pages / heap file deferred, see log |
| Phase 2: Buffer pool | in progress | — | |
| Phase 3: B+tree index | not started | — | |
| Phase 4: Catalog + executors | not started | — | |
| Phase 5: SQL frontend | not started | — | |
| Phase 6: Concurrency | not started | — | |
| Phase 7: WAL + recovery | not started | — | |
| Phase 8: Stretch | not started | — | |

## Current focus

Phase 2 — Buffer pool.

Gate: correctly works with a database larger than the configured
buffer-pool memory budget.

## Log

- Use this space for short dated notes on design decisions or things to
  revisit — not a full diary, just what a future session (yours or
  Claude's) would need to avoid re-deriving context.

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
