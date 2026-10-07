---
id: L-20261007T150000Z-sandbox-no-pip-no-sudo
title: Claude Code cloud sandbox has no pip, no ensurepip, no sudo and no cmake
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: Medium
tags: [tech:python, tech:cmake, phase:intake, kind:environment]
recorded_by: planner
run: gen-20261007T145241Z-trumpet-vst-plan
recorded_at: 2026-10-07T15:00:00Z
---
## Trigger
Planning or implementing in a Claude Code cloud container (Ubuntu 24.04, user `claude`). Signals: `cmake: command not found`, `python3 -m venv` fails with "Failing command: .../venv/bin/python3", `No module named pip`, `sudo: a password is required`.
## What went wrong
Tried `python3 -m venv venv && venv/bin/pip install cmake ninja`; venv creation failed because ensurepip is not installed, so neither pip nor cmake was available.
## Correction
`python3 -m venv --without-pip <venv>`, then `curl -sSL https://bootstrap.pypa.io/get-pip.py | <venv>/bin/python -`; CMake and Ninja from their GitHub release binaries (`cmake-<v>-linux-x86_64.tar.gz`, `ninja-linux.zip`). System -dev packages: `apt-get download` + `dpkg -x` into a local prefix, or build in CI.
## Prevention rule
In a sandbox without sudo, create Python venvs with `--without-pip` plus get-pip.py and take build tools from release binaries; never assume apt or system pip.
## Detection check
`<venv>/bin/pip --version` and `cmake --version` succeed before any spike runs; `plan/ENVIRONMENT.md` lists the no-sudo commands.
## Evidence
Planning session 2026-10-07, Phase 0.1 diagnostic (`which cmake` empty; venv failure output).
