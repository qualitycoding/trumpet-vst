#!/usr/bin/env bash
# Append one results directory to the orphan branch ci-results (D-026 R-018, R-020). Never force-pushes.
# usage: push_results.sh <source-dir-with-files> ; env: GITHUB_TOKEN, GITHUB_REPOSITORY, SRC_SHA, RUNNER_TEMP
set -euo pipefail
SRC="$1"; SHA="${SRC_SHA:?}"; W="${RUNNER_TEMP:?}/ci-results-wt"
git config user.name "github-actions[bot]"; git config user.email "41898282+github-actions[bot]@users.noreply.github.com"
# Authentication only for this script, through the environment (never in argv, a URL or .git/config).
export GIT_CONFIG_COUNT=1
export GIT_CONFIG_KEY_0="http.https://github.com/.extraheader"
export GIT_CONFIG_VALUE_0="AUTHORIZATION: basic $(printf 'x-access-token:%s' "$GITHUB_TOKEN" | base64 -w0)"
rm -rf "$W"; git worktree prune
if git fetch origin ci-results 2>/dev/null; then
  git worktree add -B ci-results "$W" FETCH_HEAD
else
  git worktree add --detach "$W"
  git -C "$W" checkout --orphan ci-results
  git -C "$W" rm -rfq . || true
fi
test "$(git -C "$W" symbolic-ref --short HEAD)" = ci-results
mkdir -p "$W/results/$SHA"
cp -r "$SRC"/. "$W/results/$SHA/"
awk '/^## TinySOL/{f=1} /^## Fréour/{f=0} f' ATTRIBUTION.md > "$W/ATTRIBUTION.txt"
git -C "$W" add -A
if git -C "$W" diff --cached --quiet; then echo "nothing to commit"; exit 0; fi
git -C "$W" commit -qm "realism results for $SHA"
for i in 1 2 3; do
  git -C "$W" push origin ci-results && exit 0
  git -C "$W" fetch origin ci-results && git -C "$W" rebase FETCH_HEAD
done
echo "::error::push to ci-results failed"; exit 1
