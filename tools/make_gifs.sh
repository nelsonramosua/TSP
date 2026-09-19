#!/usr/bin/env bash
# make_gifs.sh - regenerate the algorithm animations into docs/gifs/.
# Requires: a C compiler, python3 with matplotlib + Pillow.
# The Trace hook is inert in the normal build; only the animate driver installs a sink.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
CC="${CC:-/usr/bin/gcc}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

echo "Building project objects..."
make >/dev/null

echo "Building animate driver..."
$CC -Wall -Wextra -Iheaders -I. -O2 tools/animate.c \
    $(find builds -name '*.o' ! -name 'TSPTest.o') -o "$TMP/animate" -lm

mkdir -p docs/gifs

# algo-key  ->  caption
render() { # <key> <title>
    "$TMP/animate" "$1" > "$TMP/$1.frames"
    python3 tools/render_gif.py "$TMP/$1.frames" "docs/gifs/$1.gif" "$2 (Eil51)"
}

render nn           "Nearest Neighbour"
render farthest     "Farthest Insertion"
render christofides "Christofides"
render 2opt         "2-Opt"
render lk           "Lin-Kernighan"
render sa           "Simulated Annealing"
render aco          "Ant Colony"
render ga           "Genetic Algorithm"
render grasp        "GRASP"

echo "Done. GIFs in docs/gifs/"