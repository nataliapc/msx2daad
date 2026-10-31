#!/usr/bin/env python3
"""
img9990.py: image converter for the GFX9000 (V9990) build of MSX2DAAD.

Writes .IMG files for SCREEN=16 (bitmap mode B1, BD16 colour: GRB555, two
bytes per pixel, little endian). The file uses the version 2 chunk layout of
imgwizard.php with two chunk types of its own:

    "IMG" + 'G'                       magic and screen type
    128 Info                          { 1, chunks, width, height, 5 (BD16), 3 (GRB555), 4 (V9990) }
    1   Palette (optional)            data 16 GRB555 words: the colours of INK, PAPER...
    129 FixedImg (optional)           { offsetX, offsetY } or
                                      { offsetX, offsetY, winX, winY, winW, winH } (window in characters)
    22  V9990Cmd                      extra header { n }, data n * 21 bytes (R#32-R#52)
    23  V9990CmdData (repeated)       extra header { compressor, uncompressed size }, data

Every chunk starts with { type (1 byte), extra header size (2), data size (2) }.
The V9990Cmd holds one LMMC over the whole rectangle; the V9990CmdData chunks
carry its pixel bytes, split so that every payload is at most 2040 bytes.
Compressors: 0 RAW, 1 RLE (mark, count, value; count 0 ends), 3 ZX0 (each
chunk compressed on its own, at most 32768 uncompressed bytes).
The interpreter decodes the chunks to a linear VRAM buffer and draws the
rectangle with one BMXL, which gives the same picture as the LMMC.

ZX0 needs the zx0 compressor by Einar Saukas (https://github.com/einar-saukas/ZX0),
found in PATH, in the ZX0 environment variable or given with --zx0.

Commands:
    img9990.py image  <in.png|in.SC8> <out.IMG> [options]   picture (window relative)
    img9990.py font   <in.png|in.SC8> <out.IMG> [options]   font: white glyphs on black
    img9990.py list   <file.IMG>                            show the chunks
"""

import argparse
import os
import shutil
import struct
import subprocess
import sys
import tempfile

try:
    from PIL import Image
except ImportError:
    sys.exit("ERROR: Pillow is needed (pip install Pillow)")

CHUNK_INFO = 128
CHUNK_FIXEDIMG = 129
CHUNK_PALETTE = 1
CHUNK_G9KCMD = 22
CHUNK_G9KDATA = 23
CHUNK_NAMES = {0: "Redirect", 1: "Palette", 17: "CLS", 19: "Pause", 20: "V9938Cmd", 21: "V9938CmdData",
               CHUNK_G9KCMD: "V9990Cmd", CHUNK_G9KDATA: "V9990CmdData", CHUNK_INFO: "Info",
               CHUNK_FIXEDIMG: "FixedImg"}

CMP_RAW, CMP_RLE, CMP_ZX0 = 0, 1, 3
CMP_NAMES = {CMP_RAW: "RAW", CMP_RLE: "RLE", 2: "PLETTER", CMP_ZX0: "ZX0"}
PAYLOAD_MAX = 2040              # IMG_CHUNK_SIZE (2043) - 3 bytes of extra header
ZX0_UNCOMP_MAX = 32768          # per chunk (the decoder counts the chunk bytes in 16 bits)

PIXEL_BD16, PALETTE_GRB555, CHIPSET_V9990 = 5, 3, 4
OP_LMMC, LOG_IMP = 0x10, 0x0C
SCREEN_W, SCREEN_H = 256, 212


# ----------------------------------------------------------------------------- pixels

