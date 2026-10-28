#!/usr/bin/env bash
# Pushes a directory of gate evidence (PNG/WAV and SHA256SUMS) to the orphan branch evidence/<gate> (force-pushed: the
# evidence branch is a rebuildable artefact, never the implementation branch). usage: push_evidence.sh <gate> <dir>
set -euo pipefail
GATE="$1"; SRC="$(cd "$2" && pwd)"
ROOT="$(git rev-parse --show-toplevel)"; cd "$ROOT"
W="$(dirname "$ROOT")/trumpet-evidence-$GATE"     # outside the repository directory (T-031 finds no WAV in the tree)
(cd "$SRC" && find . -type f ! -name SHA256SUMS | sort | sed 's|^\./||' | xargs sha256sum > SHA256SUMS)
if [ -n "${GITHUB_TOKEN:-}" ]; then
  export GIT_CONFIG_COUNT=1
  export GIT_CONFIG_KEY_0="http.https://github.com/.extraheader"
  export GIT_CONFIG_VALUE_0="AUTHORIZATION: basic $(printf 'x-access-token:%s' "$GITHUB_TOKEN" | base64 -w0)"
  git config user.name "github-actions[bot]"; git config user.email "41898282+github-actions[bot]@users.noreply.github.com"
fi
rm -rf "$W"; git worktree prune
git worktree add --detach "$W"
git -C "$W" checkout --orphan "evidence/$GATE"
test "$(git -C "$W" symbolic-ref --short HEAD)" = "evidence/$GATE"
git -C "$W" rm -rfq . || true
cp -r "$SRC"/. "$W"/
if [ -f "$ROOT/ATTRIBUTION.md" ]; then awk '/^## TinySOL/{f=1} /^## Fr/{f=0} f' "$ROOT/ATTRIBUTION.md" > "$W/ATTRIBUTION.txt"; fi
git -C "$W" add -A
git -C "$W" commit -qm "evidence for $GATE from $(git rev-parse --short HEAD)"
git -C "$W" push --force origin "evidence/$GATE"
git worktree remove --force "$W"
echo "pushed evidence/$GATE"
