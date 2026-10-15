# Pre-mortem round 2 (planning protocol Phase 4)

- Reviewer: fresh-context pre-mortem agent (round 2). I wrote neither the plan nor round 1. The review is read-only,
  except for this file.
- Plan reviewed: `gen-20261007T145241Z-trumpet-vst-plan` at `2892012`, then `fa128c2` and the later working-tree changes
  (pass-6 fixes: hashed lock in `tools/requirements.hashes.txt`, `heldValves` as a JSON key, the R-022 processor flag,
  CI additions wired into S-002, artifact naming, the S-009 long hold). The coordinator reports these as commit
  `93a06c0`. I did not raise risks that those changes already fix.
- Inputs: `HANDOFF.md`, `plan/*.md`, `premortem/round-1.md`, `premortem/RISK_REGISTER.md`,
  `research/PRIOR_KNOWLEDGE.md`, `research/claims.json` (99 claims), `research/rounds/*.md`, the frozen tests, the
  CMake files, `.github/workflows/plan-verify.yml`, `research/spikes/*` and `research/coldread/pass-6-dryrun.md`.
- Independent checks, all in the session scratchpad. Nothing was committed.
  - **Repository visibility.** An unauthenticated `GET https://api.github.com/repos/qualitycoding/trumpet-vst`
    (2026-10-14) returned HTTP 200 with `"private": false, "visibility": "public", "default_branch": "main"`. Its branches
    are `main` and `gen-20261007T145241Z-trumpet-vst-plan`.
  - **Tuning-extreme spike (T-034 feasibility).**
    - Setup: `research/spikes/lipsim.cpp` built with `g++ -O2`, using `table_draft.json` and the
      `regime_map_result.json` fscale scaled by A4/440. It covered the 31 standard notes, written 54–84, held 2 s.
    - Results at mf (2.5 × threshold): all 31 notes keep their partial at A4 = 440, 415 and 466 Hz. The largest pitch
      errors are 0.5, 1.5 and 1.1 c against 12-TET at that A4.
    - Results at pp (1.3 × threshold): all 31 keep their partial. The largest errors are 1.7, 1.3 and 2.7 c.
  - **Nonlinear-propagation warp offset (D-012).**
    - Method: Δ = β·L_nl·p⁺/(ρc³), with β = 1.2, ρ = 1.176018, c = 347.288184 and p⁺ ≈ p/2 (as in
      `timbre_spike.py:70`). The pressures are the spike's mouthpiece peaks (`timbre_spike_result.json`, `peak_p_kpa`).
      §3 R-028 gives the numbers.

---

## 1. Incident report (simulation: 6 months after handover)

**Title:** A plugin build nobody approved spreads publicly, and brassy *ff* lead lines crackle in a delivered mix. The
owner can no longer obtain a binary or tell which build is "his".

This failure is different from round 1's. Round 1's incident involved the dynamic CRT crash, unpersisted held valves and
CI turning red. Each of those is mitigated, and the mitigations are assumed to have been carried out perfectly here.

**Timeline (simulated)**
1. **Implementation (month −1 to 0).**
   - S-002 creates `ci.yml` with the D-025 artifact upload, `trumpet-${{ matrix.os }}` with 30-day retention.
   - D-025 (DECISIONS.md:425) states "Workflow artifacts are private to the repository and are **not** a release", and
     GATES.md:33 exempts "this repository's private workflow artifacts" from G-002.
   - `qualitycoding/trumpet-vst` is a **public** repository. Every push therefore published Windows, macOS and Linux
     plugin binaries that any signed-in GitHub user could download. The source run and S-016's calibration rounds
     alone produced about 40 runs. The stub artifacts are already 80.8 MB, 20.6 MB and 67.5 MB
     (`plan_verify_ci.md`, run 38582474550).
   - The artifacts carry no `LICENSE`, `NOTICE` or `THIRD_PARTY_NOTICES.md`, because D-025 uploads only
     `TrumpetVST_artefacts/Release`. They have no version string the user can see.
2. **Handover (month 0).**
   - G-006 passed. The owner ran the REAPER checklist (GATES.md:71) at **default** brightness, mf/ff, at 48 kHz, and
     installed `trumpet-windows-2025` from the S-017 run.
   - `REPORT.md` reported every frozen test green except the accepted G-005 failures.
