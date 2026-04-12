#!/usr/bin/env bash
# Export COMP710 GitHub commit log for docs/GHL – Student ID.txt
# Usage: ./scripts/export-github-log.sh 23196422
set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
if [[ "${1:-}" == "" ]]; then
  echo "Usage: $0 STUDENT_ID" >&2
  echo "Example: $0 23196422" >&2
  exit 1
fi
SID="$1"
OUT="$ROOT/docs/GHL – ${SID}.txt"
if ! git -C "$ROOT" rev-parse --git-dir >/dev/null 2>&1; then
  echo "Error: no git repo at $ROOT (run: git init)" >&2
  exit 1
fi
mkdir -p "$ROOT/docs"
{
  echo "GitHub commit log – student ${SID}"
  echo "Generated: $(date -u +"%Y-%m-%dT%H:%M:%SZ")"
  echo "Repository: $(git -C "$ROOT" remote get-url origin 2>/dev/null || echo "(no origin remote)")"
  echo ""
  echo "----"
  echo ""
  git -C "$ROOT" log --reverse --pretty=format:"%h | %ad | %s%n%b%n" --date=iso
} > "$OUT"
echo "Wrote: $OUT"
