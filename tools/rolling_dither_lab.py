#!/usr/bin/env python3
"""Rolling Thunder Dithering Lab v0.1.

Non-destructive preview of arcade sprite palettes and ordered dithering.
Requires the user's own revision-3 rthunder.zip. Does not modify game files.
"""
from __future__ import annotations

import argparse
from hashlib import sha1
from pathlib import Path
from tkinter import Tk, StringVar, IntVar, filedialog, messagebox, ttk
import tkinter as tk
from zipfile import ZipFile

from PIL import Image, ImageTk

# Reuse the original author's verified Namco System 86 ROM layout.
from extract_assets import EXPECTED_SHA1, SPRITE_ROMS, PROM_ROMS, bit

BAYER4 = (
    (0, 8, 2, 10),
    (12, 4, 14, 6),
    (3, 11, 1, 9),
    (15, 7, 13, 5),
)
MODES = ("Original", "Bayer 4x4", "Bayer Balanced", "Bayer Adaptive")


def decode_sprite(raw: bytes, code: int) -> list[list[int]]:
    """Return source pen numbers for one 32x32 arcade sprite."""
    if len(raw) != 0x80000 or not 0 <= code < 1024:
        raise ValueError("Invalid sprite ROM region or sprite index")
    pixels = []
    base = code * 4096
    for y in range(32):
        row = []
        yo = (y & 15) * 64 + (2048 if y >= 16 else 0)
        for x in range(32):
            xo = (x & 15) * 4 + (1024 if x >= 16 else 0)
            row.append(sum(bit(raw, base + xo + yo + plane) << (3 - plane)
                           for plane in range(4)))
        pixels.append(row)
    return pixels


def source_palette(prom: bytes) -> list[tuple[int, int, int]]:
    """Recreate pre-RGB333 DAC colors (approximate resistor weights)."""
    if len(prom) < 0x400:
        raise ValueError("Incomplete palette PROM")
    resistors = (2200, 1000, 470, 220)
    weights = [round(255 * (1 / r) / sum(1 / v for v in resistors))
               for r in resistors]
    palette = []
    for i in range(512):
        rg, blue = prom[i], prom[i + 0x200]
        channels = (
            sum(weights[k] for k in range(4) if rg & (1 << k)),
            sum(weights[k] for k in range(4) if rg & (1 << (k + 4))),
            sum(weights[k] for k in range(4) if blue & (1 << k)),
        )
        palette.append(tuple(min(255, n) for n in channels))
    return palette


