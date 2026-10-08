#!/usr/bin/env bash
# FROZEN — DO NOT MODIFY (hash in tests/FROZEN_MANIFEST.sha256)
# SPDX-License-Identifier: Apache-2.0
# T-032: fails if any frozen artifact changed, or if a frozen numeric literal disagrees with its generator
# (lesson L-20261007T150300Z). Run from the repository root.
set -euo pipefail
sha256sum --check --strict --quiet tests/FROZEN_MANIFEST.sha256
python3 research/spikes/expected_values.py --check
echo "freeze OK"
