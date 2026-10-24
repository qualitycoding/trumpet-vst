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