def rgb333(color):
    """Convert each 8-bit RGB component to Mega Drive's 3-bit grid."""
    return tuple(round(channel * 7 / 255) * 255 // 7 for channel in color)


def distance(left, right):
    return sum((left[k] - right[k]) ** 2 for k in range(3))


def luminance(color):
    return .2126 * color[0] + .7152 * color[1] + .0722 * color[2]


def choose_pair(source, target, mode):
    """Find a pair within a single hardware palette, or keep nearest color."""
    nearest = min(range(len(target)), key=lambda i: distance(source, target[i]))
    baseline = distance(source, target[nearest])
    if mode == "Original":
        return nearest, nearest, 0
    best = (nearest, nearest, 0)
    best_score = baseline
    for a in range(len(target)):
        for b in range(a + 1, len(target)):
            contrast = abs(luminance(target[a]) - luminance(target[b]))
            if mode == "Bayer Balanced" and contrast > 56:
                continue
            if mode == "Bayer Adaptive" and contrast > 80:
                continue
            for count in range(1, 16):
                ratio = count / 16
                midpoint = tuple((1 - ratio) * target[a][c] +
                                 ratio * target[b][c] for c in range(3))
                score = distance(source, midpoint)
                if mode == "Bayer Balanced":
                    score += .075 * contrast ** 2
                elif mode == "Bayer Adaptive":
                    score += .045 * contrast ** 2
                if score < best_score:
                    best_score = score
                    best = a, b, count
    if mode == "Bayer Adaptive" and baseline - best_score < 18:
        return nearest, nearest, 0
    return best


def render_sprite(pixels, prom: bytes, group: int, mode: str):
    """Render one sprite with its user-selected palette and Bayer options."""
    if not 0 <= group < 128 or mode not in MODES:
        raise ValueError("Invalid palette or mode")
    lookup = prom[0xC00:0x1400]
    if len(lookup) != 2048:
        raise ValueError("Incomplete sprite color lookup PROM")
    palette = source_palette(prom)
    # Source pen 15 is transparent, so only 15 entries may be dithered.
    sources = [palette[256 + lookup[group * 16 + pen]] for pen in range(15)]
    targets = [rgb333(c) for c in sources]
    pairs = [choose_pair(src, targets, mode) for src in sources]
    image = Image.new("RGB", (32, 32))
    out = image.load()
    changed = 0
    for y, row in enumerate(pixels):
        for x, pen in enumerate(row):
            if pen == 15:
                # Checkerboard is a preview-only background, not an output pen.
                out[x, y] = (48, 50, 58) if (x // 4 + y // 4) % 2 else (65, 67, 75)
                continue
            a, b, count = pairs[pen]
            use_b = BAYER4[y % 4][x % 4] < count
            out[x, y] = targets[b if use_b else a]
            if a != b and use_b:
                changed += 1
    return image, changed


class Lab(Tk):
    def __init__(self, rom_path=None):
        super().__init__()
        self.title("Rolling Thunder MD — Dithering Lab 0.1")
        self.geometry("810x570")
        self.minsize(755, 520)
        self.sprite_data = None
        self.prom = None
        self.original = None
        self.dithered = None
        self.images = [None, None]
        self.rom_label = StringVar(value="No arcade ROM loaded")
        self.status = StringVar(value="Open your own Rolling Thunder revision 3 rthunder.zip.")
        self.sprite_index = IntVar(value=0)
        self.palette_group = IntVar(value=0)
        self.algorithm = StringVar(value=MODES[0])
        self.create_widgets()
        if rom_path is not None:
            self.after(100, lambda: self.open_rom(rom_path))

    def create_widgets(self):
        header = ttk.Frame(self, padding=12)
        header.pack(fill="x")
        ttk.Label(header, text="Rolling Thunder — Dithering Lab",
                  font=("Sans", 17, "bold")).pack(anchor="w")
        ttk.Label(header, text="v0.1 / Sprite palette laboratory / Preview only").pack(anchor="w")
        filebar = ttk.Frame(self, padding=(12, 0, 12, 10))
        filebar.pack(fill="x")
        ttk.Button(filebar, text="Open arcade ROM…",
                   command=self.pick_rom).pack(side="left")
        ttk.Label(filebar, textvariable=self.rom_label).pack(side="left", padx=12)
        controls = ttk.Frame(self, padding=12)
        controls.pack(fill="x")
        ttk.Label(controls, text="Sprite (0–1023)").grid(row=0, column=0, sticky="w")
        ttk.Label(controls, text="Palette (0–127)").grid(row=0, column=1, sticky="w")
        ttk.Label(controls, text="Algorithm").grid(row=0, column=2, sticky="w")
        ttk.Spinbox(controls, from_=0, to=1023, textvariable=self.sprite_index,
                    width=10, command=self.refresh).grid(row=1, column=0, padx=(0, 14))
        ttk.Spinbox(controls, from_=0, to=127, textvariable=self.palette_group,
                    width=10, command=self.refresh).grid(row=1, column=1, padx=(0, 14))
        ttk.Combobox(controls, values=MODES, textvariable=self.algorithm,
                     width=20, state="readonly").grid(row=1, column=2, padx=(0, 14))
        ttk.Button(controls, text="Compare", command=self.refresh).grid(row=1, column=3, padx=4)
        ttk.Button(controls, text="Export PNG", command=self.export).grid(row=1, column=4)
        gallery = ttk.Frame(self, padding=(16, 10))
        gallery.pack(fill="both", expand=True)
        self.preview_labels = []
        for column, label in enumerate(("Original RGB333", "Bayer / Dithered RGB333")):
            pane = ttk.Frame(gallery)
            pane.grid(row=0, column=column, padx=14, sticky="n")
            ttk.Label(pane, text=label, font=("Sans", 11, "bold")).pack(pady=5)
            view = ttk.Label(pane, text="32×32 sprite")
            view.pack()
            self.preview_labels.append(view)
        gallery.columnconfigure(0, weight=1)
        gallery.columnconfigure(1, weight=1)
        ttk.Label(self, textvariable=self.status, padding=12,
                  wraplength=775).pack(fill="x")
        for variable in (self.sprite_index, self.palette_group, self.algorithm):
            variable.trace_add("write", lambda *_: self.after_idle(self.refresh))

    def pick_rom(self):
        path = filedialog.askopenfilename(filetypes=[("ZIP archive", "*.zip")])
        if path:
            self.open_rom(path)

    def open_rom(self, path):
        try:
            with ZipFile(path) as archive:
                for filename in SPRITE_ROMS + PROM_ROMS:
                    data = archive.read(filename)
                    if sha1(data).hexdigest() != EXPECTED_SHA1[filename]:
                        raise ValueError("Unsupported arcade ROM chip: " + filename)
                sprite_data = b"".join(archive.read(n) for n in SPRITE_ROMS)
                prom = b"".join(archive.read(n) for n in PROM_ROMS)
            self.sprite_data, self.prom = sprite_data, prom
            self.rom_label.set(Path(path).name)
            self.refresh()
        except Exception as error:
            self.sprite_data = None
            self.prom = None
            messagebox.showerror("Cannot open arcade ROM", str(error), parent=self)

    def refresh(self):
        if self.sprite_data is None:
            return
        try:
            index = self.sprite_index.get()
            group = self.palette_group.get()
            if not (0 <= index < 1024 and 0 <= group < 128):
                return
            pixels = decode_sprite(self.sprite_data, index)
            self.original, _ = render_sprite(pixels, self.prom, group, "Original")
            self.dithered, changed = render_sprite(
                pixels, self.prom, group, self.algorithm.get())
            for i, picture in enumerate((self.original, self.dithered)):
                self.images[i] = ImageTk.PhotoImage(
                    picture.resize((256, 256), Image.Resampling.NEAREST))
                self.preview_labels[i].configure(image=self.images[i], text="")
            self.status.set(
                f"Sprite {index} · Palette {group} · {self.algorithm.get()} "
                f"· {changed} Bayer secondary-color pixels. "
                "Palette is selected manually: some combinations may not match the game.")
        except (ValueError, tk.TclError):
            return

    def export(self):
        if self.original is None:
            messagebox.showinfo("No preview", "Open an arcade ROM first.", parent=self)
            return
        path = filedialog.asksaveasfilename(
            defaultextension=".png", filetypes=[("PNG", "*.png")],
            initialfile="rolling_thunder_dither_comparison.png")
        if not path:
            return
        canvas = Image.new("RGB", (64, 32))
        canvas.paste(self.original, (0, 0))
        canvas.paste(self.dithered, (32, 0))
        canvas.resize((1024, 512), Image.Resampling.NEAREST).save(path)
        self.status.set("Saved comparison to " + path)


def self_test():
    assert decode_sprite(bytes(0x80000), 0) == [[0] * 32 for _ in range(32)]
    assert rgb333((0, 128, 255)) == (0, 145, 255)
    assert choose_pair((0, 0, 0), [(0, 0, 0)] * 15, "Original") == (0, 0, 0)
    prom = bytes(0xC00) + bytes(range(256)) * 8 + bytes(0x20)
    pixels = [[15 if x == 0 else 0 for x in range(32)] for _ in range(32)]
    for mode in MODES:
        image, _ = render_sprite(pixels, prom, 0, mode)
        assert image.size == (32, 32)
    print("Self-test passed — decoder, RGB333, palette, Bayer modes")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--rom", help="Owner-supplied rthunder.zip revision 3")
    parser.add_argument("--self-test", action="store_true")
    args = parser.parse_args()
    if args.self_test:
        self_test()
    else:
        Lab(args.rom).mainloop()
