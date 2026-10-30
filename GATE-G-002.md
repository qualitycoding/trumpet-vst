# GATE-G-002: approval to upload release-candidate binaries (S-017)

## Action requested
Run the CI job `rc-artifacts` for the commit **404f981** (the current head of the implementation branch). It builds Release on Linux, macOS and Windows and uploads
one workflow artifact per OS; because the repository `qualitycoding/trumpet-vst` is **public**, the artifacts are downloadable by other GitHub users for 90 days.
Nothing is uploaded until you approve.

## What exactly will happen on `approve`
1. I push the empty commit `git commit --allow-empty -m "[g002-approved rc 404f981]"` to the branch `impl-20261007T145241Z-trumpet-vst-plan` (not to `main`).
2. The job (`.github/workflows/ci.yml`, job `rc-artifacts`, introduced in commit 58eee99) checks out the full history, extracts `404f981` from the marker, checks that it is an ancestor
   of the head and that only record files (`GATE-*.md`, `EXECUTION_LOG.md`, `.checkpoints/*`, `logs/*`) changed since it, builds the head with `-DTPT_GIT_SHA_OVERRIDE=404f981`,
   and uploads three artifacts with `retention-days: 90`: `trumpet-rc-404f981-ubuntu-24.04`, `trumpet-rc-404f981-macos-15`, `trumpet-rc-404f981-windows-2025`.
3. Each artifact contains: the plugin formats (VST3 and Standalone; AU on macOS), `LICENSE`, `NOTICE`, `THIRD_PARTY_NOTICES.md`, `ATTRIBUTION.md`, `licenses/` (JUCE `LICENSE.md`,
   the VST3 SDK licence, nlohmann/json `LICENSE.MIT`, the AGPLv3 text) and `BUILD_INFO.txt` (approved sha, built sha, date, runner image, toolchain, source URL).
4. I record the artifact list and their SHA-256 values in this file and write `GATE-G-006.md` (REAPER test with the Windows artifact).
Nothing else is published: no tag, no GitHub Release, no merge to `main`.

## Evidence
- Job definition: `.github/workflows/ci.yml`, job `rc-artifacts` (commit 58eee99). A dry run (same checks and staging, no upload) was green on all three OSes:
  https://github.com/qualitycoding/trumpet-vst/actions/runs/41057319845 (marker `[rc-dryrun 58eee99]`, jobs `rc-artifacts (ubuntu-24.04 | macos-15 | windows-2025)`).
- CI for the commit to be built: https://github.com/qualitycoding/trumpet-vst/actions/runs/41067613182 (head 404f981); the identical sound code was green on all jobs in
  https://github.com/qualitycoding/trumpet-vst/actions/runs/41057223515 (commit 20d8aed) and https://github.com/qualitycoding/trumpet-vst/actions/runs/41065627439.
- Licence implications (C-098): the binaries include JUCE, so they are AGPLv3 works; the corresponding source is this repository at the approved sha (BUILD_INFO.txt gives the URL). Our own code is Apache-2.0.
  pluginval (GPLv3) and TinySOL are not included. The repository's `NOTICE` states this.

## Question
1. Approve this action exactly as described? (The artifacts will be downloadable by other GitHub users for 90 days and are AGPLv3 works.)

## Allowed responses (verbatim)
`approve`, `deny`, `approve-with-changes: <text>`

## Other open items
- If you answer `deny`, no binary is uploaded and G-006 will give you local build instructions for Windows (Visual Studio 2026) instead.
- G-005 is closed (`proceed`); no accepted failures.
