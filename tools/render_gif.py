#!/usr/bin/env python3
"""Render an algorithm trace (from tools/animate.c) into an animated GIF.

Usage:  render_gif.py <frames.txt> <out.gif> "<Title>" [max_frames]

Input format (see tools/animate.c):
    COORDS <N>
    <x> <y>            (N lines)
    FRAME <count> <cost>
    <v0 v1 ... >       (count indices)
    ... repeated
"""
import sys
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from PIL import Image


def parse(path):
    coords, frames = [], []
    with open(path) as f:
        lines = f.read().splitlines()
    i = 0
    n = int(lines[i].split()[1]); i += 1
    for _ in range(n):
        x, y = lines[i].split(); coords.append((float(x), float(y))); i += 1
    while i < len(lines):
        if not lines[i].startswith("FRAME"):
            i += 1; continue
        _, count, cost = lines[i].split(); i += 1
        idx = [int(v) for v in lines[i].split()] if int(count) > 0 else []
        i += 1
        frames.append((idx, float(cost)))
    return coords, frames


def subsample(frames, cap):
    if len(frames) <= cap:
        return frames
    # keep evenly spaced frames but always include the last (final tour)
    step = len(frames) / (cap - 1)
    picked = [frames[int(k * step)] for k in range(cap - 1)]
    picked.append(frames[-1])
    return picked


def main():
    if len(sys.argv) < 4:
        print(__doc__); sys.exit(2)
    src, out, title = sys.argv[1], sys.argv[2], sys.argv[3]
    cap = int(sys.argv[4]) if len(sys.argv) > 4 else 90

    coords, frames = parse(src)
    if not frames:
        print("no frames"); sys.exit(1)
    frames = subsample(frames, cap)

    xs = [c[0] for c in coords]; ys = [c[1] for c in coords]
    pad_x = (max(xs) - min(xs)) * 0.05 + 1
    pad_y = (max(ys) - min(ys)) * 0.05 + 1
    n = len(coords)

    images = []
    for idx, cost in frames:
        fig, ax = plt.subplots(figsize=(4.6, 4.6), dpi=80)
        ax.scatter(xs, ys, s=14, c="#444", zorder=3)
        if idx:
            # closed if it is a full tour, or explicitly marked by a repeated first vertex
            marked = len(idx) > 1 and idx[0] == idx[-1]
            closed = (len(idx) == n) or marked
            if marked:
                idx = idx[:-1]
            seq = idx + [idx[0]] if closed else idx
            px = [coords[v][0] for v in seq]
            py = [coords[v][1] for v in seq]
            ax.plot(px, py, "-", lw=1.4, c="#1f77b4" if closed else "#ff7f0e", zorder=2)
            # highlight the current head while constructing
            if not closed:
                ax.scatter([coords[idx[-1]][0]], [coords[idx[-1]][1]], s=45, c="#d62728", zorder=4)
        ax.set_title(f"{title}   cost = {cost:.0f}", fontsize=11)
        ax.set_xlim(min(xs) - pad_x, max(xs) + pad_x)
        ax.set_ylim(min(ys) - pad_y, max(ys) + pad_y)
        ax.set_xticks([]); ax.set_yticks([])
        ax.set_aspect("equal", adjustable="box")
        fig.tight_layout()
        fig.canvas.draw()
        images.append(Image.frombytes("RGBA", fig.canvas.get_width_height(),
                                      fig.canvas.buffer_rgba().tobytes()).convert("P", palette=Image.ADAPTIVE))
        plt.close(fig)

    # hold the final frame a bit longer by repeating it
    durations = [110] * (len(images) - 1) + [1600]
    images = images + [images[-1]] * 6
    durations = durations + [1600] * 6
    images[0].save(out, save_all=True, append_images=images[1:], duration=durations, loop=0, optimize=True)
    print(f"wrote {out}  ({len(frames)} frames, {n} cities)")


if __name__ == "__main__":
    main()