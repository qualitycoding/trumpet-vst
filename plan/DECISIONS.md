# Decisions, interfaces and decision rules

Claim IDs `C-###` are in `research/claims.json`. Frozen interfaces are the headers in `core/include/tpt/*.h`, the
plugin headers in `plugin/src/*.h`, and the Python stub signatures in `tools/**`.

Signatures may be **extended** (new members, new functions) but never changed. Every addition is logged in
`DEVIATIONS.md`.

## D-001 Pitch and transposition
- 12-TET frequency is `a4 · 2^((midi − 69)/12)`.
- B♭ transposition: written = concert + 2.
- `noteName` uses sharps, with MIDI 60 = "C4".
- MIDI input is concert pitch (A-003).
- The UI shows both written and concert names.

## D-002 Valves, fingerings, alternates, triggers
- **Valve masks:** bit 0 = valve 1, bit 1 = valve 2, bit 2 = valve 3.
- **Lowering (C-067):** valve 2 = 1 semitone, valve 1 = 2, valve 3 = 3; combinations add.
- **Fingering table:** `tests/fixtures/trumpet_fingerings.txt` is authoritative (C-057–C-065). It has 36 standard rows, written 54–89, and 20 alternate rows.
  - It is compiled into `Fingering.cpp` as a static array in fixture order. The fixture is never read at run time.
  - Alternate G♯5 on 1-2-3 partial 9 was dropped because it fails calibration (C-081).
- **Partial rule (C-056):** `round(48 + 12·log2 n − lowering) == written` holds for every row (checked by T-003).
- **Triggers:**
  - 3rd slide on C♯4 and D4 only (C-064). F♯3 and G3 are already low (C-079).
  - 1st slide on F5, A5 and A♯5 (C-065).
  - Shown in the UI only while Intonation realism < 0.5 (D-013).
- **Above C6:** follows the two-source consensus (C-061). The human checks the whole chart at G-004.

## D-003 Fixed-valves mode and keyswitches
- **Keyswitches** are MIDI 24–31, mapping in order to `0, 2, 1, 12, 23, 13, 123, 3`.
  - They never sound, and they are consumed in both modes. `TrumpetVoice::noteOn` ignores notes 24–31 in every mode; only
    `keyswitch()` acts on them (the processor and `tpt_render` route them there). Held valves survive `prepare()` and
    `reset()` (D-026 R-014); only a keyswitch or `setHeldValves` changes them.
  - In normal mode they do not change anything.
  - In Fixed-valves mode they set the held valves (default open; kept by `prepare()` and `reset()`, D-026 R-014).
- **Resolution** (`resolveNote`, header contract):
  - Choose the partial n in 1..13 whose nominal written pitch `48 + 12·log2 n − lowering` is nearest to the request (ties → lower n).
  - The sounding note is that partial's natural pitch.
- **fscale for fixed-valves notes,** taken from the first rule that applies:
  - (1) the table entry with the same valve combination (no-trigger state) and the same partial, standard before alternate;
  - (2) otherwise the mean `fscale` of all entries (standard and alternate) whose state is that combination's no-trigger state;
  - (3) otherwise the mean of all entries.
  - Intonation realism does not apply in Fixed-valves mode (natural pitch).
- **Pedal tones** (partial 1) started directly in Fixed-valves mode are untested (R-008). They are reachable by Underblow (T-015).

## D-004 Bore model and resonator table (`data/trumpet_resonators.json`, format `tpt-resonators-1`)
- **Bore:** own parametric geometry with 7 parameters, fitted to the Fréour 2022 Table 1 pole frequencies (C-076). Air is at 27 °C: c = 347.288184 m/s (`tmm.C27`), ρ = 1.176018 kg/m³ (`tmm.RHO27`), written with 10 significant digits. Reference code is `research/spikes/bore_fit.py` and `tmm_trumpet.py`.
- **Fit (C-099):** compare like with like. The TMM impedance is fitted with a complex modal fit (`tmm.complex_modal_fit`, 14 modes, f_max 1800 Hz). Its pole frequencies are compared with the measured poles, not with |Z| peaks.
- **Valve loops:**
  - Cylinders of the cylinder radius, inserted at `x_valve` = leadpipe end + 0.35 × the cylinder length.
  - Each is tuned by bisection so that the valve alone lowers partials 3–6 by its nominal interval on average (C-079). Spike values: 0.0896 m (valve 2), 0.1841 m (valve 1), 0.2837 m (valve 3).
  - Combinations simply add the loops.
- **Trigger states:**
  - Extend the corresponding loop, by bisection, so that the target partial's pole has the same deviation from 12-TET as the open horn's same partial.
  - State id `<valves>+t<1|3>@<written>`.
- **Calibration (C-077, C-099),** for mode k:
  - pole ratio c_s,k = s_meas,k / s_TMM-open,k (complex);
  - residue ratio c_R,k = R_meas,k / R_TMM-open,k (complex);
  - k ≤ 11 are measured, and k > 11 reuse k = 11's ratios (R-007).
  - Every state's mode k becomes s·c_s,k and R·c_R,k.
  - So the open state equals the measured modes exactly (T-030), with complex residues kept.
- **Modes per state:** 14. Validation needs ≥ 4; the embedded table needs ≥ 13 (T-007).
- **JSON layout:**
```json
{"format":"tpt-resonators-1","air":{"T_C":27.0,"c":347.2,"rho":1.176},"nlp":{"length_m":0.85,"beta":1.2},
 "radiation":[{"type":"highpass|lowpass|peak|highshelf|lowshelf","f_hz":0,"q":0,"gain_db":0}],
 "states":[{"id":"0","valves":"0","trigger":"-|1|3","trigger_len_m":0.0,"modes":[{"s":[re,im],"R":[re,im]}]}],
 "notes":[{"written":60,"kind":"std|alt","state":"0","partial":2,"fscale":1.0,"natural_dev_cents":0.0}],
 "provenance":{...}}
```
- **Validation limits** (`fromJson`; any violation → ParseError; size checked before parsing):

| Item | Limit |
|---|---|
| Input size | ≤ 1 MiB |
| `format` | must equal `tpt-resonators-1` |
| States | 8–64, unique ids; each of the 8 valve names needs a `-` (no-trigger) state |
| Modes per state | 4–24, sorted by Im(s) |
| Pole s | Re(s) < 0, 0 < Im(s)/2π < 20 kHz |
| Residue R | a pair with Re(R) > 0 (flow solve needs Z0 > 0, R5 D-8) and \|R\| < 1e13 |
| `valves` | one of the 8 canonical names |
| `trigger` | `-`, `1` or `3` |
| Notes | 1–128, unique (written, kind); written in 54..89; partial in 1..min(13, mode count); `kind` std or alt; `state` must exist |
| `fscale` | in [0.8, 1.25] |
| `natural_dev_cents` | in [−100, 100] |
| Radiation | 1–6 sections; f in (10, 20000) Hz; q in (0.1, 10); gain in ±40 dB |
| `nlp` | length in [0, 2] m; beta in (0, 2] |
| `air` | c in [300, 400]; rho in [1.0, 1.4] |
- **`natural_dev_cents`:** `hs_dev(n) + [dev(state, n) − dev(open, n)]`, where:
  - `hs_dev(n) = 1200·log2 n − 100·round(12·log2 n)`;
  - `dev(state, n) = cents(Im(s_n)/2π / ET(written − 2))`;
  - `dev(open, n)` uses the open horn's nominal note `48 + round(12·log2 n) − 2`.
