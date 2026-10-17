#!/usr/bin/env python3
"""Decode Rolling Thunder rev. 3 graphics ROMs into Genesis-ready data.

The graphics layouts and PROM ordering follow the publicly documented MAME
Namco System 86 video layout. This script only reads graphics/PROM data from
the supplied ROM archive. It never copies or emits CPU program regions.
"""
from __future__ import annotations

import argparse
import json
from pathlib import Path
from hashlib import sha1
import struct
import wave
from zipfile import ZipFile

SPRITE_ROMS = [f"rt1_{n}.{suffix}" for n, suffix in zip(
    range(9, 17), ["12h", "12k", "12l", "12m", "12p", "12r", "12t", "12u"]
)]
TILE0_ROMS = ["rt1_7.7r", "rt1_8.7s"]
TILE1_ROMS = ["rt1_5.4r", "rt1_6.4s"]
PROM_ROMS = ["rt1-1.3r", "rt1-2.3s", "rt1-3.4v", "rt1-4.5v", "rt1-5.6u"]
SAMPLE_ROMS = ["rt1_21.f3", "rt2_22.h3"]
EXPECTED_SHA1 = {
    "rt1_9.12h": "483b4de79f2429236f45c32ec56b97a9a90574a3",
    "rt1_10.12k": "ba9400fb19d29a285897cc3a2d4d739ce845f897",
    "rt1_11.12l": "a771e922ad868ca1e008d08a8ff5fdf28aa315fc",
    "rt1_12.12m": "ead143c2730a77911839a25734550188533c7b96",
    "rt1_13.12p": "a04b57424acbf2584f736b55740d613a1aae2b8b",
    "rt1_14.12r": "b4d6de4eec47856a62f396f55d531fbf345cf12a",
    "rt1_15.12t": "949ae8b00b94bfa5bc2d07888aafbaaaea559b06",
    "rt1_16.12u": "8d27c49b36d5e23dd446c150ada3853eec75e4c1",
    "rt1-1.3r": "4636d6b8ba7611b11d4863fab02475dc4a619eaf",
    "rt1-2.3s": "935f140bfa7e6f8cebafa7f1b0de99dd319273d4",
    "rt1-3.4v": "ca5fea028674882a61507ac7c89ada96f5b2674d",
    "rt1-4.5v": "8ca94e22110b20d2ecdf03610bcc89ff4245920f",
    "rt1-5.6u": "e1a3e1383186d036fba6dc8a8681f48f24f59281",
    "rt1_7.7r": "1ed63b421a93960668cb4558c1ca1b3c86b1f6be",
    "rt1_8.7s": "82fe0adf6c5b3abef19031646e1eca1585dcc481",
    "rt1_5.4r": "70f7f1e29527044eae405f58af08bad3097990bd",
    "rt1_6.4s": "4a8f1e024e5be4d76f2c99d506ae7da86af3d1f5",
    "rt1_21.f3": "e0a679353491190b6d4f0355324456a1bd7c8a7a",
    "rt2_22.h3": "4c9ce4e4c8e756a743c541f670a6741b520125e3",
}


def bit(data: bytes, offset: int) -> int:
    # MAME's readbit() numbers the most-significant bit of each byte as bit 0.
    return (data[offset >> 3] >> (7 - (offset & 7))) & 1


def decode_sprites(raw: bytes) -> tuple[bytes, bytes]:
    """Return row-major 8x8 Genesis packed-pixel tiles and arcade pixel indices."""
    if len(raw) != 0x80000:
        raise ValueError(f"expected 512 KiB sprite region, got {len(raw):#x}")
    tile_data = bytearray()
    all_pixels = bytearray()
    for code in range(1024):
        pixels = [[0] * 32 for _ in range(32)]
        base = code * 4096
        for y in range(32):
            yoff = (y & 15) * 64 + (2048 if y >= 16 else 0)
            for x in range(32):
                xoff = (x & 15) * 4 + (1024 if x >= 16 else 0)
                po = base + xoff + yoff
                # The first layout plane is the most significant pen bit.
                # Reversing this order preserves silhouettes but permutes
                # colors; the game-over character capture exposes all 16 pens.
                pixels[y][x] = sum(bit(raw, po + plane) << (3 - plane)
                                   for plane in range(4))
        for row in pixels:
            all_pixels.extend(row)
        # Store the 4x4 source tile grid in row order for crop lookup.
        # Each native Genesis pattern byte holds two pixels, high nibble first.
        # Runtime sprite upload reorders these tiles into VDP column order.
        for ty in range(0, 32, 8):
            for tx in range(0, 32, 8):
                for y in range(8):
                    for x in range(0, 8, 2):
                        left = pixels[ty + y][tx + x]
                        right = pixels[ty + y][tx + x + 1]
                        left = 0 if left == 15 else left + 1
                        right = 0 if right == 15 else right + 1
                        tile_data.append((left << 4) | right)
    return bytes(tile_data), bytes(all_pixels)


