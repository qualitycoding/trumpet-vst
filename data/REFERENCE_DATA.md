# Reference data

## TinySOL (test data, never committed or shipped)
- Source: Zenodo record 3685367, https://doi.org/10.5281/zenodo.3685367, licence CC BY 4.0 (see `ATTRIBUTION.md`).
- Files and md5 (verified by `tools/realism/tinysol.py`):
  - `TinySOL.tar.gz`: 1 026 917 185 bytes, md5 `36030a7fe389da86c3419e5ee48e3b7f`;
  - `TinySOL_metadata.csv`: 317 576 bytes, md5 `a86c9bb115f69e61f2f25872e397fc4a`.
- Extracted subset: `Brass/Trumpet_C/ordinario/*.wav` (96 files, 51 MB) under `reference-data/tinysol/` (git-ignored). The
  marker `.extracted-36030a7fe389da86c3419e5ee48e3b7f` makes `download()` a no-op; the CI cache restores the directory.
- Notes used by T-029: concert MIDI 54..82 minus the 4 semitone-resampled files (E5 mf, G#3 ff, A#3 ff, B3 ff; C-096) = **82 notes**
  (29 pp, 28 mf, 25 ff).

## Fréour et al. 2022, Table 1
- `tests/fixtures/freour2022_open_modes.txt`, reproduced unchanged (CC BY 4.0, `ATTRIBUTION.md`).
