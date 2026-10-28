# SPDX-License-Identifier: Apache-2.0
import os
# Reproducibility (D-004): single-threaded BLAS, set before numpy is imported by the submodules.
os.environ["OPENBLAS_CORETYPE"] = "Haswell"   # one kernel family on every x86-64 CI runner: bitwise-reproducible fits
os.environ.setdefault("OMP_NUM_THREADS", "1")
os.environ.setdefault("OPENBLAS_NUM_THREADS", "1")