3. **Month 1.**
   - A forum user found the public repository through the GitHub search "physical model trumpet vst". He downloaded an
     artifact from a mid-S-016 run, round 2 of DR-REAL, which predates the final calibration and the G-005 sign-off. He
     re-posted it as "free physically modelled trumpet VST3".
   - The owner learned of this through an issue on the public repository: "crackles at high notes". He could not tell
     which build the reporter had, because nothing in the binary or the UI names the commit.
4. **Months 1–4.**
   - The owner produced a client big-band arrangement with the lead trumpet at **brightness 1.0** (the "brassy"
     setting), *ff*, up to written C6.
   - D-012 maps brightness 1 to L_nl = 1.7 m, but the shipped warp uses the fixed 64-sample look-behind ring
     (DECISIONS.md:210). At *ff* on written ≥ 78, the warp offset reaches 52–67 internal samples at 88.2/96 kHz: 62–67
     samples for written 81 and more for written 82–84, which the spike does not cover.
   - Following DR-DEFAULT, the implementer had wrapped the read index modulo 64 instead of clamping it. Peaks that
     overflowed therefore read stale ring entries, producing a click once per period, softened by the radiation
     high-pass.
   - No frozen test exercised brightness > 0.5 at *ff*:
     - T-022 uses the default brightness;
     - T-034 runs brightness 1 at mf only and checks only lock, pitch and sustain. The warp is outside the feedback loop,
       so those cannot change.
     - T-023 checks only finiteness and |y| ≤ 1.
   - The owner heard "grit", took it for brassiness, and delivered the mix.
5. **Month 5.**
   - The client's mastering engineer rejected the mix because of crackle in the trumpet stem.
   - The owner tried to make a fixed build:
     - His installed artifact had expired 30 days after it was built.
     - The monthly `schedule:` (DECISIONS.md:455) had never run. `ci.yml` exists only on `impl-*`, and scheduled
       workflows run only from the default branch, `main`. DECISIONS.md:373 already notes this for `workflow_dispatch`.
     - So no canary had run, and no fresh artifact existed.
     - The only path left was pushing a commit to `impl-*`.
6. **Month 5, credential side-effect (security track).**
   - The implementer's `GH_TOKEN` was the PAT the human pasted on 2026-10-14 (A-020), the token he had at hand.
   - At G-004 he revoked "the three tokens" as instructed (GATES.md:76). S-016's `[gate-evidence G-005]` push then
     failed. The implementer halted with `BLOCKED.md` for two weeks until a new token was issued.
   - A later attempt to avoid this left one pasted token un-revoked while `tokens-revoked` was recorded.
7. **Discovery and impact.**
   - The owner re-rendered the client project at brightness 0.6 under deadline pressure.
   - Third-party copies of an uncalibrated pre-G-005 build keep circulating, and they cannot be recalled.

**Impact**
- **Irreversible external publication without G-002:** binaries are public, and their licence notices are missing
  (AGPLv3 JUCE; MIT VST3 SDK notice).
- **Defective audio in a delivered product:** an untested corner of the control space (brightness × dynamics).
- **No durable deliverable:** expired artifacts, a canary that never ran, and no release path.
- **Implementer blocked by the token-revocation mitigation:** the credential's identity was never checked.

**How it was discovered:** by third parties and a client, not by any test, gate or CI job.

**Root causes (§3):**
- R-025: public repository; D-025/G-002 rest on a false premise.
- R-026: no durable binary after handover; the schedule cannot fire on a non-default branch.
- R-027: the implementer's credential may be one of the tokens to be revoked.
- R-028: the 64-sample warp ring is too small for the brightness range and *ff*; overflow behaviour is unspecified.
- R-013 (insufficient mitigation): T-034 tests brightness at mf only, with no timbre or artefact criterion.

---

## 2. Lenses

### 2.1 Technical correctness
- **D-012 warp buffer (R-028, new).**
  - D-012 fixes the real-time warp at a "64-sample look-behind ring buffer" (DECISIONS.md:210). That size is not backed by
    any claim or spike: `timbre_spike.py:23-28` warps offline with `np.interp` over the whole array, and only at
    L_nl ∈ {0, 0.85}.
  - The offset grows with p⁺, with L_nl (up to 1.7 m at brightness 1, DECISIONS.md:207) and with the internal rate. That
    rate is fs·os, which is fs itself at 88.2–192 kHz.
  - Measured *ff* peaks reach 33.7 kPa (written 81) in the spike, so the offset exceeds 64 samples at brightness 1 *ff*.
    At 176.4/192 kHz host rates it exceeds 64 samples even at the default brightness.
  - The plan does not say what happens on overflow (clamp, wrap, enlarge), or whether the fixed warp delay counts towards
    `latencySamples()`. D-006 (DECISIONS.md:119) counts only the decimator.
