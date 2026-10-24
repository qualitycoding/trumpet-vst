# SPDX-License-Identifier: Apache-2.0
import os
# Reproducibility (D-004): single-threaded BLAS, set before numpy is imported by the submodules.
os.environ.setdefault("OMP_NUM_THREADS", "1")
os.environ.setdefault("OPENBLAS_NUM_THREADS", "1")