- **Radiation sections:** fitted by `generate_table` (least squares on log-magnitude, 1/3-octave smoothed, 80 Hz–8 kHz) to |j·ω·H/Z_rad| of the open state.
  - Use at most 4 sections, initialised as a high-pass at 1 kHz with Q 0.7, plus a peak.
  - Accept if the RMS error is ≤ 3 dB, otherwise add a section (up to 6).
- **Embedding:** the table is embedded at configure time (`file(READ … HEX)` → a generated `.cpp` with a byte array and a terminating 0), as in saxophone-vst. The JSON is registered with `set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS <json>)` so that
  `cmake --build` re-configures and re-embeds after every change (a plain `set(CMAKE_CONFIGURE_DEPENDS …)` does nothing).
- **`fscale` values:** written by `tpt_calibrate` (D-011). `generate_table --keep-calibration` preserves them.
- **Cents convention:** `generate_table` writes `provenance.cents_reference` = "12-TET, A4 = 440 Hz, concert = written − 2;
  natural_dev_cents = hs_dev + dev(state) − dev(open)" (informative; the parser does not require it, so the frozen T-006
  fixture stays valid).
- **Number formatting:** `generate_table` and `tpt_calibrate` write every float rounded to 10 significant digits
  (Python `float(format(x, '.10g'))`, C++ `snprintf("%.10g")`), including `air.c` and `air.rho` (unrounded otherwise,
  e.g. c = 347.288…). The C++ side reads exactly these numbers. `--keep-calibration` copies `fscale` unchanged.

## D-005 Lip model and per-partial settings
- **Model:** Fréour 2022 eq. 1, the outward-striking one-mass valve (header `LipModel.h`), with the Doc 2023 parameter set: Ql 20, μ 9 kg/m², width 12 mm (C-036).
- **`lipSettingFor(n, f_res)`:**
  - fl = ratio(n)·f_res, with ratio(2..6) = 235/232.70, 340/348.07, 467/462.60, 586/582.14, 703/690.58;
  - ratio(1) = 1.01, and ratio(n ≥ 7) = ratio(6);
  - h0(2..6) = 0.242, 0.218, 0.190, 0.172, 0.168 mm;
  - h0(1) = 0.27 mm, and h0(n ≥ 7) = max(0.12, 0.168 − 0.006·(n − 6)) mm.
- **`thresholdPressurePa(n)`:** 1047, 1788, 2477, 3158, 4109 Pa for n = 2..6 (Doc Table I, C-053); 800 Pa for n = 1; 4109 + 700·(n − 6) for n ≥ 7.
- **What the voice uses:**
  - `lipSettingFor` on the *scaled* resonance (fscale applies to fl too);
  - lip opening h0 × (1.15 − 0.3·lipStiffness), so 1.0 at the default 0.5;
  - an optional per-partial trim table in `core/src/VoiceTuning.h`. These are non-frozen calibration constants, all 1.0 initially.

## D-006 Numerical scheme (real time)
- **Internal rate** f_int = fs × os, with os the smallest value in {1, 2, 4} such that fs × os ≥ 88 200 Hz: 4 below 44.1 kHz
  (22.05–44.09 kHz), 2 for 44.1–88.19 kHz, 1 at ≥ 88.2 kHz. The lip and modal scheme therefore always runs at ≥ 88.2 kHz, the regime of the spikes (96 kHz).
- **Decimation:** cascaded half-band FIR stages, 63 taps each, Kaiser β = 8, linear phase.
  - `latencySamples()` is the sum of the stage delays in output samples, rounded half away from zero (`std::lround`): 16 at os = 2 (31 internal samples = 15.5 output samples), 23 at os = 4 (7.75 + 15.5), 0 at os = 1.
  - It is reported to the host.
- **Modes:** q[k+1] = a·q[k] + g·(u[k+1] + u[k]), with a = (1 + sT/2)/(1 − sT/2) and g = R·T/2/(1 − sT/2) (bilinear). This is the same as `lipsim.cpp`.
- **Implicit flow:** closed form, both signs (`lipsim.cpp`, verified by R5). The flow law uses ρ = 1.2041 kg/m³ (the
  `lipsim` value with which every lip, pressure and regime result was obtained); only D-012 uses the table's
  `airDensity()` and `soundSpeed()`.
- **Lip:** semi-implicit Euler with the previous p.
- **Control rate:** fscale, fl, h0, pm and the parameter smoothing update every 16 internal samples, on a grid counted from `prepare()`/`reset()` and independent of the host block size. Interpolation is linear within each 16-sample segment. This makes the output bitwise independent of block partitioning (T-023).
- **Precision:** double inside the voice; float output.
- **Safety:** if |p| > 1e6 Pa or the state is non-finite, reset the voice state and output 0 for that block (no exception).
- **Idle:** once the output is below −100 dBFS for 50 ms after release, the voice stops processing and outputs exact zeros.

## D-007 Attack ("tongue release", C-089)
- On note-on from silence:
  - lips at y = y0_frac·h0;
  - pressure rises from 0 to pm_on over t_r with a raised cosine;
  - pm_on = k_acc·pm_sus, at least 1.3 × threshold;
  - then it relaxes to pm_sus over 150 ms.
- Settings as a function of blowing level L:
  - L < 0.35: t_r 15 ms, y0_frac 0, k_acc 1.0;
  - L in 0.35–0.75: t_r 15 ms, y0_frac 0, k_acc 1.5;
  - L > 0.75: t_r 15 ms, y0_frac 0.5, k_acc 1.5;
  - parameters are interpolated linearly across ±0.05 around each boundary.
- Grid evidence (C-089): pp 92 %, mf 58 %, ff 50 % of notes inside the trimmed-reference band. The frozen criterion is only the absolute median onset ≤ 150 ms (T-029; spike medians 47–102 ms).
- Legato note-on while sounding: no attack, D-010 instead.
- **Release:** on the last note-off, pm falls to 0 over 40 ms (raised cosine; `kReleaseS` in `VoiceTuning.h`); the lips and
  resonator keep running until the idle rule of D-006 stops the voice.
- All attack constants live in `VoiceTuning.h` and are calibrated in S-016.

## D-008 Dynamics map
- **Blowing level L in [0, 1]:** velocity, or the breath controller once a value ≥ 0 has been received (until `reset()`).
- **Sustain factor k_sus(L):**
  - 0.8 for L ≤ 0.2;
  - log-linear 0.8 → 2.5 for L in 0.2–0.6;
  - log-linear 2.5 → 5.0 for L in 0.6–1.0.
- **Sustain pressure:** pm_sus = k_sus(L)·thresholdPressurePa(target partial) × (1 + overblow brightening, D-009).
- **Evidence:**
  - pp 1.3/0.6–1.0 sustains and gives a pp MAD of 4.3–6.6 dB (C-086, C-094);
  - mf 2.5 and ff 5 (C-091);
  - pressures agree with players within about 10 % (C-095).
