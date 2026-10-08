# SPDX-License-Identifier: Apache-2.0
"""Generates data/trumpet_resonators.json (format tpt-resonators-1, D-004). Reference: research/spikes/make_table.py
plus the C-099 corrections. STUB (D-018), implemented in S-005.

Usage: python -m tools.resonator.generate_table --fingerings tests/fixtures/trumpet_fingerings.txt --out <json>
         [--keep-calibration <existing json>]
Deterministic: same inputs -> identical bytes (json.dumps(indent=1, sort_keys=True), floats rounded to 10 significant
digits, provenance.git = `git rev-parse HEAD` or "unknown"). --keep-calibration copies "fscale" of matching notes from
an existing table (calibration is done by tpt_calibrate, S-009)."""


def main(argv=None):
    raise NotImplementedError("generate_table.main")


if __name__ == "__main__":
    raise SystemExit(main())
