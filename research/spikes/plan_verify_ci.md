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

## Run 38582474550 (commit 2892012, 2026-10-14): pre-mortem hardening R-010 and R-011
- All three jobs succeeded with the static MSVC runtime (`CMAKE_MSVC_RUNTIME_LIBRARY`, D-024).
- **Windows `dumpbin /dependents`** on `Trumpet.vst3\Contents\x86_64-win\Trumpet.vst3` lists only system DLLs:
  KERNEL32, USER32, GDI32, SHELL32, ole32, OLEAUT32, COMDLG32, ADVAPI32, WININET, WS2_32, SHLWAPI, WINMM, IMM32,
  COMCTL32, dwmapi, api-ms-win-shcore-scaling, DWrite, d2d1, dxgi, d3d11. There is no `msvcp140`, `vcruntime140`,
  `ucrtbase` or `api-ms-win-crt-*`, so the check passed.
- pluginval (strictness 10) and auval still pass on the stub.
- **Artifacts uploaded (D-025):** `trumpet-stub-windows-2025` (80.8 MB), `trumpet-stub-macos-15` (20.6 MB),
  `trumpet-stub-ubuntu-24.04` (67.5 MB).
