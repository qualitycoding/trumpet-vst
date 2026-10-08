# Planning CI verification: run 37748754510 (`.github/workflows/plan-verify.yml`, commit 451f7eb)

The stubs were built **with the plugin** (JUCE 9.0.3, Release) on GitHub-hosted runners, 2026-10-08.

| Runner | Configure + build | ctest -LE perf (unit, integration, operational, alloc, plugin) | tpt_plugin_tests | pluginval 1.0.4 strictness 10 (stub VST3) | auval |
|---|---|---|---|---|---|
| ubuntu-24.04 | success | 0 % passed, 5/5 failed (red as expected) | 1 passed (T-026 "parameters" guard), 4 failed | SUCCESS (under xvfb-run) | n/a |
| macos-15 | success | 0 % passed, 5/5 failed | 1 passed, 4 failed | SUCCESS (includes the pluginval auval step) | `auval -v aumu Tpts Qcod`: AU VALIDATION SUCCEEDED |
| windows-2025 (VS 2026) | success | 0 % passed, 5/5 failed | 1 passed, 4 failed | SUCCESS (stdin hashing, no backslash issue) | n/a |

The plugin failures are assertion failures on the stub behaviour: silence, no state, latency 0, default UI state. This
matches D-018.

The run also verified two permissions:
- the token can push `.github/workflows/*`, so it has the workflows permission (Q-E7);
- logs are readable through `gh run view --log`.