def shuffle_tile_rom(raw: bytes) -> bytes:
    """Apply Namco System 86's packed-ROM to planar-ROM shuffle.

    The first two planes are nibble-packed in the first two thirds of each
    tile region. The final third supplies plane three with inverted polarity.
    This is the same transformation performed by MAME's init_namco86().
    """
    if len(raw) % 3:
        raise ValueError("tile ROM region must split into three equal planes")
    packed_size = len(raw) * 2 // 3
    if packed_size & 1:
        raise ValueError("packed tile ROM data must contain byte pairs")
    output = bytearray(len(raw))
    half = packed_size // 2
    third_plane = packed_size
    for source in range(0, packed_size, 2):
        first, second = raw[source], raw[source + 1]
        output[source // 2] = ((first << 4) | (second & 0x0f)) & 0xff
        output[half + source // 2] = (first & 0xf0) | (second >> 4)
        output[third_plane + source // 2] = raw[third_plane + source // 2] ^ 0xff
    return bytes(output)


def decode_tiles(raw: bytes) -> bytes:
    """Decode shuffled MAME gfx_8x8x3_planar data to Mega Drive patterns."""
    if len(raw) % 3:
        raise ValueError("3-plane tile ROM region must split evenly into planes")
    plane_bytes = len(raw) // 3
    plane_bits = plane_bytes * 8
    tile_count = plane_bytes // 8
    output = bytearray()
    # MAME gfx_8x8x3_planar plane offsets are {2/3, 1/3, 0}; the
    # decoder assigns those offsets pixel weights 4, 2, and 1 respectively.
    for code in range(tile_count):
        rows = []
        for y in range(8):
            row = []
            for x in range(8):
                pixel = 0
                bitoff = code * 64 + y * 8 + x
                for plane, region_part in enumerate((2, 1, 0)):
                    pixel |= bit(raw, region_part * plane_bits + bitoff) << (2 - plane)
                # Namco tilemaps use pen 7 as transparent. Reserve Mega Drive
                # index 0 for transparency and shift the seven visible pens.
                row.append(0 if pixel == 7 else pixel + 1)
            rows.append(row)
        for row in rows:
            for plane in range(4):
                value = 0
                for x, pixel in enumerate(row):
                    value |= ((pixel >> plane) & 1) << (7 - x)
                output.append(value)
    return bytes(output)


def rgb_palette(prom: bytes) -> list[list[int]]:
    """Approximate the original resistor DAC's 4-bit component weighting."""
    if len(prom) < 0x400:
        raise ValueError("palette PROM region is incomplete")
    # MAME network uses 2.2k, 1k, 470, 220 ohm resistors. These normalized
    # weights retain the board's bit ordering and are quantized to Genesis RGB.
    conductance = [1 / r for r in (2200, 1000, 470, 220)]
    scale = 255 / sum(conductance)
    weights = [round(v * scale) for v in conductance]
    colors = []
    for i in range(512):
        rg, b = prom[i], prom[i | 0x200]
        r = sum(weights[k] for k in range(4) if (rg >> k) & 1)
        g = sum(weights[k] for k in range(4) if (rg >> (k + 4)) & 1)
        blue = sum(weights[k] for k in range(4) if (b >> k) & 1)
        # Genesis uses 3 bits per component. Return expanded values so PNG
        # tooling can display the palette without changing the source lookup.
        colors.append([round(c / 255 * 7) * 255 // 7 for c in (r, g, blue)])
    return colors


def decode_samples(sample_roms: list[bytes], output: Path) -> list[dict[str, int]]:
    """Decode Namco 63701X unsigned PCM + silence-run format (6 kHz)."""
    manifest = []
    for bank, rom in enumerate(sample_roms):
        for sample_id in range(1, 32):
            table = 2 * (sample_id - 1)
            start = (rom[table] << 8) | rom[table + 1]
            # The 63701X uses the table result verbatim. Some valid starts in
            # this set point inside the 0x40-byte pointer table; do not discard
            # them based on a guessed minimum data offset.
            if start >= len(rom):
                continue
            pos = start
            pcm = bytearray()
            while pos < len(rom):
                value = rom[pos]
                pos += 1
                if value == 0xFF:
                    break
                if value == 0:
                    if pos >= len(rom):
                        break
                    count = rom[pos] + 1
                    pos += 1
                    pcm.extend(b"\x80" * count)
                else:
                    pcm.append(value)
            else:
                continue
            if not pcm:
                continue
            # De-duplicate empty/alias entries, retaining the first bank/id.
            rom_bank = bank * 2  # second 64 KiB EPROM is mapped at block 2
            file = output / f"bank{rom_bank}_sample{sample_id:02d}.wav"
            with wave.open(str(file), "wb") as wav:
                wav.setnchannels(1)
                wav.setsampwidth(1)
                wav.setframerate(6000)
                wav.writeframes(pcm)
            manifest.append({"bank": rom_bank, "sample": sample_id, "start": start, "frames": len(pcm)})
    return manifest


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("archive", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    args.output.mkdir(parents=True, exist_ok=True)
    with ZipFile(args.archive) as zf:
        names = set(zf.namelist())
        needed = SPRITE_ROMS + TILE0_ROMS + TILE1_ROMS + PROM_ROMS + SAMPLE_ROMS
        missing = [n for n in needed if n not in names]
        if missing:
            raise SystemExit("ROM archive missing: " + ", ".join(missing))
        for name, expected in EXPECTED_SHA1.items():
            actual = sha1(zf.read(name)).hexdigest()
            if actual != expected:
                raise SystemExit(f"{name}: SHA1 mismatch ({actual}), expected rev. 3 {expected}")
        sprite_region = b"".join(zf.read(n) for n in SPRITE_ROMS)
        tile0_region = b"".join(zf.read(n) for n in TILE0_ROMS)
        tile1_region = b"".join(zf.read(n) for n in TILE1_ROMS)
        proms = b"".join(zf.read(n) for n in PROM_ROMS)
        sample_roms = [zf.read(n) for n in SAMPLE_ROMS]
    genesis_tiles, pixels = decode_sprites(sprite_region)
    (args.output / "sprites_genesis_4bpp.bin").write_bytes(genesis_tiles)
    tile0 = decode_tiles(shuffle_tile_rom(tile0_region))
    tile1 = decode_tiles(shuffle_tile_rom(tile1_region))
    (args.output / "tiles0_genesis_4bpp.bin").write_bytes(tile0)
    (args.output / "tiles1_genesis_4bpp.bin").write_bytes(tile1)
    (args.output / "sprites_indices_32x32.bin").write_bytes(pixels)
    (args.output / "sprite_color_lookup.bin").write_bytes(proms[0xC00:0x1400])
    (args.output / "tile_color_lookup.bin").write_bytes(proms[0x400:0xC00])
    (args.output / "tile_address_decoder.bin").write_bytes(proms[0x1400:0x1420])
    palette_rgb = rgb_palette(proms)
    (args.output / "sprite_palette_rgb.json").write_text(
        json.dumps(palette_rgb, separators=(",", ":")) + "\n"
    )
    # Native Mega Drive CRAM words for the 512 indirect colors. Namco's
    # sprite lookup PROM selects these with 256 + PROM[group * 16 + pen].
    palette_words = bytearray()
    for red, green, blue in palette_rgb:
        # Mega Drive CRAM stores blue in bits 9-11, green in 5-7, and red
        # in 1-3. Keep the resistor-derived RGB channel order intact.
        quantize = lambda channel: (channel * 7 + 127) // 255
        word = (quantize(blue) << 9) | (quantize(green) << 5) | (quantize(red) << 1)
        palette_words.extend(struct.pack(">H", word))
    (args.output / "sprite_palette_vdp.bin").write_bytes(palette_words)
    voice_dir = args.output / "voices_6khz"
    voice_dir.mkdir(exist_ok=True)
    voice_manifest = decode_samples(sample_roms, voice_dir)
    (voice_dir / "manifest.json").write_text(json.dumps(voice_manifest, indent=2) + "\n")
    print(f"Decoded 1024 32x32 sprite frames: {len(genesis_tiles):,} Genesis tile bytes")
    print(f"Decoded background tile banks: {len(tile0) // 32:,} + {len(tile1) // 32:,} patterns")
    print(f"Sprite PROM lookup: {len(proms[0xC00:0x1400]):,} bytes; palettes: 512")
    print(f"Decoded {len(voice_manifest)} PCM samples at the board's 6 kHz rate")


if __name__ == "__main__":
    main()