def load_rgb(path, rect=None):
    """Load a picture as an RGB image. An .SC8 file is an MSX BSAVE of SCREEN 8 VRAM."""
    if path.upper().endswith(".SC8"):
        raw = open(path, "rb").read()
        if raw[0] != 0xFE:
            sys.exit(f"ERROR: {path} is not a BSAVE file")
        vram = raw[7:]
        h = min(len(vram) // 256, SCREEN_H)
        img = Image.new("RGB", (256, h))
        px = img.load()
        for y in range(h):
            for x in range(256):
                c = vram[y * 256 + x]                       # GGGRRRBB
                g, r, b = (c >> 5) & 7, (c >> 2) & 7, c & 3
                px[x, y] = (r * 255 // 7, g * 255 // 7, b * 255 // 3)
    else:
        img = Image.open(path).convert("RGB")
    if rect:
        x, y, w, h = rect
        img = img.crop((x, y, x + w, y + h))
    return img


def to_grb555(r, g, b):
    c5 = lambda v: (v * 31 + 127) // 255
    return (c5(g) << 10) | (c5(r) << 5) | c5(b)


def bd16_bytes(img, mono=False):
    """Pixel bytes in LMMC order (rows, left to right, low byte first)."""
    out = bytearray()
    rgb = img.tobytes()
    for i in range(0, len(rgb), 3):
        r, g, b = rgb[i], rgb[i + 1], rgb[i + 2]
        if mono:
            v = 0x7FFF if (r + g + b) >= 384 else 0x0000
        else:
            v = to_grb555(r, g, b)
        out += struct.pack("<H", v)
    return bytes(out)


# ----------------------------------------------------------------------------- compressors

def rle_encode(data):
    counts = [0] * 256
    for b in data:
        counts[b] += 1
    mark = counts.index(min(counts))
    out = bytearray([mark])
    i, n = 0, len(data)
    while i < n:
        v = data[i]
        run = 1
        while i + run < n and data[i + run] == v and run < 255:
            run += 1
        if run >= 4 or v == mark:
            out += bytes([mark, run, v])
        else:
            out += bytes([v]) * run
        i += run
    out += bytes([mark, 0])
    return bytes(out)


class Zx0:
    def __init__(self, exe):
        self.exe = exe
        self.tmp = tempfile.mkdtemp(prefix="img9990_")

    def __call__(self, data):
        src = os.path.join(self.tmp, "chunk.bin")
        dst = src + ".zx0"
        with open(src, "wb") as f:
            f.write(data)
        subprocess.run([self.exe, "-f", src, dst], check=True, stdout=subprocess.DEVNULL)
        with open(dst, "rb") as f:
            return f.read()

    def close(self):
        shutil.rmtree(self.tmp, ignore_errors=True)


def split_chunks(data, comp, zx0):
    """Split the pixel bytes into (compressor, uncompressed size, payload) chunks."""
    chunks, pos, total = [], 0, len(data)
    while pos < total:
        left = total - pos
        if comp == CMP_RAW:
            size = min(left, PAYLOAD_MAX)
            chunks.append((CMP_RAW, size, data[pos:pos + size]))
        else:
            encode = rle_encode if comp == CMP_RLE else zx0
            cap = left if comp == CMP_RLE else min(left, ZX0_UNCOMP_MAX)
            # largest input whose payload fits (binary search; at least 1 byte)
            lo, hi, best = 1, cap, None
            while lo <= hi:
                mid = (lo + hi) // 2
                payload = encode(data[pos:pos + mid])
                if len(payload) <= PAYLOAD_MAX:
                    best, lo = (mid, payload), mid + 1
                else:
                    hi = mid - 1
            size, payload = best
            chunks.append((comp, size, payload))
        pos += chunks[-1][1]
    return chunks


# ----------------------------------------------------------------------------- file

def chunk(ctype, extra, data=b""):
    return struct.pack("<BHH", ctype, len(extra), len(data)) + extra + data


def build_img(img, comp, zx0, fixed=None, mono=False, window=None, palette=None, more=()):
    w, h = img.size
    if w > SCREEN_W or h > SCREEN_H:
        sys.exit(f"ERROR: {w}x{h} is larger than {SCREEN_W}x{SCREEN_H}")
    data = bd16_bytes(img, mono)
    # WM 0xFFFF: every bit is written, so bit 15 (YS) ends up 0 whatever VRAM held
    cmd = struct.pack("<HHHHHHBBHHHB", 0, 0, 0, 0, w, h, 0, LOG_IMP, 0xFFFF, 0, 0, OP_LMMC)
    body = []
    if palette:
        # Legacy chunk layout, as imgwizard.php writes it: { 1, 32, 32 } and the 32 bytes
        body.append(struct.pack("<BHH", CHUNK_PALETTE, 32, 32)
                    + b"".join(struct.pack("<H", to_grb555(*c)) for c in palette))
    if fixed:
        extra = struct.pack("<HH", *fixed)
        if window:
            extra += struct.pack("<BBBB", *window)
        body.append(chunk(CHUNK_FIXEDIMG, extra))
    body.append(chunk(CHUNK_G9KCMD, bytes([1]), cmd))
    parts = split_chunks(data, comp, zx0)
    for cid, size, payload in parts:
        body.append(chunk(CHUNK_G9KDATA, struct.pack("<BH", cid, size), payload))
    # Further rectangles (the graphic half of a font), each one LMMC with its data
    for extra_img, dx, dy in more:
        ew, eh = extra_img.size
        data2 = bd16_bytes(extra_img, mono)
        body.append(chunk(CHUNK_G9KCMD, bytes([1]),
                          struct.pack("<HHHHHHBBHHHB", 0, 0, dx, dy, ew, eh, 0, LOG_IMP, 0xFFFF, 0, 0, OP_LMMC)))
        more_parts = split_chunks(data2, comp, zx0)
        for cid, size, payload in more_parts:
            body.append(chunk(CHUNK_G9KDATA, struct.pack("<BH", cid, size), payload))
        data += data2
        parts += more_parts
    info = chunk(CHUNK_INFO, struct.pack("<BHHHBBB", 1, len(body) + 1, w, h,
                                         PIXEL_BD16, PALETTE_GRB555, CHIPSET_V9990))
    return b"IMGG" + info + b"".join(body), len(data), parts


def list_img(path):
    d = open(path, "rb").read()
    print(f"{path}: magic {d[:3]!r}, screen '{chr(d[3])}', {len(d)} bytes")
    p = 4
    while p + 5 <= len(d):
        t, a, b = struct.unpack_from("<BHH", d, p)
        name = CHUNK_NAMES.get(t, f"type {t}")
        extra = d[p + 5:p + 5 + a]
        if t == CHUNK_INFO:
            v, n, w, h, px, pal, chip = struct.unpack("<BHHHBBB", extra)
            print(f"  {name}: version {v}, {n} chunks, {w}x{h}, pixel {px}, palette {pal}, chipset {chip}")
        elif t == CHUNK_PALETTE:
            words = struct.unpack_from("<16H", d, p + 5)
            print(f"  {name}: " + " ".join(f"{w:04x}" for w in words))
        elif t == CHUNK_FIXEDIMG:
            print(f"  {name}: offset {struct.unpack_from('<HH', extra)}"
                  + (f", window {tuple(extra[4:8])}" if len(extra) >= 8 else ""))
        elif t == CHUNK_G9KCMD:
            for i in range(extra[0]):
                f = struct.unpack_from("<HHHHHHBBHHHB", d, p + 5 + a + i * 21)
                print(f"  {name}: S({f[0]},{f[1]}) D({f[2]},{f[3]}) N({f[4]},{f[5]}) "
                      f"ARG {f[6]:02X} LOG {f[7]:02X} WM {f[8]:04X} FC {f[9]:04X} OP {f[11]:02X}")
        elif t in (CHUNK_G9KDATA, 21):
            cid, size = struct.unpack("<BH", extra)
            print(f"  {name}: {CMP_NAMES.get(cid, cid)}, {size} -> {b} bytes")
        else:
            print(f"  {name}: extra {a}, data {b}")
        p += 5 + a + b if t >= 20 else 5 + (a if t == 1 else 0)


# ----------------------------------------------------------------------------- main

def find_zx0(opt):
    exe = opt or os.environ.get("ZX0") or shutil.which("zx0")
    if not exe or not os.access(exe, os.X_OK):
        sys.exit("ERROR: zx0 not found: put it in PATH, set ZX0=... or use --zx0 (or use -c rle)")
    return exe


def pair(text):
    a, b = (int(v) for v in text.split(","))
    return a, b


def palette16(text):
    cols = [tuple(bytes.fromhex(v)) for v in text.split(",")]
    if len(cols) != 16 or any(len(c) != 3 for c in cols):
        raise argparse.ArgumentTypeError("16 colours as RRGGBB")
    return cols


def quad(text):
    return tuple(int(v) for v in text.split(","))


def main():
    ap = argparse.ArgumentParser(description="GFX9000 (V9990 BD16) image converter for MSX2DAAD")
    sub = ap.add_subparsers(dest="cmd", required=True)
    for name in ("image", "font"):
        s = sub.add_parser(name)
        s.add_argument("input")
        s.add_argument("output")
        s.add_argument("-c", "--compressor", choices=("zx0", "rle", "raw"), default="zx0")
        s.add_argument("--zx0", help="path of the zx0 compressor")
        s.add_argument("--rect", type=quad, metavar="X,Y,W,H", help="crop the source")
        if name == "font":
            s.add_argument("--gchars", metavar="PNG",
                           help="graphic charset (256x32, the same layout), loaded 264 lines below the font")
        if name == "image":
            s.add_argument("--fixed", type=pair, metavar="X,Y",
                           help="draw at this screen position instead of the current window")
            s.add_argument("--palette", type=palette16, metavar="RRGGBB,...",
                           help="16 colours for INK, PAPER and the rest (the palette a fixed DAAD picture sets)")
            s.add_argument("--window", type=quad, metavar="C,R,W,H",
                           help="with --fixed: the current window becomes this rectangle "
                                "(column, row, width, height in characters), as a fixed DAAD picture does")
    s = sub.add_parser("list")
    s.add_argument("input")
    args = ap.parse_args()

    if args.cmd == "list":
        list_img(args.input)
        return

    comp = {"zx0": CMP_ZX0, "rle": CMP_RLE, "raw": CMP_RAW}[args.compressor]
    zx0 = Zx0(find_zx0(args.zx0)) if comp == CMP_ZX0 else None
    try:
        rect = args.rect
        if args.cmd == "font" and rect is None:
            rect = (0, 0, 256, 32)          # 4 rows of 32 characters (8x8 cells)
        img = load_rgb(args.input, rect)
        window = getattr(args, "window", None)
        if window and (len(window) != 4 or not args.fixed):
            sys.exit("ERROR: --window needs four values and --fixed")
        more = []
        if getattr(args, "gchars", None):
            more.append((load_rgb(args.gchars, (0, 0, 256, 32)), 0, 264))
        out, raw_len, parts = build_img(img, comp, zx0, getattr(args, "fixed", None),
                                        mono=(args.cmd == "font"), window=window,
                                        palette=getattr(args, "palette", None), more=more)
    finally:
        if zx0:
            zx0.close()
    with open(args.output, "wb") as f:
        f.write(out)
    print(f"{args.output}: {img.size[0]}x{img.size[1]} BD16, {raw_len} pixel bytes in "
          f"{len(parts)} {args.compressor.upper()} chunks, {len(out)} bytes")


if __name__ == "__main__":
    main()
