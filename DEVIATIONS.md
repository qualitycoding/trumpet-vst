# Deviations
- S-002 (DR-SECURITY): `pip-audit` reported 4 vulnerabilities (certifi 2026.7.22; filelock 4.0.12). Both are test-only
  tooling. Upgraded within the same major version: certifi 2026.10.19, filelock 4.2.1; lock and hashes regenerated;
  re-audit clean (`logs/S-002/pip-audit*.txt`).
- S-000: credential source is the gh keyring login (account qualitycoding), not a harness `GH_TOKEN`.
- S-008 (Conventions "Interfaces", additive members): `TrumpetVoice::setHeldValves(Valves)` and `heldValves()` (D-026 R-014),
  `setLegatoPedal(bool)` (D-010, S-010) and `releaseOverblowControl()` (D-014, S-013) were added to `TrumpetVoice.h`
  together with the voice core, because they share state with it. No existing signature changed.
- S-005/S-009 (D-004 "Calibration", DR-T007): the residue ratio c_R,k = R_meas,k / R_TMM-open,k was not applied as a
  complex ratio. The TMM fit's residue phases carry fit noise, and transferring them made the 3-valve states active
  (Re Z < 0 near DC): the reference integrator diverged on state "123" (rms 1e21) and the lowest notes (written 54, 55,
  61) never locked. Instead |R| is scaled by |R_meas|/|R_TMM-open| and the phase is `atan(|Re s'|/Im s')` (the phase of a
  passive second-order mode) plus the measured excess `arg R_meas - atan(|Re s_meas|/Im s_meas)`, which is 0 above the
  measured modes (k > 11). The open state still equals the measured modes exactly (T-030, T-007); only the states with
  valves differ from the literal D-004 wording. Re Z near DC stays slightly negative (|Re Z| < 0.6 Zc) because the
  measured modes themselves have a positive excess for k >= 6.
- S-011 (D-004, DR-REGIME): the measured residue-phase excess is kept for modes 1..9 of the valve states (and for all 11
  measured modes of the open state, as T-030 requires) but not for modes 10 and 11 (and not above), whose excess of 9.7 and
  23.7 degrees made Re Z clearly negative at low frequency and the pedal tones (partial 1) of the 3-valve states aperiodic
  (reference integrator f0 = 0). Keeping the excess of modes 1..9 preserves the oscillation margin the lip trims were tuned for.
- S-011 (D-012, DR-RANGE): `nlp.length_m` is 0.4 m instead of 0.85 m. With 0.85 m (and 0.6, 0.5) the strong nonlinear
  steepening of the loud low notes (written 54-59) made the shipped YIN return 0 on a 1 s window with +-25 cent vibrato
  (T-034 "vibrato 3 Hz / 8 Hz"); 0.4 m passes T-034 with no failure. Brightness 1 still doubles it (0.8 m). The value is a
  table field (validated to [0, 2]) and may be raised again in S-016 if the listening test asks for more brass.
- S-011 (D-004, DR-REGIME, DR-RANGE): the residues of the valve states use the peak heights of the TMM impedance times the
  open horn's measured/TMM peak-height ratio (as research/spikes/make_table.py did) instead of the fit's |R| ratio: the
  fit inflated the weak upper modes (modes 7-11 by 20-60 % in the 3-valve states) and written 61 (1-2-3, partial 3) then
  locked partial 4 or the pedal tone at ff. Frequencies, damping and the open state stay as specified (T-030).
- S-013 (Conventions "Interfaces", additive members): `TrumpetAudioProcessor::unsupportedRate() const noexcept` (D-026 R-022).
  The processor links `nlohmann_json` (heldValves in the state, D-026 R-014).
- S-014 (D-013 "Level meter"): the editor shows a "sounding" lamp, not a level meter. `UiState` carries no level (it is
  compared field by field by the frozen T-026/T-020 tests), so a meter would need a new field; the voice's meter exists
  only for `sounding`, the tracker gate and the idle rule. Logged in `logs/S-014/ui-checklist.md`.
- S-014 (D-004 reproducibility, T-030): the table generator re-executes itself once with the CPU-dependent numerics pinned
  (glibc FMA/AVX2 libm variants disabled through GLIBC_TUNABLES, NumPy AVX-512 loops disabled, OpenBLAS kernel fixed to
  Haswell, one thread), rounds the valve loops and trigger lengths to 0.1 um and runs the modal fits to ftol 1e-15.
  Without this the CI `python` job regenerated the table with differences up to 4e-8 on some runner CPUs (the weak modes
  12-14 and the tiny imaginary parts of the residues), which fails T-030's 1e-9 comparison intermittently.
- S-016 (DR-REAL, D-012): besides the listed constants, the voice has a level-dependent high shelf (`kFfShelfDb` +8 dB at ff,
  `kPpShelfDb` -8 dB at pp, corner `kFfShelfHz` 1800 Hz, Q 0.7, linear in the blowing level, 0 dB at mf) after the radiation
  biquads. A static radiation change fixed ff but broke pp/mf, and neither L_nl (`kNlFfBoost`, a level-dependent factor, kept
  at 0) nor a higher ff blowing factor moved the ff centroid beyond 36 %. K_out is 1/92000 (written C5 at ff peaks at -6.0 dBFS).
- S-016 round 2 (G-005 `iterate`, DR-REAL): the voice has a player-instability model (slow pitch wander with a larger share
  just after a note start, fast jitter), a lip-frequency scoop at tongued note starts, an output-level bloom and a high-shelf bloom after the
  attack. They follow the owner's note that the attack and the start of the clips are too clean; the constants are in `VoiceTuning.h`
  (`kWander*`, `kJitter*`, `kScoop*`, `kCreep*`, `kBloom*`). The wander noise is a seeded xorshift reset with the voice, so renders stay
  deterministic and independent of the block size (T-023).
