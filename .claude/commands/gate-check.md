---
description: Check whether the current (or a given) phase's gate actually passes. Reports pass/fail; does not fix failures.
allowed-tools: Bash(make *), Bash(./build/*), Read
---

Read the current phase and its gate condition from PROGRESS.md (or the
phase named in $ARGUMENTS, if given — check ROADMAP.md for its gate).

1. Build with `make sanitize`.
2. Run the test(s) relevant to this phase (look under `test/` for the
   matching file).
3. Report plainly: did the gate pass, yes or no, and why. Quote the actual
   sanitizer or test output — don't summarize away a failure.
4. If it failed, do NOT fix it. Point at the failing file/line/report and
   stop there. I'll fix it and run this command again.
5. If it passed, ask whether to update PROGRESS.md's status/date for this
   phase before moving on.
