---
id: L-20261008T010500Z-circular-fft-filter-fakes-onsets
title: Circular FFT filtering wraps a note's loud end onto its start and fakes instant onsets
status: active
supersedes: []
recurrence_of: null
occurrences: 1
severity: High
tags: [domain:audio, tech:python, phase:research, kind:verification-gap]
recorded_by: planner
run: gen-20261007T145241Z-trumpet-vst-plan
recorded_at: 2026-10-08T01:05:00Z
---
## Trigger
Applying a frequency response to a finite audio signal with `irfft(rfft(x) * H, len(x))` (no zero padding) and then
measuring anything time-dependent (onset, attack, envelope, transient) near the start or end of the signal.
Signal: attack metrics that do not change when the attack is made slower (here 0-1 ms onsets for 5, 15 and 30 ms
pressure rises).

## What went wrong
`timbre_spike.py` / `onset_spike.py` / `onset_grid.py` filtered the mouthpiece pressure with the bell transfer
function by circular convolution. The filter's impulse response wrapped the loud sustained end of each note onto
t = 0, so the onset metric found a near-full-level frame in the first millisecond. The planner first concluded that the
"tongue release" attack produced a click (claim C-088), which was an artefact.

## Correction
Zero-pad to at least 4x the signal length before the FFT and keep the first N samples (linear convolution); re-ran the
attack grid; C-088 superseded by the corrected grid result.

## Prevention rule
Whenever a spectrum-domain filter is applied to audio whose time structure will be measured, zero-pad the FFT to at
least (signal length + impulse-response length) and truncate, or use a causal time-domain filter.

## Detection check
`grep -n "irfft(np.fft.rfft(" research/spikes/*.py tools/**/*.py` — every hit passes an explicit FFT length larger
than the signal length; a sanity test shows that a slower synthetic attack yields a later measured onset.

## Evidence
research/spikes/onset_grid_result.txt (before: all onsets 0-1 ms; after: see the file), commit on the generation
branch on 2026-10-08.