- **Brightness is outside the feedback loop.**
  - D-012 applies the warp to the outgoing wave after the lip–resonator loop. So T-034's "brightness 0/1" variants
    (`tests/core/test_voice_ranges.cpp:28-29`) check lock, pitch and sustain, which brightness cannot change.
  - T-034 is feasible but cannot detect the R-013 failure mode, which round 1 described as "harsh shock artefacts or
    aliasing at brightness 1 and ff".
- **Tuning extremes are sound.**
  - My spike re-run shows A4 415/466 keeps every normal-range standard note on its partial at mf and pp, within 2.7 c.
  - DR-RANGE's tuning remedy (per-A4 fscale) will probably not be needed.
  - Breath noise 1 and vibrato 3/8 Hz at depth 1 were not re-run. Breath noise acts on the flow inside the loop, so its
    pp behaviour is still unspike-checked (§2.3).
- **Static CRT (D-024): sound.**
  - Run 38582474550 shows only system DLLs. JUCE supports `CMAKE_MSVC_RUNTIME_LIBRARY`, and the VST3 ABI passes no CRT
    objects across the DLL boundary.
  - Each static-CRT DLL uses its own heap and FLS slot. Modern Windows 10/11 limits make FLS exhaustion implausible for
    one plugin. No entry.
- **Checked, no new issue:**
  - `heldValves` as a JSON key, which `deserializeState` ignores (D-026 R-014);
  - the R-022 processor-side flag;
  - the sized `string_view` (R-016);
  - UI atomics allocated in the constructor (R-015);
  - `-ffp-contract=off` / `/fp:precise` (R-021).

### 2.2 Dependency and supply-chain drift
- **Covered by round 1 and D-026:** JUCE, Catch2 and json SHA pins; action SHA pins; image drift (R-019).
  `tools/requirements.hashes.txt` with `--require-hashes` (93a06c0) fixes the T-031 conflict.
- **Residual: wheel hashes per platform.**
  - Hashes produced by `pip download` in the Linux sandbox cover only the cp312 manylinux wheels.
  - Every `--require-hashes` install runs on ubuntu-24.04 (`python`, `realism`, `gate-evidence`), so this holds as long
    as `setup-python` stays at 3.12.
  - A minor-version bump (3.13 wheels) breaks the install with a hash mismatch, not a CVE. `MAINTENANCE.md` should say
    "regenerate both lock files together". Low, no entry.
- **ENVIRONMENT.md:49** still pipes an unpinned `get-pip.py`. This affects local setup only (R-020 residual, unchanged).

### 2.3 Invalid research assumptions
- **Single-source and inferred claims.** These are unchanged since round 1: C-048, C-049, C-051, C-055 and C-063.
  - C-048 (the nonlinear-propagation algorithm, inferred) is the one with new weight. The real-time form in D-012
    (ring size, overflow, delay) has no source, and the spike's offline form does not constrain it (R-028).