- **Test velocities:** pp 24/127 (L = 0.19 → 0.8), mf 76/127 (L = 0.60 → 2.5), ff 124/127 (L = 0.98 → about 4.8).
- **Smoothing:** pm changes are smoothed with a 10 ms one-pole at control rate.

## D-009 Overblow / Underblow (A-007a; C-083, C-084, C-093)
- **Mapping:** a = 2·o, for o in [−1, 1].
- **Register offset k:**
  - Work on the magnitude b = |a| with the register count j = |k| when sign(a) = sign(k) or k = 0; if the sign of a is
    opposite to that of k, first set k = 0.
  - Up: if b ≥ j + 1, set j = floor(b). Down: else if b < j − 0.2, set j = floor(b + 0.2). Otherwise keep j.
    Then k = sign(a)·j. Evaluated at every control-rate update.
  - Examples: o 0 → 0.6 → 1.0 → 0 gives k = 0 → 1 → 2 → 0; o = 0.47 after 0.6 keeps k = 1 (b = 0.94 ≥ 0.8).
- **Target partial:** m = clamp(n + k, 1, max(n, 9)). The cap at 9 is there because jumps into partials 10–14 don't sustain (C-093).
- **Register change:**
  - Ramp f_l, h0 and pm from the current setting to partial m's setting over 20 ms (C-083).
  - The resonator is unchanged.
  - fscale ramps to that of the table entry for (state, m) per the D-003 rule.
- **Within a register** (residual r = a − k in [0, 1)): pm × (1 + 0.9·r) brightens (C-084: up to 1.9× tested). For o < 0, r in (−1, 0]: pm × (1 + 0.15·r).
- **Precedence:** `setOverblowControl` (CC16, (v − 64)/63 clamped) overrides the parameter until the parameter value next changes.
- **UI:** `targetPartial` = m, and `soundingPartial` = the partial measured by the D-013 tracker.

## D-010 Lip slurs, valve changes and re-articulation
- **Legato vs re-articulation (pre-mortem R-029):** a note-on is legato if and only if at least one *other* note is held
  at that moment (the note stack, not the output level, decides), or the legato pedal is on (CC68 ≥ 64, D-014) and the
  note-on arrives within 20 ms after the last note-off while the voice is still sounding. The pedal reaches the voice
  through the additive `void TrumpetVoice::setLegatoPedal(bool on) noexcept` (S-010 adds it, logged in `DEVIATIONS.md`). Otherwise — no note held (even if the previous note is
  still ringing), or the same pitch as the only held note — the note is re-articulated: a **tongue stop** of 25 ms
  (`kTongueStopS` in `VoiceTuning.h`): lips held closed (y = 0, ẏ = 0), p_m = 0, the new note's bore state, fscale and
  lip setting are applied at the start of the stop; then the **re-attack**: p_m rises from 0 to pm_sus over 3 ms
  (raised cosine, `kReattackS`), no accent (k_acc = 1), lips starting closed — exactly the shape of the spike (the D-007
  attack applies only to notes started from silence). Spike
  `research/spikes/tongue_spike.py` (C-101): a 25 ms stop gives a 5 ms-RMS dip of 11.7–20.1 dB on every normal-range
  standard note (median 15.9) and recovery to within 3 dB of the steady level in a median 95 ms (97 % within 250 ms);
  a 100 ms gap gives ≥ 29 dB.
- **Legato,** a note-on while another note is held, never re-attacks.
- **Same valve state (lip slur):** a register change exactly as in D-009, ramping f_l/h0/pm over 20 ms, plus an fscale ramp over 20 ms.
- **Different valve state (valve slur):**
  - Each mode's s and R interpolate linearly from old to new over 15 ms, with coefficients updated at control rate and q states kept.
  - The mode count is the same for every state (14).
  - The lip setting ramps over 20 ms as above.
- **Note stack:** up to 16 held pitches, fixed array, last-note priority (D-014), one entry per pitch with a count: a
  note-on of a held pitch increments its count (and re-articulates if it is the only held pitch); a note-off decrements;
  the pitch leaves the stack at zero, so the first note-off of two overlapping same-pitch notes does not end the second.

## D-011 Tuning, Intonation realism, bend, vibrato
- **Effective scale:** fscale_eff = fscale_note · 2^(r·naturalDevCents/1200) · (A4/440) · 2^(bend/12) · 2^(vib/1200).
  - r is the Intonation realism; Fixed-valves mode uses r = 0 and its own D-003 fscale.
  - vib = 25·depth_eff·sin(2π·rate·t) cents, with depth_eff = max(vibratoDepth, vibratoControl).
- **Where it applies:** to every s and R (a frequency scale, as in `lipsim fscale`) and to f_l.
- **Calibration tool `tpt_calibrate`** (S-009):
  - For each table note, render 1.2 s at mf (L = 0.6) at 96 kHz (os = 1) with r = 0 and default parameters.
  - Secant iteration on fscale until |cents vs ET| < 0.5, at most 8 iterations, on the YIN f0 of the last 0.5 s.
  - Write fscale back.
  - Reject the table if any note fails to sustain (sustain gate as in T-012) → DR-CAL.
