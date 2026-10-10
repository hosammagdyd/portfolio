#!/usr/bin/env bash
# Copies the built Qt WASM ventilator-monitor demo into site/demo/ so the
# portfolio page can embed it. Re-run this whenever qt-showcase changes and
# you rebuild the WASM target.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

SRC="${1:-$REPO_ROOT/qt-showcase/build/wasm_Release/app}"
DEST="$REPO_ROOT/site/demo"

if [[ ! -d "$SRC" ]]; then
    echo "error: WASM build output not found at $SRC" >&2
    echo "Build it first, e.g.:" >&2
    echo "  qt-cmake -G Ninja -S qt-showcase -B qt-showcase/build/wasm_Release -DCMAKE_BUILD_TYPE=MinSizeRel" >&2
    echo "  cmake --build qt-showcase/build/wasm_Release" >&2
    exit 1
fi

mkdir -p "$DEST"
for f in appQtShowcase.html appQtShowcase.js appQtShowcase.wasm qtloader.js qtlogo.svg; do
    cp "$SRC/$f" "$DEST/$f"
done

echo "Synced WASM demo from $SRC to $DEST"
du -sh "$DEST"
