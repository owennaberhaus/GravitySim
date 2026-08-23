#!/usr/bin/env bash
# Web build. Run from WSL after:  source ~/emsdk/emsdk_env.sh
#
#   bash build.sh             -> ~/gsbuild, for local testing with emrun
#   bash build.sh --deploy    -> the site repo's public/, ready to commit
#
# Output is named gravitysim.* and sits at the top of public/, NOT in a
# public/gravitysim/ folder. Vercel has cleanUrls with trailingSlash false, so
# the page is served at /gravitysim with no trailing slash - and a relative
# <script src="index.js"> on that URL resolves to /index.js, one directory too
# high. Flat names sidestep it: from /gravitysim, "gravitysim.js" resolves to
# /gravitysim.js, which is exactly where the file is.
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/.." && pwd)"
SRC="$ROOT/src"
GLM="$ROOT/dependencies/GLM/glm"

SITE="${SITE:-/mnt/d/coding/personal-website}"

# Default output lives on the Linux filesystem - writing thousands of small
# object files across /mnt/c is many times slower, and OneDrive would sync them.
OUT="${OUT:-$HOME/gsbuild}"

if [ "${1:-}" = "--deploy" ]; then
  if [ ! -d "$SITE" ]; then
    echo "site repo not found at $SITE - set SITE=/path/to/personal-website" >&2
    exit 1
  fi
  OUT="$SITE/public"
fi

mkdir -p "$OUT"

em++ "$SRC"/*.cpp -o "$OUT/gravitysim.html" \
  -std=c++17 -O3 \
  -I"$GLM" -I"$GLM/gtc" \
  -sUSE_GLFW=3 \
  -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 \
  -sFULL_ES3=1 \
  -sALLOW_MEMORY_GROWTH=1 \
  -sINITIAL_MEMORY=64MB \
  --shell-file "$HERE/shell.html" \
  -Wno-macro-redefined

echo
ls -lh "$OUT"/gravitysim.*
echo

if [ "${1:-}" = "--deploy" ]; then
  echo "written to $OUT"
  echo "now commit from Windows, not from WSL - see web/README.md"
else
  echo "run it:   emrun --no_browser --port 8080 $OUT/gravitysim.html"
  echo "then open http://localhost:8080/gravitysim.html"
fi