- **Bore state:** the voice always plays the note entry's own state, including its trigger state (e.g. written 62 plays
  on `13+t3@62` at every r). The natural deviation of a trigger note is therefore the table value (≈ the harmonic-series
  deviation, because the trigger corrects the combination's sharpness), not the uncorrected sharpness of the plain
  combination.
- **Evidence:** C-081 and C-092. At r = 0 the virtual player is fully corrected. At r = 1 the deviations are the table's `natural_dev_cents` (T-014). Their realism for p2 valved notes is R-005, judged by the human at G-005.

## D-012 Output stage
- **Outgoing wave:** p⁺ = (p + Zc_in·u)/2, with Zc_in = ρ·c / (π·(8.25e-3 m)²) (cup radius; R5 review defect D-7).
- **Nonlinear propagation** (simple-wave time warping, C-045, C-047, C-085):
  - L_nl = nlp.length_m × 2·brightness, so 0.85 m at the default;
  - arrival time t_k = t − β·L_nl·p⁺/(ρc³);
  - monotone arrival times (`max` accumulate) resolve shocks;
  - resampled to the internal grid by linear interpolation, with a look-behind ring buffer sized below (real-time safe).
    The warp uses min(|p⁺|, 30 kPa) (sign kept), so the arrival offset lies in ±A with A = β·L_nl,max·30 kPa/(ρc³)·f_int
    (≈ 119 samples at f_int = 96 kHz, ≈ 238 at 192 kHz, computed at `prepare()`). The stage runs with a fixed base delay
    D = ceil(A) + 2 samples (≤ 1.3 ms at any rate; an unreported output delay, documented in README; `latencySamples()`
    stays the decimator latency, T-025) and a ring of 2·D + 4 samples; offsets are clamped to ±(D − 1) on both sides. a clamp event sets a debug counter, logged once per note.
- **Radiation:** a cascade of the table's RBJ biquads, designed at f_int at `prepare()`.
- **Breath noise:**
  - u·(1 + 0.05·breathNoise·w), where w is xorshift32 white noise mapped to [−1, 1) as w = x/2³¹ − 1 (seed 0x7A11C0DE at
    `reset()`; variance 1/3), through a 3 kHz one-pole (y += a·(w − y), a = 1 − exp(−2π·3000/f_int)). The expected power of
    the filtered noise in a band [f1, f2] is (1/3)·∫|H(f)|² df over the band divided by f_int/2, with H the one-pole
    response (S-008 noise-power test);
  - plus 0.002·breathNoise·pm/pth·w added to the output.
- **Gain:** scale by K_out (fixed so that written C5 at ff peaks near −6 dBFS; in `VoiceTuning.h`) × dB(gain), then the decimator.
- **Safety limiter:** after decimation, on the output sample: y = x for |x| ≤ 0.9, else sign(x)·(0.9 + 0.1·tanh((|x| − 0.9)/0.1)), so that |y| < 1; then the cast to float.

## D-013 UI state and drawing
- **`UiState` fields** come from atomics. `keyswitch()`, `noteOn`, `noteOff`, `allNotesOff`, `reset` and `setParameters`
  publish the fields they change (valves, fixed-valves flag, target, concert/written, extended, trigger) immediately;
  `process()` refreshes `soundingPartial` and `sounding` once per block.
- **Level meter:** one meter for everything: RMS of the final output (after gain, decimator and limiter) over the last
  20 ms, in absolute dBFS. `sounding` = a note is held, or the meter is above −60 dBFS. The partial tracker
  reports 0 when the meter is below −60 dBFS. The idle rule (D-006) uses the same meter (< −100 dBFS for 50 ms after release).
- **`soundingPartial` tracker:**
  - A period estimate from the lip opening h (nearly sinusoidal even when p⁺ has a weak fundamental): upward zero
    crossings of h − mean(h), with a hysteresis of 5 % of the peak-to-peak h, over a window of max(1024 internal samples,
    3 periods of the target partial); with fewer than two crossings in the window, the last valid estimate is kept.
    Any other estimator is acceptable if T-012 and T-020 pass.
  - Updated every 16 control steps.
  - Maps to the nearest partial of the current state × fscale_eff on a log scale.
  - 0 if the level meter is below −60 dBFS.
- **`trigger`** shows the fingering trigger only if r < 0.5.
- **Layout** (`trumpetLayout`), in normalised coordinates, side view with the bell to the right:
  - valve caps at y 0.18–0.26 and x 0.36, 0.44, 0.52 (w 0.05, h 0.08);
  - casings below them, same x and w, y 0.28–0.52 (h 0.24);
  - first trigger left of valve 1, at (0.27, 0.30, 0.05, 0.05);
  - third trigger right of valve 3 below the casings, at (0.58, 0.55, 0.05, 0.05);
  - ladder: 13 rungs at x 0.86, w 0.12, h 0.05; rung of partial k at y = 0.90 − 0.06·(k − 1);
  - caption at (0.02, 0.92, 0.80, 0.07);
  - instrument box (0.02, 0.10, 0.80, 0.70).
  - The values may be adjusted only to satisfy T-033.
- **Drawing:** `juce::Path` only, no images and no generative AI (A-009, A-020).
- **Colours:**
  - pressed valve cap: amber #FFB000; up: grey #9AA0A6;
  - trigger shown: amber, drawn extended by 0.03 in x;
  - target rung: outline amber;
  - sounding rung: filled amber;
  - both, if equal.
- **Editor:** 960×600. The view is the top 400 px; 9 rotary knobs and 2 toggles (D-016) are below. A 60 Hz timer calls `refreshFromProcessor()`.

## D-014 MIDI and expression
- **Mode:** omni, mono, last-note priority, legato (D-010).
- **Note-on:** velocity 0 means note-off. Notes 24–31 go to `keyswitch()` and never sound.
- **Out of range:** a note-on whose `resolveNote` result is nullopt is ignored.
- **Controllers:**
  - CC2 → `setBreath(v/127)` (breath controller; latches until `reset()` or the next transport start, when the processor
    calls `setBreath(-1)`: the host play-head changes from not playing to playing, pre-mortem R-030);
  - CC11 → expression: a smoothed output-gain multiplier v/127 (one-pole, 10 ms time constant, per sample at the host
    rate), not the blowing level; it starts at 1, and CC121 and every transport start restore 1 (pre-mortem R-033). A transport start is detected in
  `processBlock` when the play-head reports playing and the previous block reported not playing; the reset (CC11 gain to 1,
  `setBreath(-1)`) happens before that block's MIDI events are processed;
  - CC1 and channel pressure → `setVibratoControl(v/127)`, the maximum of the two;
  - pitch bend → semitones = 2·(value − 8192)/8192 (14-bit value);
  - CC16 → Overblow (D-009);
  - CC68 (legato pedal, ≥ 64 = on) → while on, a note-on that arrives within 20 ms after the last note-off of a note that
    was still sounding is played legato (D-010) instead of tongued; touching notes in notation exports and REAPER's legato
    command then slur (pre-mortem R-035). Events at the same sample are processed in buffer order (note-off before
    note-on as hosts send them), so without CC68 touching notes are tongued. README documents this;
  - CC120/CC123 → `allNotesOff()`: a panic, unlike the last `noteOff` (D-007 release): the output gain fades to zero over
    4 ms (raised cosine), then lip, mode and decimator states are zeroed and the voice is idle;
  - CC121 → reset the controllers (breath absent, bend 0, vibrato 0, overblow control released). Releasing needs the
    additive member `void TrumpetVoice::releaseOverblowControl() noexcept` (add it in S-013, log it in `DEVIATIONS.md`).
- **Timing:** the processor splits each block at event sample positions.
- **Invalid input:** `setParameters` follows `VoiceParameters::clamped()` (a non-finite field becomes its default); for the
  scalar setters (`setBreath`, `setPitchBend`, `setVibratoControl`, `setOverblowControl`) a non-finite value is ignored
  (previous value kept); note numbers outside
  0..127 are ignored; velocity > 1 is clamped to 1, velocity ≤ 0 is a note-off, non-finite velocity is ignored.
- **CC1 and channel pressure:** the vibrato control is the maximum of the last CC1 value and the last channel-pressure
  value (any channel).

## D-015 State persistence
- `serializeState` / `deserializeState` (header contract), stored by the processor as the UTF-8 JSON of `tpt-state-1`.
- Compact JSON (nlohmann `dump()` without indentation). Keys of `params` are the C++ `VoiceParameters` field names
  (`brightness`, `lipStiffness`, `breathNoise`, `vibratoRateHz`, `vibratoDepth`, `tuningA4Hz`, `intonationRealism`,
  `outputGainDb`, `overblow`, `useAlternates`, `fixedValves`). `params` that is not an object → nullopt.
- APVTS id → field: brightness→brightness, lipStiffness→lipStiffness, breathNoise→breathNoise, vibratoRate→vibratoRateHz,
  vibratoDepth→vibratoDepth, tuning→tuningA4Hz, intonation→intonationRealism, gain→outputGainDb, overblow→overblow,
  alternates→useAlternates, fixedValves→fixedValves.
- `setStateInformation` with invalid data leaves the parameters unchanged.
- APVTS values are set from the deserialised parameters.

## D-016 Parameters
| ID | Range | Default |
|---|---|---|
| brightness | 0–1 | 0.5 |
| lipStiffness | 0–1 | 0.5 |
| breathNoise | 0–1 | 0.1 |
| vibratoRate | 3–8 Hz | 5.5 |
| vibratoDepth | 0–1 | 0 |
| tuning | 415–466 Hz | 440 |
| intonation | 0–1 | 0 |
| gain | −24 to +12 dB | 0 |
| overblow | −1 to 1 | 0 |
| alternates | bool | false |
| fixedValves | bool | false |

`plugin/src/Parameters.h` is authoritative.

## D-017 Realism measurement (T-028, T-029)
- **Metric definitions:** `tools/realism/metrics.py`, the same as `research/spikes/ref_spread.py`.
- **Reference:** TinySOL TpC ordinario, concert 54–82, excluding the 4 resampled files (C-096). 82 notes.
- **Renders:** `tpt_render` (S-015), 2.0 s at 44.1 kHz, velocities pp 24 / mf 76 / ff 124 (out of 127).
- **Harmonics:** levels of harmonics 1..10 relative to the strongest of the ten (`harmonic_levels_db(count=10)`); the MAD
  uses the first 8 (`harm_syn` and `harm_ref` hold 10 values each).
- **`--tinysol`:** the download destination; `compare_tinysol` calls `tinysol.download(dest)` itself.
- **f0 and segments:** f0 = the 12-TET frequency of the concert pitch (A4 = 440) for both sides; harmonics and centroids on
  `steady_segment` of each signal; the synthetic onset on the full render.
- **Frozen thresholds, per dynamic:**

| Metric | Threshold |
|---|---|
| Harmonic MAD (H1..H8) | mean ≤ 6 dB, and ≥ 90 % of notes ≤ 10 dB |
| Centroid ratio | in [0.75, 1.333] for ≥ 85 % of notes |
| Synthetic onset | median ≤ 150 ms |
| ncentroid ordering pp < mf < ff | ≥ 90 % of pitches |

- Feasibility evidence is in the T-029 docstring.

## D-018 Stubs, red run and guards
- **Stubs:** every non-`noexcept` stub throws `tpt::NotImplemented`. Every `noexcept` real-time stub is inert: it outputs zeros, latency −1, default `UiState`. `deserializeState` returns nullopt.
- **Expected red:** all C++ test cases fail (verified: 48/48), and all T-026 plugin cases fail.
- **Guards** (tests that pass against the stubs, by design):
  - T-031 `fetchcontent_pins`, `python_lock` and `workflows_pinned_and_least_privilege` (files pinned during planning; the last is satisfied by `plan-verify.yml` until S-002 replaces it with `ci.yml`);
  - T-026 "parameters" (Parameters.h is final);
  - pluginval and auval on the stub plugin (must pass, `plan-verify` workflow).
- T-029 is skipped without `TPT_RENDER` and `TINYSOL_DIR`.

## D-019 Branches
- Plan: `gen-20261007T145241Z-trumpet-vst-plan`.
- Implementation: `impl-20261007T145241Z-trumpet-vst-plan`, created from it in S-000 (protocol 3.7.0).
- Gate evidence with audio: orphan branches `evidence/G-004` and `evidence/G-005` (lesson L-20261007T151000Z-large-evidence-off-impl-branch). WAVs are never committed on the implementation branch.
- CI results: branch `ci-results` (written by the realism job).
- Merging to `main`, tags and releases: never without G-002.

## D-020 CI (`.github/workflows/ci.yml`, created in S-002; `plan-verify.yml` deleted in the same commit)
- **Triggers and permissions:** `on: [push, pull_request]`, `permissions: contents: read`. No `pull_request_target` or `workflow_run` (C-097). Actions are pinned to the SHAs below.
- **Pins:**
  - `actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1` (v7.0.1);
  - `actions/setup-python@5fda3b95a4ea91299a34e894583c3862153e4b97` (v7.0.0; no `pip-install` input);
  - `actions/cache@55cc8345863c7cc4c66a329aec7e433d2d1c52a9` (v6.1.0);
  - `actions/upload-artifact@cf430e030ddbb5b0abf93d22962f4752f3646cd9` (v7.0.2), for logs and the `tpt_render` binary;
  - `actions/download-artifact@9000827ccba6bdab643e8b6fd33ac0654aef8333` (v8.0.2, C-021), used by `realism` to fetch
    `tpt_render` (run `chmod +x` after download: artifacts drop the executable bit). Keep these pins even if newer
    patches exist.
- **Jobs:**
  - `freeze` (ubuntu): `bash tests/scripts/verify_freeze.sh`.
  - `core` (3 OSes): configure with `-DTPT_BUILD_PLUGIN=OFF`, build Release, `ctest -LE perf`, then `ctest -L perf` as its own
    step with `if: ${{ !cancelled() }}` (so T-024b is evidenced even while accepted failures keep `integration` red).
  - `python` (ubuntu): venv outside the checkout (`$RUNNER_TEMP/venv`, symlinked as `.venv`) from `tools/requirements.lock`, then `pytest tests/python -k "not t029"` and `pip-audit -r tools/requirements.lock`.
  - `plugin` (3 OSes): build with the plugin, run `tpt_plugin_tests` (xvfb-run on Linux), `run_pluginval.sh`, and `auval -v aumu Tpts Qcod` on macOS.
  - `build-linux` (ubuntu): Release build of `tpt_render` only; uploads it as an artifact for `realism` with
    `retention-days: 1` (an internal hand-off of an Apache-2.0 core tool without JUCE, not a product binary, D-025).
  - `realism` (ubuntu, `needs: build-linux`): set up the venv, download TinySOL (cache path `reference-data/tinysol`, the
    extracted subset; key `tinysol-36030a7fe389da86c3419e5ee48e3b7f`), run `python -m tools.realism.compare_tinysol --tinysol reference-data/tinysol --render <tpt_render> --out results.json`,
    then `tools/realism/summarize.py results.json --out SUMMARY.md`, then T-029 (pytest), and push; the summarize, artifact and
    push steps use `if: ${{ !cancelled() && steps.chk.outputs.present == '1' }}` so that results reach `ci-results` even
    when T-029 is red (the job is then red only because of T-029);
    the results to `ci-results` (append-only, D-026 R-018).
  - `gate-evidence` (added in S-014; ubuntu): see below.
- **Write permission:** the `realism` and `gate-evidence` jobs' pushes need `permissions: contents: write` on those jobs
  only, using `GITHUB_TOKEN`.
- **`realism` job conditions:** `needs: build-linux` (a job that only builds `tpt_render` in Release), not `core`, so it
  runs while frozen core tests are still red. Until S-015 `tools/render/main.cpp` does not exist: both `build-linux` and
  `realism` start with a step `id: chk` running `test -f tools/render/main.cpp && echo present=1 >> "$GITHUB_OUTPUT" || true`,
  and every later step has `if: steps.chk.outputs.present == '1'` (`hashFiles()` is not allowed in a job-level `if:`).
  A job whose steps are all skipped this way counts as green.
- **`ci-results`:** an orphan branch, created by the first push (`git checkout --orphan ci-results; git rm -rf .`), holding
  `ATTRIBUTION.txt` and one directory `results/<source sha>/` per run (D-026 R-018); commit message `realism results for <source sha>`.
- **`gate-evidence` job:** ubuntu, `contents: write`, runs on push only when the head commit message contains
  `[gate-evidence G-004]` or `[gate-evidence G-005]` (`if: contains(github.event.head_commit.message, '[gate-evidence')`;
  `workflow_dispatch` is not used because it only works for workflows on the default branch):
  builds the plugin and tools, runs `tpt_ui_snapshots` under `xvfb-run -a` (G-004) or `make_g005_bundle.py` (G-005), and
  force-pushes the output directory with `SHA256SUMS` to the orphan branch `evidence/<gate>` via
  `tools/render/push_evidence.sh`.
- **Windows:** multi-config generator: always `cmake --build build --config Release` and `ctest -C Release`.
- **Wording constraint (T-031 substring checks):** `ci.yml` must not contain the strings `pull_request_target`,
  `workflow_run` or `pip-install:` anywhere, comments included.
- **WAV files:** renders and bundles go under `renders/` (git-ignored); `git worktree`s for evidence branches are created
  outside the repository directory (e.g. `../trumpet-evidence`), so that T-031 finds no WAV in the tree.
- **Failure reporting:** each job writes a one-line summary with `::error::` annotations, so failures are readable through the checks API even when the log host is unreachable.

## D-021 Third-party notices
`THIRD_PARTY_NOTICES.md` (S-002) has one row per component (name, version, licence, URL, use, shipped yes/no). Versions and
URLs come from `plan/ENVIRONMENT.md` and the pins (repository URLs on github.com; TinySOL and Fréour DOIs from `ATTRIBUTION.md`):

| Component | Licence | Use |
|---|---|---|
| JUCE 9.0.3 | AGPLv3/commercial; binaries are AGPLv3 works (C-098) | — |
| VST3 SDK 3.8 | MIT, Steinberg | inside JUCE |
| Catch2 3.16.0 | BSL-1.0 | tests |
| nlohmann/json 3.12.0 | MIT | — |
| pluginval 1.0.4 | GPLv3 | CI only, not shipped |
| TinySOL | CC BY 4.0 | test data, not shipped |
| Fréour et al. 2022 Table 1 | CC BY 4.0 | data |
| ASIO SDK | — | not used: `JUCE_ASIO=0` |

`ATTRIBUTION.md` already exists.

## D-022 Environment constraints (planning and implementation)
- **Planning sandbox:** no sudo, no system pip and no JUCE Linux development packages (C-090).
- **Plugin builds:** verified in GitHub Actions (`plan-verify.yml`).
- **Implementer:** uses `plan/ENVIRONMENT.md`.
  - If sudo is unavailable, build the plugin only in CI, and run core and Python locally.
  - Never commit credentials (L-20261007T150100Z-no-credentials-in-command-lines).
- **Workflow files:** the token used for planning can push them. Verified by the `plan-verify` run 37748754510.

## D-023 Range, extended range and blowing floors
- **Normal range:** written 54–84.
- **Extended range:** written 85–89. These notes are always played, and `extended` is set.
- **Blowing-factor floors** (C-091), applied to k_sus whatever the velocity or breath: written 86, 87 and 89 at least 2.5; written 88 at least 5.0.
- **Range limits:** requests outside written 54–89 are ignored (normal mode).

## D-024 Static MSVC runtime (R-010)
- The root `CMakeLists.txt` sets `CMAKE_MSVC_RUNTIME_LIBRARY` to `MultiThreaded$<$<CONFIG:Debug>:Debug>` (set during
  planning, commit after 1b60fbb), so the Windows VST3 and Standalone do not import `msvcp140`, `vcruntime140`,
  `ucrtbase` or `api-ms-win-crt-*`.
- The CI `plugin` job on Windows checks this with `dumpbin /dependents` on the VST3 binary (the step exists in
  `plan-verify.yml`; copy it into `ci.yml`, S-002).

## D-025 Installable builds and host acceptance (R-011, R-025, R-026)
- **The repository is public** (C-100). Workflow artifacts of a public repository are downloadable by other GitHub users,
  so uploading built binaries is a distribution: it needs **G-002** approval (lesson L-20261015T090000Z).
- **Routine CI** (`plugin` job) builds and validates on every push but uploads **no** binaries (logs only).
- **Release-candidate job** `rc-artifacts` (in `ci.yml`, added in S-017): `if: github.event_name == 'push' &&
  contains(github.event.head_commit.message, '[g002-approved rc ')`, and only after `GATE-G-002.RESPONSE.md` with `approve`
  is committed. `<short sha>` = the first 7 characters of the commit named in `GATE-G-002.md` . The job checks out with `fetch-depth: 0` (the default depth 1 cannot
  answer ancestry, pre-mortem R-034), extracts the sha with the regex `\[g002-approved rc ([0-9a-f]{7})\]`, and fails
  unless (1) `git merge-base --is-ancestor <sha> HEAD` succeeds and (2) every path in
  `git diff --name-only <sha> HEAD` matches `GATE-*.md`, `EXECUTION_LOG.md`, `.checkpoints/*` or `logs/*` (the gate and
  record commits that lie between the approved commit and the marker). It then builds `HEAD`, whose code equals the
  approved commit's. It builds
  Release on the three OSes and uploads one artifact per OS named `trumpet-rc-<short sha>-<os>`, with
  `retention-days: 90` (the maximum). Each artifact contains:
  - the plugin formats (`VST3`, `Standalone`, and `AU` on macOS);
  - `LICENSE`, `NOTICE`, `THIRD_PARTY_NOTICES.md` and `ATTRIBUTION.md`;
  - `BUILD_INFO.txt` (approved sha, `GITHUB_SHA`, UTC date, runner image from `$ImageVersion`, toolchain versions, and the
    source URL `https://github.com/qualitycoding/trumpet-vst/tree/<approved sha>` required by AGPLv3);
  - the full licence texts: JUCE's `LICENSE.md` and the AGPLv3 text it refers to (`agpl-3.0.txt` from the JUCE source
    tree, else https://www.gnu.org/licenses/agpl-3.0.txt), the VST3 SDK `LICENSE.txt`, nlohmann/json `LICENSE.MIT`
    (pre-mortem R-025 residual).
  - The plugin carries its build id: `target_compile_definitions(TrumpetVST PRIVATE TPT_GIT_SHA="<sha>")` from the CMake
    cache variable `TPT_GIT_SHA_OVERRIDE` if set (the `rc-artifacts` job sets it to the approved sha), else
    `git rev-parse --short=7 HEAD` at configure time, else "unknown". The editor draws "Build <sha>" in 11 px grey text
    at the bottom-right corner of the editor, outside the T-033 view rectangles (S-014), matching `BUILD_INFO.txt`.
  Staging: copy `build/plugin/TrumpetVST_artefacts/Release/*`, the four notice files and the four full licence texts
  (`${juce_SOURCE_DIR}/LICENSE.md`; the VST3 SDK `LICENSE.txt` found with `find ${juce_SOURCE_DIR} -path '*VST3_SDK*' -name
  LICENSE.txt`; `${nlohmann_json_SOURCE_DIR}/LICENSE.MIT`; the AGPLv3 text from the JUCE tree if present, else downloaded
  from https://www.gnu.org/licenses/agpl-3.0.txt) into `rc/<os>/`, write
  `BUILD_INFO.txt` there, and upload `rc/<os>`.
- **Dry run:** the same job also runs for the marker `[rc-dryrun <sha7>]`: the sha extraction, ancestry and records-only
  checks and the staging run, but `upload-artifact` is skipped. S-017 needs one green dry run before asking G-002.
- **Approval scope:** a G-002 approval covers exactly one sha. Every new RC upload (for example after G-006 `issues`
  fixes) needs its own G-002 request for the new sha, and G-006 is re-issued after it.
- **Durability:** artifacts expire after 90 days, so `GATE-G-006.md` tells the owner to download and keep the artifact.
  A durable channel (a GitHub Release) is a separate G-002 decision for the owner, outside this plan.
- **Host acceptance:** gate G-006 (after S-017, which first asks G-002 for the RC upload) gives the owner a REAPER
  checklist to run with the RC artifact on Windows 11. S-018 waits for G-006.
- **Owner alternative:** if G-002 is denied, G-006 is offered with local build instructions (`plan/ENVIRONMENT.md`,
  Windows with Visual Studio 2026) instead of an artifact.

## D-026 Smaller hardening decisions (pre-mortem round 1)
- **R-014, held valves:** the processor writes the held valves as an optional top-level key `"heldValves":"<valvesName>"`
  in the same `tpt-state-1` JSON (it reads the current value from `uiState().valves` while Fixed-valves mode is on;
  `deserializeState` ignores unknown keys). On restore the processor parses it with `parseValves` (invalid → ignored) and
  calls the additive member `void TrumpetVoice::setHeldValves(Valves) noexcept` (S-008 adds it, logged in `DEVIATIONS.md`).
  Mechanics: `TrumpetVST` links `nlohmann_json` PRIVATE (S-013); the processor parses the sized string, adds or reads
  `heldValves`, and `dump()`s compactly.
  `prepare()` and `reset()` keep the held valves (an additive refinement of the header's "resets all state", logged);
  only a keyswitch or `setHeldValves` changes them. README documents that hosts should chase keyswitch notes or that the state
  restores the last held valves.
- **R-015, UI atomics:** the UI atomics live in a struct owned by `TrumpetVoice` itself, allocated once in the constructor
  and never reallocated by `prepare()`, so a GUI read during a rate change is safe. The Linux `core` CI job also runs
  `tpt_operational_tests` built with `-fsanitize=thread` in a separate build directory (a step; a TSan report fails the
  job).
- **R-016, state blobs:** `setStateInformation` converts with
  `std::string_view(static_cast<const char*>(data), static_cast<size_t>(std::max(0, sizeInBytes)))`; it never relies on
  NUL termination. The Linux `plugin` job runs `tpt_plugin_tests` once more under ASan
  (`-fsanitize=address,undefined`, separate build directory).
- **R-017, stale G-005:** `GATE-G-005.md` records the source commit of the bundle. If S-017 changes any file under
  `core/src/`, `data/` or `tools/realism/` after that commit, re-run T-029, rebuild the bundle and re-issue G-005 before
  S-018.
- **R-018, traceable results:**
  - The `ci-results` branch is append-only. Each run commits `results/<source sha>/{results.json,results.meta.json,SUMMARY.md}`
    without force (rebase-and-retry 3 times on a push rejection), so the D-020 "force push" is replaced by this.
  - The S-016 round logs and the `REPORT.md` realism tables are produced by `tools/realism/summarize.py` from a committed
    `results.json` and cite its path; no number is typed by hand (Rule 9).
- **R-019, CI upkeep:**
  - Scheduled workflows run only from the default branch (`main`), which this plan never changes (G-002), so no
    schedule is relied on (R-026). `MAINTENANCE.md` gives the owner two ways to get a new binary: (a) as the G-002
    approver, commit `GATE-G-002.RESPONSE.md` with `approve` for a sha and push the `[g002-approved rc <sha>]` marker
    commit (D-025), or (b) build locally on Windows (first ask the owner to paste `cmake --version` and the generator list
    of `cmake --help`, and name the Visual Studio generator that list shows) with Visual Studio 2026 (`plan/ENVIRONMENT.md` "Windows" commands;
    the VST3 is under `build/plugin/TrumpetVST_artefacts/Release/VST3/`). `MAINTENANCE.md` tells the owner to push an empty commit or re-run the last workflow
    once a month, and that a `schedule: - cron: '0 6 1 * *'` trigger becomes active only once `ci.yml` is merged to `main`.
  - `pip-audit` reports without failing on push (`continue-on-error: true`, `::warning::`).
  - Every job logs the toolchain versions (`cmake --version`, the compiler, the runner image version).
  - S-018 writes `MAINTENANCE.md`: what each job does, how to refresh the pins and the lock, what to do when Zenodo or a
    runner image changes, and how to re-run calibration.
- **R-020, supply chain:** `tools/requirements.lock` stays pins-only (`name==version`, the frozen T-031 form). S-002 adds
  `tools/requirements.hashes.txt` with the same pins plus `--hash=sha256:…` lines (from `pip download` + `pip hash`, or
  `pip-compile --generate-hashes`), used only for `pip install --require-hashes -r tools/requirements.hashes.txt`;
  `pip-audit` keeps using the lock. The jobs that push (`realism`, `gate-evidence`) check out with
  `persist-credentials: false`, install the packages and run the tools, and only the final push step sets the remote
  authentication from `GITHUB_TOKEN`. Branch protection on `main` is recommended to the human in `REPORT.md` (a repository setting,
  not changed by the implementer).
- **R-021, floating point:** `tpt_core` compiles with `-ffp-contract=off` (GCC/Clang) and `/fp:precise` (MSVC). If T-012 or
  T-013 fails on one OS only, run `tpt_calibrate` on that OS in CI and compare: a difference > 2 cents for any note →
  `BLOCKED.md` with the comparison (DR-CAL).
- **R-022, host rates:** outside [22.05, 192] kHz the processor does not prepare the voice, outputs silence, and sets a
  processor-side `std::atomic<bool> unsupportedRate` read by the editor, which draws "unsupported sample rate" (not part
  of `UiState`); it logs once. README states the range. This replaces any clamping of the rate.
- **Calibration-tool modes:** `tpt_calibrate <json>` calibrates and rewrites `fscale`. `--long-hold <json>` (R-023) and
  `--brightness-ff <json>` (R-028) are read-only: they never modify the JSON. Both render at 48 kHz with default
  parameters except where stated; the sounding partial of a 1 s window is `uiState().soundingPartial` sampled at the end
  of the window.
- **R-023, long hold:** S-009 runs `tpt_calibrate --long-hold data/trumpet_resonators.json`, which renders every standard
  note at mf for 20 s and writes `logs/S-009-long-hold.md`: one table row per note with the sounding partial and the RMS
  (dBFS) of each 1 s window. A note fails if any window's partial differs from the target, or RMS(window 20) <
  RMS(window 1) − 6 dB → DR-REGIME.
- **R-024, CI concurrency:** `realism` and `gate-evidence` use `concurrency: { group: results-${{ github.ref }}, cancel-in-progress: false }`
  and push only on `push` events (not `pull_request`).

## Decision rules (if → then)
Each rule names the step(s) where it applies.

| Rule | If | Then |
|---|---|---|
| DR-STUB (S-001) | A stub fails to compile on a CI OS | Fix only the stub or its CMake (no test change). Log in `DEVIATIONS.md`. |
| DR-DEP (S-000, S-001) | A FetchContent or apt install fails | Retry 3× with backoff. Then use the pinned commit from a mirror tarball (`codeload.github.com/<repo>/tar.gz/<sha>`) via `FETCHCONTENT_SOURCE_DIR_<NAME>`. Never change a pin. |
| DR-T003 (S-003) | The fixture disagrees with a source the implementer trusts | The fixture wins. Write `TEST_CHALLENGE.md`, and the human decides at G-004. |
| DR-T007 (S-005, S-006) | The open state misses the measured modes, or a valve's lowering is off by > 5 c | Check the like-with-like fit (C-099) and the bisection tolerance. Never widen tolerances. |
| DR-T009 (S-007) | The Fréour reproduction is off | Compare line by line with `research/spikes/lipsim.cpp`. The port must be exact (same ρ = 1.2041, os = 2). |
| DR-REGIME (S-008–S-011) | T-012 fails for some notes, or the S-009 long-hold log shows a failure | (1) Check that sustain gating and the floors are applied. (2) Tune the per-partial trims in `VoiceTuning.h` (fl ratio ±3 %, h0 ±15 %), at most 5 rounds. (2b) For failures only at lip-stiffness extremes: narrow the h0 span of D-005 (1.15 − 0.3·s) symmetrically around 1.0, down to ±0.05 (logged). (3) If still failing, `TEST_CHALLENGE.md` with the evidence; never edit T-012. |
| DR-CAL (S-009) | Calibration does not converge for a note, or fscale leaves [0.8, 1.25] | Apply DR-REGIME to that note first. If still failing, `BLOCKED.md`. |
| DR-OVERBLOW (S-011) | T-015 fails | Check hysteresis and cap. Tune the ramp time (10–40 ms) and the brightening slope (0.5–1.2) in `VoiceTuning.h`, at most 5 rounds. Then `TEST_CHALLENGE.md`. |
| DR-VALVE (S-011) | T-016 valve slurs fail (dip or no settle) | Try, in order: (a) 25 ms interpolation; (b) interpolation in log-frequency; (c) a 15 ms equal-power crossfade of two voices with shared lips (an allowed private implementation change). Log it. |
| DR-ALLOC (S-008) | T-024 counts allocations | Find them with a debugger breakpoint on `operator new`. Preallocate in `prepare()`. |
| DR-PERF (S-017) | RTF > 0.05 on a CI OS | (1) Check Release/LTO. (2) Control-rate decimation to 32 samples. (3) Reduce modes to 12 for partials ≤ 8 only if T-007/T-012 still pass. Never relax T-024. |
| DR-REAL (S-016) | T-029 fails | Up to 5 calibration rounds on non-frozen constants only: D-007 attack, D-008 map, D-012 L_nl / K_out, D-005 trims. Log each round in `logs/S-016-round-<k>.md`. Then go to G-005 with the metrics. Thresholds are never edited. |
| DR-RANGE (S-011) | T-034 fails for a parameter extreme | Brightness or breath noise: reduce the internal mapping (`L_nl` maximum, noise gains in `VoiceTuning.h`) so that the extreme stays on the partial; never change the parameter range (T-026). Tuning extremes: add optional per-note `fscale_415` and `fscale_466` fields to the table (additive, optional in the parser, written by `tpt_calibrate --a4 415,440,466`) and interpolate log-linearly in A4. Vibrato: limit the modulation to the control rate (16 samples) with linear interpolation. At most 5 rounds, then `TEST_CHALLENGE.md`. |
| DR-ARTIC (S-011) | T-035 fails | Tune only `kTongueStopS` within 20–40 ms (the spiked range is 15–40 ms, C-101); `kReattackS` stays 3 ms (the only spiked value). At most 5 rounds; then `TEST_CHALLENGE.md`. |
| DR-INTON (G-005 response) | The human says the natural tendencies (r = 1) are wrong for some notes | Record the human's values as an override table `natural_dev_override` in `generate_table` input (new optional JSON field, additive). Re-run T-014 against the overridden values via a `TEST_CHALLENGE` amendment. |
| DR-PLUGINVAL (S-017) | pluginval fails | Fix the plugin. On Windows, a path or hash issue is fixed in the workflow, never in the frozen script. |
| DR-ZENODO (S-015, S-016) | Zenodo is unreachable locally | Run T-029 in CI only. Continue the other steps. |
| DR-SECURITY (S-002, S-017) | `pip-audit` reports a vulnerability | Upgrade within the same major version if available and regenerate the lock (log it). Otherwise record it in `DEVIATIONS.md`. Halt with `BLOCKED.md` only if it is exploitable in the shipped plugin (Python is test-only). |
| DR-CRED (any step) | A push or API call fails with HTTP 401/403 (for example after the human revoked a pasted token, R-027) | Diagnose first (names only, never values): `env \| grep -E '^(GIT_CONFIG\|GH_TOKEN\|GITHUB_TOKEN\|GIT_ASKPASS\|GIT_)' \| cut -d= -f1`, `gh auth status`, then retry the git command once with the `GIT_CONFIG_*` variables unset in the command scope and `gh auth setup-git` re-run; a per-command `-c http.extraheader` built from the environment (never typed, never written to `.git/config`) is an accepted fallback. If the error text names the proxy, "authorized repository set" or "not enabled for this session", the cause is repository authorisation, not the token: ask the human to attach the repository to the session (or accept a manual upload: a zip mirroring the repository paths plus `UPLOAD.md`), and do not ask for a token. On a reproducible server-side `index-pack`/`unpack` error, try the GitHub Git Data API. Only when all this fails is the token treated as bad. Do not retry with any token from the chat. Finish only the current step locally; start no new step. Write `BLOCKED.md` (asking for a new credential via `GH_TOKEN`) and end the session with a message that lists the unpushed commits (sha and subject) and the request. On restart with a working credential, push first; if the local commits are gone, re-run the steps after the last pushed commit and log it (pre-mortem R-031). |
| DR-PUSH (any step) | `git push` fails with "remote unpack failed", "index-pack failed" or "did not receive expected object" twice, with different content | Stop retrying. Confirm with a tiny test push to a scratch branch; if that also fails, push through the GitHub Git Data API (blobs, tree, commit, ref update via `gh api`), skipping git-ignored files, and log it; if the API also fails, DR-CRED's manual-upload path. |
| DR-DEFAULT (any) | Anything unanticipated | Choose the most reversible option that does not expand scope, log it in `DEVIATIONS.md` with the rationale, and continue. Exception: if it touches frozen tests, security, data integrity, a public interface or research integrity, halt and write `BLOCKED.md`. |