- **Short spikes.**
  - **T-034 thresholds were frozen without a feasibility run.** That is lesson #3 (L-20261007T150400Z) again.
    pass-6-dryrun.md:3 asserts "T-034 tolerances are satisfiable by the D-011/D-012 design" by reasoning, not a run
    (lesson #0, L-20261008T151500Z).
  - My run confirms the tuning variants. The brightness variants are trivially satisfiable (§2.1). Breath noise 1 and
    vibrato depth 1 remain unrun. Their risk is Low: T-034 has DR-RANGE, and breath noise is ±5 % flow modulation at mf.
- **Repository visibility was assumed, not checked.**
  - D-025 and GATES.md:33 assume private artifacts. The repository is public (verified; R-025).
  - No claim records visibility, and S-000 does not check it.

### 2.4 Implementer misinterpretation
- **GATES.md:33, read literally on a public repository:** it says "upload binaries anywhere outside this repository's
  *private* workflow artifacts". Since none of the artifacts are private, every S-002 artifact upload is a G-002 trigger.
  The implementer must either halt at S-002 or ignore G-002. Either way it guesses (R-025, High).
- **Warp overflow behaviour and latency accounting** are unspecified (R-028).
- **`GH_TOKEN` identity.**
  - Round 1's mitigation for R-012 included "S-000: the implementer confirms its own credential is distinct from both".
    The register still lists it (RISK_REGISTER.md:19).
  - PLAN.md S-000 (lines 32–46) has no such action, so the implementer cannot know whether its token is one of the three
    the human will revoke (R-027).
- **`tokens-revoked`** is now unambiguous (GATES.md:60). No item.

### 2.5 Integrity (Rule 9)
- **Planning records overstate the mitigations.**
  - **R-012** (RISK_REGISTER.md:19): "S-000 confirms a distinct credential" is absent from PLAN.md.
  - **R-013** (RISK_REGISTER.md:20): the Mitigation column says "S-011 range sweep + DR-RANGE; G-005 demo 09; T-012b
    amendment".
    - No `range_check`, demo 09 or T-012b exists in `plan/` (`grep` finds none, and GATES.md:61 lists demos 01–08 only).
    - The Status column says "mitigated: new frozen test T-034 (red-verified)". Red-verified shows only that the test
      fails against stubs, not that it is feasible or that it covers the failure mode.
  - This is not fabricated data, but a later reader would believe controls exist that do not. Corrections are proposed
    under the verdicts (§4).
- **D-025's "private to the repository"** is a factual statement in a decision, and it is false. D-025 is the basis for
  carving out G-002 (R-025).
- **No fabricated numbers found.** The `plan_verify_ci.md` run 38582474550 figures match the coordinator's account.
  `research/` and the git history contain no secrets (pattern scan for `github_pat_`/`ghp_`; clean).

### 2.6 Past lessons (each row of `research/PRIOR_KNOWLEDGE.md`)
| # | Lesson | Applied by | Verdict (round 2) |
|---|---|---|---|
| 0 | L-20261008T151500Z reasoned expected value | `t028_values.py`, `expected_values.py`, S-004 | Applied for T-028. **Partly regressed for T-034:** its feasibility was reasoned (pass-6), not run. My run covers the tuning variants only. |
| 1 | L-20261008T010500Z circular FFT | C-089, T-028, S-004/S-005/S-015 | Applied. |
| 2 | L-20261007T150300Z hand-typed constant | `expected_values.py --check`, T-032 | Applied. T-034's tolerances (10/15 c) are thresholds, not algorithm outputs. |
| 3 | L-20261007T150400Z threshold vs spread | `ref_spread.py`, T-029 docstring | Applied for T-029. Not applied for T-034 before freezing (see #0). |
| 4 | L-20261007T150500Z linear-bore ceiling | `timbre_spike.py`, D-012 | Applied at L_nl 0.85. The real-time form (ring size) was not spiked: R-028. |
| 5 | L-20261007T150600Z control mappings | T-034, DR-RANGE | **Still partly unapplied.** Brightness is checked only where it cannot matter (mf lock/pitch). The *ff* × brightness range was never run with the real-time warp (R-013 insufficient, R-028). |
| 6 | L-20261007T150700Z regime per note | `regime_map.py`, T-012, S-009 long hold | Applied. |
| 7 | L-20261007T150000Z no pip/sudo | ENVIRONMENT no-sudo, S-000 | Applied. Reconfirmed: my scratchpad venv needed `--without-pip` + get-pip. |
| 8 | L-20261007T150100Z credentials | HANDOFF, S-000, S-KNOW scan | Applied to commands and files. A third token was pasted in chat on 2026-10-14 (A-020). The rule does not cover credentials supplied through chat, and no S-000 check separates the implementer's token from the ones to be revoked (R-027). |
| 9 | L-20261007T150800Z host accessors | D-013, `setHeldValves`, processor `unsupportedRate` | Applied (additive, logged). |
| 10 | L-20261007T150200Z restart | HANDOFF | Applied. |
| 11 | L-20261007T150900Z sha256sum backslash | `run_pluginval.sh` | Applied. |
| 12 | L-20261007T151000Z evidence off branch | D-019 orphan branches | Applied. On a public repository the evidence branches are public too (TinySOL CC BY 4.0 with `ATTRIBUTION.txt`, so permitted). |

### 2.7 Scale and performance
- **Real-time factor:** unchanged from round 1 (about 2× margin).
- **Ring size:** enlarging the warp ring to cover brightness 1 at 192 kHz costs memory only (a few hundred doubles),
  not CPU.
- **CI cost on a public repository:** standard-runner minutes and artifact storage are free, so the 169 MB of artifacts
  per run is not a quota risk.
- **Sample-rate coverage:** at 176.4/192 kHz the lips run at fs (os = 1). Calibration was done at 96 kHz. T-025 checks
  only concert 67 within 15 c there (`tests/core/test_voice_robustness.cpp:37-49`). This is rare for the owner; Low, no
  entry.

### 2.8 Security
- **R-012 tokens.**
  - Three PATs were pasted, and one (2026-10-14) was used for CI run 38582474550. The repository is public, so a live
    Contents and Workflows write token is a direct supply-chain path.
  - With such a token, anyone can push a modified `ci.yml` or source to `impl-*`. The owner installs exactly those
    artifacts at G-006 and afterwards.
  - The `tokens-revoked` gate cannot verify revocation (the implementer has no token values, correctly). It also does
    not make sure the implementer's own token is not on the list (R-027).
- **Public artifacts are unsigned.**
  - Windows SmartScreen or Smart App Control on a clean Windows 11 install may block an unsigned DLL or EXE. Whether
    Smart App Control blocks DLLs loaded by REAPER was not verified here.
  - G-006 item 1 would show it on the owner's machine. No entry; noted for `MAINTENANCE.md`.
- **Fork PRs:** on a public repository, first-time contributors need approval to run workflows, and their tokens are
  read-only. The D-026 R-024 push-only-on-`push` rule holds. No new risk.
- **Personal data:** none in the tree or the history (author is the `users.noreply` address only).

### 2.9 Operational realities
- **The scheduled canary can never run (R-026).**
  - D-026 R-019 adds `schedule:` to `ci.yml` (DECISIONS.md:455), but `ci.yml` lives only on `impl-*`.
  - GitHub runs scheduled workflows from the default branch (`main`), and merging to `main` needs G-002, which no step
    requests. DECISIONS.md:373 already records the same restriction for `workflow_dispatch`.
  - Even after a merge, a public repository disables scheduled workflows after 60 days without activity. Round 1 noted
    this in §2.9, and no mitigation was adopted.
- **The binary does not outlive 30 days (R-026).**
  - D-025 sets 30-day retention, and no step produces a durable binary. A release needs G-002, which no step proposes.
  - Six months on, the owner has only what he copied to disk, with no commit stamp in the binary or UI to tell builds
    apart.
- **G-006 feasibility:** the checklist is feasible for the owner. Item 7 ("no clicks") is run at default brightness, so
  it cannot catch R-028.

## 3. Root cause analysis

Severity follows the rubric in the request. Likelihood is L, M or H.

### R-025: The repository is public, so D-025's "private artifacts" premise and the G-002 carve-out are false
- **Lens:** implementer misinterpretation / invalid research assumption / integrity. **Severity: High.**
  - Read literally, GATES.md:33 makes every artifact upload in S-002 a G-002 trigger. The implementer must either halt
    or ignore G-002.
  - Either way, the plan carries out the external action that G-002 exists to gate: public distribution of binaries.
    That action cannot be undone, and the binaries carry no licence notices.
  - It is not Critical, because the source is public and nothing secret is exposed.
- **Likelihood: H.** The visibility is verified: the API returns `"visibility": "public"` (2026-10-14).
- **Root cause:**
  - D-025 (DECISIONS.md:425) and GATES.md:33 state, without checking, that the artifacts are private.
  - No claim records the repository's visibility, and S-000 (PLAN.md:39-46) does not check it.
  - The artifact path `TrumpetVST_artefacts/Release` contains no `LICENSE`, `NOTICE` or `THIRD_PARTY_NOTICES.md`.
  - The binary and UI show no commit id.
- **Traces to:** D-025, G-002, G-006, A-016, D-021, C-098, R-011 mitigation.
- **Proposed mitigation:**
  1. Record a new claim: "the repository is public (API, 2026-10-14)". Rewrite D-025 to say: "Workflow artifacts of this
     public repository can be downloaded by any signed-in GitHub user".
  2. Make the choice explicit at G-002, and pick one of two options. The planner must not decide this silently.
     - **(a) Recommended:** only the `plugin` job on `impl-*` push events uploads artifacts. `retention-days` is 14.
       The artifact also contains `LICENSE`, `NOTICE` and `THIRD_PARTY_NOTICES.md`, plus a `BUILD_INFO.txt` with the
       commit sha and "development build, not a release". The human's one-time G-002 approval "publish development
       artifacts" is requested in S-002, before the first upload.
     - **(b) Alternative:** ask the human to make the repository private before S-002.
  3. The editor caption, or the `About` text, shows `TPT_GIT_SHA` (a compile definition from CMake). Any build in
     circulation can then be identified.
  4. S-000 records `gh api repos/qualitycoding/trumpet-vst --jq .visibility`. If the result differs from the plan's
     assumption, write `BLOCKED.md`.

### R-026: No durable binary after handover; the monthly canary cannot run
- **Lens:** operational / dependency drift. **Severity: Medium** (the workaround is to push an empty commit to `impl-*`
  and download within 30 days; it is undocumented and depends on CI still being green). **Likelihood: H.**
- **Root cause:**
  - `schedule:` (DECISIONS.md:455) is in `ci.yml` on `impl-*`. GitHub only runs scheduled workflows from the default
    branch, `main`, which DECISIONS.md:373 itself acknowledges for `workflow_dispatch`.
  - Merging to `main` is G-002, and no step proposes it.
  - On a public repository, scheduled workflows are disabled after 60 days of inactivity anyway.
  - D-025 sets 30-day retention, and no step produces a durable binary.
- **Traces to:** D-020, D-025, D-026 R-019, G-002, S-018, A-017.
- **Proposed mitigation:**
  1. S-018 issues a **G-002 request** with three options. The human chooses; none is the default.
     - `approve`: merge `impl-*` to `main` and create a `v0.1.0` GitHub Release with the Windows, macOS and Linux zips
       plus licence notices. The schedule then becomes live.
     - `approve-with-changes: workflow-only`: copy only `ci.yml` to `main`.
     - `deny`: in that case `REPORT.md` states that no canary exists.
  2. `MAINTENANCE.md` documents:
     - "to get a fresh build: `git commit --allow-empty -m rebuild && git push`; artifacts expire after N days";
     - "after 60 days without activity, re-enable the schedule in the Actions tab".
  3. The owner is told at G-006 to archive the accepted artifact zip, with its run URL, outside GitHub.
- **Relation to round 1:** this is where the R-019 mitigation (monthly schedule) and the R-011 mitigation (installable
  artifact) are insufficient. See §4.

### R-027: The implementer's `GH_TOKEN` may be one of the tokens the human is told to revoke
- **Lens:** security / operational. **Severity: High.**
  - If it is, revoking at G-004 kills the implementer's pushes from S-016 onwards (`[gate-evidence G-005]`, CI,
    S-017). The implementer is blocked mid-run.
  - The opposite resolution, keeping the token, leaves a pasted write token live while `tokens-revoked` is recorded.
    That resolves R-012 in name only.
- **Likelihood: M.**
  - The human supplied a third token by chat on 2026-10-14 for planning CI (A-020, ASSUMPTIONS.md:62).
  - The implementer's harness token comes from the same human, and nothing distinguishes the two.
- **Root cause:**
  - Round 1's mitigation step "S-000 confirms a distinct credential" (round-1.md:378-379; RISK_REGISTER.md:19) was never
    written into PLAN.md S-000 (lines 32–46).
  - The revocation instruction (GATES.md:76) covers "the three tokens" without telling the human to issue a fresh,
    never-pasted token for the implementer first.
- **Traces to:** A-020, R-012, S-000, G-004, G-005, HANDOFF "Open human actions", L-20261007T150100Z.
- **Proposed mitigation:**
  1. HANDOFF "Open human actions", step 1 (before S-000): the human creates a **new fine-grained token** that was never
     pasted in chat. It has the ENVIRONMENT.md permissions, is named `trumpet-vst-impl-<date>` and expires in 90 days or
     less. The human provides it only through the harness `GH_TOKEN`.
  2. S-000 records the token's name and expiry without printing the token. It uses `gh api /user` for the login and the
     response header `github-authentication-token-expiration`.
  3. S-000 writes into `GATE-G-004.md`: "the implementer's token expires <date>; revoke the three pasted tokens, not
     this one".
  4. Add one sentence to the lesson L-20261007T150100Z prevention rule (as an S-RETRO lesson): "credentials are
     supplied only through the harness environment, never pasted in chat".

### R-028: The real-time warp ring (64 samples) is too small for the brightness range and *ff*; overflow behaviour is unspecified
- **Lens:** technical / invalid research assumption / misinterpretation. **Severity: Medium** (audible artefacts or a
  brightness control that does nothing in its upper half at *ff*; the workaround is lower brightness). **Likelihood: M.**
- **Evidence:**
  - Offset Δ = β·L_nl·p⁺/(ρc³), with p⁺ ≈ p/2 and the spike's *ff* mouthpiece peaks (`timbre_spike_result.json`).

    | Written, dynamic | p peak | L_nl 0.85 (default) at 88.2 / 96 kHz | L_nl 1.7 (brightness 1) at 88.2 / 96 kHz | Brightness 1 at 192 kHz |
    |---|---|---|---|---|
    | 69 ff | 24.7 kPa | 23 / 25 samples | 45 / 49 | 98 |
    | 78 ff | 28.6 kPa | 26 / 28 | 52 / 57 | 114 |
    | 81 ff | 33.7 kPa | 31 / 34 | 62 / 67 | 134 |
    | 81 mf | 16.0 kPa | 15 / 16 | 29 / 32 | 63 |

  - Written 82–84 at *ff* (T-029 range, concert ≤ 82) are not in the spike and will be higher. The look-behind must
    span both the positive and the negative offsets.
  - At 192 kHz, the default brightness at *ff* on written 81 (67 samples) also exceeds 64.
- **Root cause:**
  - D-012 (DECISIONS.md:210) fixes the ring at 64 samples without a source or spike. The spike warps offline
    (`timbre_spike.py:23-28`).
  - The plan does not say whether the offset is clamped, the ring sized from L_nl,max and fs, or the warp delay added to
    `latencySamples()` (DECISIONS.md:119).
  - No frozen test exercises brightness > 0.5 with a timbre or artefact criterion:
    - T-022 uses the default brightness;
    - T-034 checks lock and pitch at mf, which is independent of the warp;
    - T-023 checks only finiteness and bounds.
- **Traces to:** D-012, D-006, C-045, C-047, C-048, C-085, T-022, T-023, T-034, R-013, lesson #4/#5.
- **Proposed mitigation:**
  1. D-012: bound the warp input at |p⁺| ≤ 25 kPa (in the warp formula only). Size the ring at `prepare()` (allocated
     there, so this is real-time safe) as `ceil(2 · 1.25 · β · L_nl,max · 25e3 / (ρc³) · f_int) + 4` samples, with
     L_nl,max = 2·nlp.length_m. That is about 2.6 ms of samples (232 at 88.2 kHz, 501 at 192 kHz). State "clamp the
     offset to the ring capacity; never wrap".
  2. Apply a fixed warp delay of D = half the ring, about 1.3 ms. **Do not** add it to `latencySamples()`.
     - At os = 2 it would be about 57 output samples plus the decimator's 16, which breaks the frozen T-025 bound of
       ≤ 64 (`tests/core/test_voice_robustness.cpp:42`).
     - Instead, document it in D-006 and the README as an unreported delay of ≤ 1.3 ms, which is musically negligible.
  3. Add a non-frozen S-011 check, `tools/render/range_check.cpp` → `logs/S-011-range-check.md`:
     - written 69, 78, 81 and 84 at *ff*, brightness {0.5, 1}, at 48 and 192 kHz;
     - criteria: no ring clamp events (a debug counter), and energy above 16 kHz below −40 dB relative to the total.
  4. Add G-005 demo 09, "brightness 0 → 1 sweep, concert 79 *ff*".

  This is the round-1 R-013 proposal that was not adopted.

### Round-1 mitigations: new risks they introduced (summary)
- **D-024, static CRT:** none found. It is verified by run 38582474550.
- **D-025 / G-006:**
  - R-025: the "private" premise is false;
  - R-026: the artifacts are not durable;
  - G-006 itself is feasible.
- **T-034:**
  - The tuning variants are feasible (my spike run). The brightness variants are feasible, but they test nothing about
    brightness.
  - Breath noise and vibrato are unrun but low risk.
  - No frozen test is proven invalid.
- **`tokens-revoked` gating:** R-027, a possible conflict with the implementer's own token. The G-005 wording is now
  unambiguous (GATES.md:60).
- **D-026 (round 1, after the 93a06c0 fixes):** no new risk.
  - The hashed lock is a separate file, so T-031 holds.
  - `heldValves` is a JSON key.
  - R-022 no longer clamps.
  - R-018 append-only `ci-results` works because pushes happen only on `push` events with a concurrency group.

---

## 4. Tables

### Proposed register entries
| ID | Description | Lens | Severity | Likelihood | Root cause | Traces to | Mitigation | Status |
|---|---|---|---|---|---|---|---|---|
| R-025 | The repository is public: D-025's "private artifacts" premise is false; every artifact upload is a literal G-002 trigger and publishes binaries without licence notices or a build id | misinterpretation / research assumption / integrity | High | H | Visibility assumed, never checked (DECISIONS.md:425, GATES.md:33, S-000) | D-025, G-002, G-006, A-016, D-021, C-098 | New claim (public); rewrite D-025; G-002 decision in S-002 (dev artifacts with LICENSE, NOTICE, THIRD_PARTY_NOTICES, BUILD_INFO, 14 days) or human makes the repository private; `TPT_GIT_SHA` in the UI; S-000 visibility check | open |
| R-026 | No durable binary after handover; the monthly `schedule:` never fires because `ci.yml` is not on the default branch (and 60-day public-repository disable) | operational / drift | Medium | H | Scheduled workflows run from `main` only; merge needs G-002; 30-day retention; no release step | D-020, D-025, D-026 R-019, G-002, S-018 | S-018 G-002 request (release, workflow-only, or deny, documented); `MAINTENANCE.md` rebuild and re-enable instructions; owner archives the G-006 artifact | open |
| R-027 | The implementer's `GH_TOKEN` may be one of the three pasted tokens: revoking at G-004 blocks S-016+ pushes, or keeping it leaves a pasted token live | security / operational | High | M | Round-1 "S-000 distinct credential" step not in PLAN S-000; revocation instruction does not exempt or replace the implementer token | A-020, R-012, S-000, G-004, G-005 | Fresh never-pasted fine-grained token before S-000; S-000 records its name and expiry; GATE-G-004 names which to revoke; lesson amendment | open |
| R-028 | The real-time D-012 warp ring (64 samples) is too small at brightness 1 × *ff* (62–67 samples at 88.2/96 kHz for written 81; 134 at 192 kHz), and overflow behaviour and latency are unspecified → clicks or a dead brightness range | technical / research assumption | Medium | M | Ring size never spiked (offline `np.interp` spike, L_nl ≤ 0.85); no test of brightness > 0.5 with a timbre criterion | D-012, D-006, C-048, T-022, T-034, R-013 | Size the ring at `prepare()` from L_nl,max, a 25 kPa bound and f_int; clamp, never wrap; unreported delay ≤ 1.3 ms documented (T-025 ≤ 64); S-011 range check (ff × brightness × 48/192 kHz, HF energy); G-005 demo 09 | open |

Corrections to existing entries (planning-record accuracy, §2.5):
- **R-012 Mitigation:** "S-000 confirms a distinct credential" is not implemented. Replace it with the R-027
  mitigation.
- **R-013 Mitigation and Status:** "S-011 range sweep … G-005 demo 09; T-012b" is not in the plan. Either implement it
  through R-028 items 3–4, or change the text to "T-034 (mf lock and pitch only) + DR-RANGE".
- **R-019 Status:** the monthly schedule is ineffective (R-026).

### Round-1 mitigation verdicts
| Risk | Verdict | Why |
|---|---|---|
| R-010 dynamic MSVC runtime | **sufficient** | D-024 is in root `CMakeLists.txt:10`, before any `FetchContent_MakeAvailable`. In run 38582474550, `dumpbin` lists only system DLLs (no `msvcp140`, `vcruntime140`, `ucrtbase` or `api-ms-win-crt-*`). The check (`plan-verify.yml`, pwsh step) fails on any of them, and S-002 copies it into `ci.yml`. No new CRT risk was found. |
| R-011 no host acceptance / no installable binary | **partly sufficient** | G-006 with the REAPER checklist removes the "never ran in a host" failure, and the artifact exists. But the G-002 carve-out rests on a false premise (R-025). The binary is not durable after handover (R-026). Item 7 is run at default brightness only (R-028). |
| R-012 pasted tokens | **insufficient** | The `tokens-revoked` gate is the strongest control available to the plan, since the implementer cannot verify revocation. But the round-1 "S-000 distinct credential" step was never added to PLAN.md, while the register claims it was. A third token was pasted on 2026-10-14 and may be the implementer's own (R-027). Exposure on a public repository is a direct supply-chain path to the binaries the owner installs. |
| R-013 control ranges | **insufficient** | T-034 covers lock, pitch and sustain at **mf** only. Brightness acts outside the loop, so its variants cannot fail. There are no pp/ff extremes and no aliasing or artefact criterion. The adopted mitigation omits round 1's S-011 range sweep, demo 09 and T-012b, although the register lists them. My spike run shows the tuning variants are feasible (≤ 2.7 c). The real failure at brightness 1 × *ff* is untested (R-028). |

COUNTS: Critical 0, High 2, Medium 2, Low 0
