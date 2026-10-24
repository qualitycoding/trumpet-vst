#!/usr/bin/env bash
# Idempotent environment setup (plan/ENVIRONMENT.md). Installs/locates tools, creates the venv, configures build/.
set -euo pipefail
cd "$(dirname "$0")/.."
T=$HOME/tools; mkdir -p "$T"
export PATH=$T/cmake-4.4.4-linux-x86_64/bin:$T:$PATH
if ! command -v cmake >/dev/null; then
  curl -sSL -o "$T/cmake.tgz" https://github.com/Kitware/CMake/releases/download/v4.4.4/cmake-4.4.4-linux-x86_64.tar.gz
  tar xzf "$T/cmake.tgz" -C "$T"
fi
if ! command -v ninja >/dev/null; then
  curl -sSL -o "$T/ninja.zip" https://github.com/ninja-build/ninja/releases/download/v1.13.2/ninja-linux.zip
  python3 -c "import zipfile;zipfile.ZipFile('$T/ninja.zip').extractall('$T')"; chmod +x "$T/ninja"
fi
V=$HOME/.venvs/trumpet-vst
if [ ! -x "$V/bin/pip" ]; then
  rm -rf "$V"
  python3 -m venv "$V" 2>/dev/null || { python3 -m venv --without-pip "$V"; curl -sSL https://bootstrap.pypa.io/get-pip.py | "$V/bin/python" -; }
fi
ln -sfn "$V" .venv
.venv/bin/pip install -q -r tools/requirements.lock
PLUGIN=ON
pkg-config --exists x11 freetype2 alsa 2>/dev/null || PLUGIN=OFF
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DTPT_BUILD_PLUGIN=$PLUGIN
echo "bootstrap done (plugin=$PLUGIN)"
