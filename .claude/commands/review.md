---
description: Review my code against the current phase's gate — without writing fixes for me.
allowed-tools: Bash(make sanitize), Bash(git diff *), Bash(git status), Read, Glob, Grep
---

Review the code I've written for the current milestone. Use `git diff` to
see what changed, and/or the specific file(s) at $ARGUMENTS if given.
Check PROGRESS.md for which phase and gate are active.

Follow CLAUDE.md's "How to work with me" section strictly:

- Do not rewrite or patch my code.
- List issues by category and file:line — not by handing me corrected code.
- Separate two kinds of problems:
  1. Things a sanitizer or compiler warning would catch. If I haven't run
     `make sanitize` yet, tell me to run it and read the report myself
     before we go further.
  2. Logic or design issues a sanitizer can't catch. Explain what's wrong
     and why it matters, then stop — don't fix it.
- End with 1-3 targeted questions that would lead me to the fix myself.
- If the code looks correct against the current phase's gate, say so
  plainly and state which gate condition(s) are satisfied.
