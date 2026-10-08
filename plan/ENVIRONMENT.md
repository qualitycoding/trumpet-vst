# Environment (pinned; verified 2026-10-08)

## Toolchain
| Component | Version / pin | Verified |
|---|---|---|
| OS (planning sandbox) | Ubuntu 24.04 x86-64, 4 vCPU, no sudo | yes |
| CI runners | `ubuntu-24.04`, `macos-15` (arm64), `windows-2025` (VS 2026) (A-022, C-023–C-025) | `plan-verify` run 37748754510 |
| C++ compiler | GCC 13.3.0 (sandbox); AppleClang (macos-15 default); MSVC 19.5x (VS 2026, windows-2025) | GCC yes; others in CI |
| CMake | ≥ 3.22. Sandbox: 4.4.4 release binary; runners: preinstalled | yes |
| Ninja | 1.13.2 release binary (sandbox); `ninja-build` apt package (CI Linux) | yes |
| JUCE | 9.0.3 @ `be29c81492b6151c8ea8d14c840e1311963b3a83` (C-001, C-002), via FetchContent | CI |
| Catch2 | v3.16.0 @ `317ac1ed4c0bb6e6b91eafc817e05c488feffcb3` (C-007) | yes |
| nlohmann/json | v3.12.0 @ `55f93686c01528224f448c19128836e7df245f72` (C-008) | yes |
| pluginval | v1.0.4; zip SHA-256 values in `tests/scripts/run_pluginval.sh` (C-010–C-012) | CI |
| Python | 3.12.3 | yes |
| Python packages | `tools/requirements.txt` (numpy 2.5.3, scipy 1.18.1, soundfile 0.14.0, pytest 9.1.1, pip-audit 2.10.1); lock `tools/requirements.lock` | yes |

## Credentials (implementer)
- **GitHub token:**
  - Contents read/write and Workflows read/write on `qualitycoding/trumpet-vst`: push to `impl-*`, `evidence/*` and `ci-results`, and create `.github/workflows/ci.yml`.
  - Actions read: read CI results.
  - Contents read/write on `qualitycoding/agent-knowledge` for S-KNOW, plus Administration write if the repository has to be created.
  - Provide it via `gh auth login --with-token`; never write it to a file (L-20261007T150100Z).
- **CI:** no secrets are needed. The realism job pushes with `GITHUB_TOKEN` (job-level `contents: write`).

## Setup commands

### Linux with sudo (CI and developer machines)
```bash
sudo apt-get update
sudo apt-get install -y g++ libasound2-dev libjack-jackd2-dev ladspa-sdk libfreetype-dev libfontconfig1-dev \
  libx11-dev libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev libxrandr-dev libxrender-dev libxi-dev \
  libcurl4-openssl-dev libegl-dev libgl-dev xvfb ninja-build cmake python3-venv
python3 -m venv .venv && . .venv/bin/activate && pip install -r tools/requirements.lock
```

### Sandbox without sudo or pip (verified in the planning sandbox; L-20261007T150000Z)
The core library, its tests and the Python tools build and run locally this way. The plugin builds only in CI (D-022).
```bash
T=$HOME/tools; mkdir -p $T && cd $T
curl -sSL -o cmake.tgz https://github.com/Kitware/CMake/releases/download/v4.4.4/cmake-4.4.4-linux-x86_64.tar.gz && tar xzf cmake.tgz
curl -sSL -o ninja.zip https://github.com/ninja-build/ninja/releases/download/v1.13.2/ninja-linux.zip && python3 -c "import zipfile;zipfile.ZipFile('ninja.zip').extractall('.')" && chmod +x ninja
export PATH=$T/cmake-4.4.4-linux-x86_64/bin:$T:$PATH
cd -
python3 -m venv --without-pip .venv && curl -sSL https://bootstrap.pypa.io/get-pip.py | .venv/bin/python - && .venv/bin/pip install -r tools/requirements.lock
```

## Build and test commands
```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release            # add -DTPT_BUILD_PLUGIN=OFF without JUCE deps
cmake --build build -j4
ctest --test-dir build -LE perf --output-on-failure
ctest --test-dir build -L perf --output-on-failure                 # Release only (T-024b)
xvfb-run -a build/plugin/tpt_plugin_tests                           # plugin tests (Linux)
.venv/bin/python -m pytest tests/python -k "not t029"
TPT_RENDER=build/tools/render/tpt_render TINYSOL_DIR=reference-data/tinysol .venv/bin/python -m pytest tests/python -k t029
bash tests/scripts/verify_freeze.sh
bash tests/scripts/run_pluginval.sh build/plugin/TrumpetVST_artefacts/Release/VST3/Trumpet.vst3
```

## Verification log
- **Planning sandbox, 2026-10-08, core and tests** (`-DTPT_BUILD_PLUGIN=OFF`, Release, Ninja, CMake 4.4.4, GCC 13.3):
  - Configure succeeded in 61 s (FetchContent of Catch2 and json). The build succeeded.
  - Red run: unit 22/22 test cases fail, integration 19/19, operational 5/5, alloc 1/1, perf 1/1. Every failure is `tpt::NotImplemented`; no fixture or syntax errors.
- **Planning sandbox Python** (venv from the lock):
  - 10 failed (`NotImplementedError`, or missing outputs of S-002/S-005), 2 errors (generator fixture: the stub raises inside a subprocess), 6 skipped (T-029), 2 passed (T-031 guards, D-018).
- **CI `plan-verify` (stubs with plugin, three OSes, run 37748754510):**
  - builds succeeded on ubuntu-24.04, macos-15 and windows-2025 (VS 2026);
  - all ctest suites red;
  - plugin tests: 1 guard passed, 4 failed;
  - pluginval strictness 10 SUCCESS on all three, and auval SUCCEEDED on macOS.

  Details: `research/spikes/plan_verify_ci.md`.
