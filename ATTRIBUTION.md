# Attribution of third-party data

This repository contains material derived from the following works, both licensed under Creative Commons
Attribution 4.0 International (CC BY 4.0, <https://creativecommons.org/licenses/by/4.0/>).

## TinySOL
Carmine Emanuele Cella, Daniele Ghisi, Vincent Lostanlen, Fabien Lévy, Joshua Fineberg, Yan Maresz.
*TinySOL: an audio dataset of isolated musical notes*, version 6.0, Zenodo, 2020. DOI
[10.5281/zenodo.3685367](https://doi.org/10.5281/zenodo.3685367).

- **Used:** the 96 "Trumpet in C", ordinario notes, as a reference for automated realism tests.
- **Changes:** the recordings are downloaded at test time and are never committed or distributed with the plugin.
  The repository contains only metrics derived from them: per-note harmonic levels, spectral centroid and onset
  times, in `research/spikes/ref_spread_result.json` and in CI result files. Gate evidence bundles that contain
  excerpts carry their own copy of this attribution.

## Fréour et al. 2022, Table 1
Vincent Fréour, Louis Guillot, Hideyuki Masuda, Christophe Vergez, Bruno Cochelin. *Parameter identification of a
physical model of brass instruments by constrained continuation*. Acta Acustica 6, 9 (2022). DOI
[10.1051/aacus/2022004](https://doi.org/10.1051/aacus/2022004).

- **Used:** the 11 poles and residues of the measured input impedance of a B♭ trumpet with open valves (Table 1).
- **Where:** `research/spikes/freour2022_open_modes.txt` and `tests/fixtures/freour2022_open_modes.txt`, reproduced
  unchanged.
- **Purpose:** to validate the bore model and to calibrate the resonator table
  (`data/trumpet_resonators.json`, derived).
