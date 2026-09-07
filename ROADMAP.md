# ROADMAP

Full version with resource links and the PostgreSQL chapter-mapping table:
see `rdbms_roadmap.pdf` (project docs). This file is the condensed,
Claude Code-readable copy — read the relevant phase here before starting
or reviewing it.

Target: not a literal PostgreSQL clone, but a database with Postgres's
*architecture* at learnable scale: heap storage, buffer pool, B+tree, a
small SQL frontend, MVCC concurrency, WAL/ARIES recovery. ~9-15 months
part-time.

Rule for every phase below: attempt it from the spec myself first. Claude
reviews and points at problems; it does not write the implementation.
See CLAUDE.md.

## Part 1 — C Preparation (complete)

- **C ramp for Java developers.** Beej's Guide to C, selected chapters
  (2-12, 17, 19-20, 22-24). Bridge exercises: growable `IntVec`, struct-to-
  binary-file serialization with padding/offsetof measurement.
  Gate: bridge exercise 2 round-trips clean under ASan/UBSan.
- **Ice-breaker: cstack's "Let's Build a Simple Database."** Slow through
  Part 5 (persistence) and Parts 7-14 (B-tree). Gate: explain
  insert-with-split from memory, tutorial closed.

## Part 2 — Core Build

### Phase 1 — Disk manager + pages (3-4 wks)

What: fixed-size pages via pread/pwrite/fsync, slotted pages for
variable-length tuples, heap file addressed by (page id, slot). Nothing
above this layer touches a file descriptor directly.

Gate: write ~1000 pages, close, reopen with a fresh struct, every page
reads back byte-identical. Out-of-range page ids return an error, not a
crash.

### Phase 2 — Buffer pool (3-4 wks)

What: frames, page table, pin counts, dirty flags, eviction policy (LRU,
then Clock or LRU-K).

Gate: correctly works with a database larger than the configured
buffer-pool memory budget.

### Phase 3 — B+tree index (4-6 wks) — hardest single component

What: node layout on pages, search, insert-with-split, range scans via
leaf sibling links. Lazy deletion is an acceptable shortcut.

Gate: point lookups and range scans both work correctly, including after
several splits.

Watch for: motivation cliff #1 — most people who quit this project quit
somewhere in this phase.

### Phase 4 — Catalog + executors (3-4 wks)

What: system catalog stored in your own tables, Volcano-style iterators
(seq scan, index scan, filter, insert/delete), three join algorithms
(nested-loop, hash, sort-merge), external merge sort. Driven by a C API
with hardcoded plans — no SQL yet.

Gate: a hardcoded query plan with at least one join and one external sort
runs correctly.

### Phase 5 — Minimal SQL frontend (2-3 wks) — keep it small on purpose

What: hand-rolled tokenizer + recursive-descent parser for a small subset
(CREATE TABLE, INSERT, SELECT/FROM/WHERE/ORDER BY, one JOIN), naive
rule-based planner. Timebox hard — this is compiler territory, not the
systems fundamentals the project is for.

Gate: a SELECT with a WHERE clause and one JOIN parses and runs end to
end.

### Phase 6 — Concurrency (6-8 wks) — two distinct layers

What: latches (thread-safe buffer pool, then B+tree latch crabbing), then
transactions (2PL lock manager with deadlock detection, then MVCC —
version chains, snapshots, visibility rules — built to replace it).

Gate: concurrent transactions run correctly under load, completely clean
ThreadSanitizer report.

Watch for: latch crabbing is the trickiest concurrent data structure in
the project — expect to redo it.

### Phase 7 — WAL + recovery (4-6 wks)

What: write-ahead logging, redo/undo records, checkpoints, ARIES-style
restart recovery.

Gate: survives a loop of `kill -9` during active writes, no corruption or
lost committed data.

Watch for: motivation cliff #2 (ARIES) — read the recovery chapter (see
PDF resource map) before writing code, not after getting stuck.

## Part 3 — Stretch & second passes

### Phase 8 — Stretch (open-ended)

Top pick: implement the PostgreSQL wire protocol so real `psql` can
connect. Others: cost-based optimizer, vacuum/GC for MVCC, vectorized
execution.

### Second-pass "implement it two ways" comparisons

Do these after the straight path works, not in parallel:

- Rollback journal vs. WAL (SQLite vs. Postgres)
- Postgres-style MVCC (versions in heap, vacuum-cleaned) vs. undo-log MVCC
  (InnoDB/Oracle-style)
- Heap tables (Postgres) vs. index-organized tables (SQLite/InnoDB)
- Your buffer pool vs. mmap, benchmarked
- B+tree vs. LSM-tree
