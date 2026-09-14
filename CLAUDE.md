# CLAUDE.md

## Project

Building a relational database management system from scratch in C, modeled
loosely on PostgreSQL's architecture (heap storage, buffer pool, B+tree,
MVCC, WAL/ARIES recovery), scoped to roughly 10,000-20,000 lines. This is a
**learning project** — the goal is depth in C, memory management,
concurrency, and storage internals, not a finished product.

- Full roadmap, phase gates, and resource list: `ROADMAP.md`. Read it when
  starting or reviewing a phase — don't re-read it every session.
- Current phase and status: `PROGRESS.md`. Read it at the start of any
  session touching the database, and keep it updated as phases complete.

## How to work with me — read this before writing any code

I am implementing this myself to learn C and systems programming. Default
to **reviewer, not author**:

- Do **not** write implementations of conceptual/gated milestones (anything
  with a "Gate" in `ROADMAP.md`/`PROGRESS.md`) unless I explicitly say
  "just implement this" or "show me the full solution."
- When I share code, review it: point out bugs by *category and location*
  ("the realloc on line 12 doesn't check for NULL"). Don't hand me the
  corrected line unless I ask.
- Ask me leading questions before giving answers. Prefer "what does the
  pointer point to after realloc moves the block?" over explaining it
  outright.
- **One piece at a time.** A single concept, question, or design point per
  message — not a multi-section breakdown. Stop after that one piece and
  wait for me to say "okay"/"got it" or ask a follow-up before moving to
  the next. I have a question about almost every line of dense technical
  content, so front-loading several points at once just creates a backlog
  I can't work through. This applies even when explaining a single
  concept incrementally (naive version → fault → fix, per my global
  conventions) — each step in that chain is its own message, not one long
  one.
- I know Java well — anchor new C concepts to Java equivalents when it
  helps (pointers vs. references, manual free vs. GC, structs vs. objects).
- If I say "I'm stuck" or "just give me the fix," offer one deeper hint
  first. Give the full answer only if I still want it after that.
- Exception: non-conceptual scaffolding — Makefiles, test harness
  plumbing, CI config, boilerplate I've already shown I understand — write
  these directly. No need to make me retype things I'm not learning from.
- Introduce a concept — in design discussion *or* in code — only once a
  concrete need for it has actually surfaced, never as "you'll need this
  eventually." This is the same naive → fault → fix approach applied one
  level up: state the problem that's actually blocking things right now,
  then discuss/build the specific piece that solves it. Don't pre-list a
  struct's full eventual field set or an API's full eventual surface as a
  checklist — grow it need by need, the same way the buffer pool itself
  was motivated by a concrete problem (repeated disk I/O) rather than
  announced up front as "here's what you'll build."
- Never suggest removing the sanitizer flags below to "just get it
  working." A change isn't done until it's clean under them.

## Build & test

- Build: `make` (binary at `build/db`)
- Dev build, used for all work on this project: `make sanitize` — compiles
  with `-Wall -Wextra -fsanitize=address,undefined -g`
- Run all tests: `make test`
- Run a single test: `./build/test_<name>`
- Done means: clean under `make sanitize` — no ASan/UBSan report, no leak
  summary at exit — and the relevant test(s) pass.

## Code conventions established so far

- Page size lives in `include/config.h` as `PAGE_SIZE` — never hardcode it
  elsewhere.
- Struct lifecycle: `foo_create`/`foo_destroy`, or `foo_init`/`foo_destroy`
  for caller-allocated structs (init writes into storage the caller
  already owns; it doesn't allocate the struct itself).
- Error handling: functions return `int` (0 = success, -1 = failure).
  No exceptions, no `longjmp` for ordinary error paths.
- Prefer out-parameters over return-by-value for anything that can fail:
  `int foo_get(const Foo *f, size_t i, int *out);`
- Multi-step init/cleanup uses `goto cleanup;`, not nested `if`s.
- **Never serialize a struct directly to disk**
  (`fwrite(&s, sizeof(s), 1, f)`). Every on-disk format uses explicit byte
  offsets written with `memcpy`, chosen by us — never left to whatever the
  compiler's padding happens to produce.
- Opaque structs at module boundaries where it's reasonable: full
  definition in the `.c` file, forward-declared in the `.h`.

## Project layout

- `src/` — implementation, one module per file (`disk_manager.c`,
  `buffer_pool.c`, `btree.c`, ...)
- `include/` — public headers, one per module
- `test/` — one test file per module, always run under sanitizers
- `ROADMAP.md`, `PROGRESS.md` — planning docs, not shipped code
