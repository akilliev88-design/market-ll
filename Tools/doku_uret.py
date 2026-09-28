"""Miras Market procedural PBR textures (tileable, 1024 px).

Usage: python Tools/doku_uret.py [output_dir]
Default output: AssetInbox/Textures/Miras. Requires numpy and Pillow.
Every texture wraps seamlessly so the triplanar store material can repeat it.
"""
import math
import sys
from pathlib import Path

import numpy as np
from PIL import Image

N = 1024
ROOT = Path(__file__).resolve().parents[1]


def periodic_noise(rng, octaves=((4, 1.0), (8, .5), (16, .25), (32, .12))):
    """Sum of random integer-frequency waves: tiles exactly on a 1024 grid."""
    y, x = np.mgrid[0:N, 0:N] / N
    out = np.zeros((N, N))
    for freq, amp in octaves:
        for _ in range(6):
            fx, fy = rng.integers(-freq, freq + 1, size=2)
            if fx == 0 and fy == 0:
                continue
            phase = rng.uniform(0, 2 * math.pi)
            out += amp * np.sin(2 * math.pi * (fx * x + fy * y) + phase)
    out -= out.min()
    return out / max(out.max(), 1e-6)


def blob(img, cx, cy, rx, ry, angle, color, rng, shade=.55, highlight=.25, edge_dark=.35):
    """Draws a shaded ellipsoid particle with wrap-around (seamless)."""
    r = int(max(rx, ry)) + 2
    for ox in (-N, 0, N):
        for oy in (-N, 0, N):
            x0, y0 = int(cx + ox) - r, int(cy + oy) - r
            x1, y1 = x0 + 2 * r, y0 + 2 * r
            if x1 < 0 or y1 < 0 or x0 >= N or y0 >= N:
                continue
            xs0, ys0, xs1, ys1 = max(x0, 0), max(y0, 0), min(x1, N), min(y1, N)
            yy, xx = np.mgrid[ys0:ys1, xs0:xs1].astype(float)
            dx, dy = xx - (cx + ox), yy - (cy + oy)
            ca, sa = math.cos(angle), math.sin(angle)
            u = (dx * ca + dy * sa) / rx
            v = (-dx * sa + dy * ca) / ry
            d2 = u * u + v * v
            mask = d2 < 1
            if not mask.any():
                continue
            z = np.sqrt(np.clip(1 - d2, 0, 1))
            # light from top-left: normal dot light
            light = np.clip(.35 + .65 * (z * .8 - u * .35 - v * .45), 0, 1.3)
            spec = np.clip(1 - ((u + .35) ** 2 + (v + .4) ** 2) * 6, 0, 1) ** 2 * highlight
            rim = 1 - edge_dark * (1 - z) ** 2
            col = np.array(color, float)[None, None, :] * (shade + (1 - shade) * light)[..., None] * rim[..., None]
            col = col + spec[..., None] * 255
            region = img[ys0:ys1, xs0:xs1]
            region[mask] = np.clip(col[mask], 0, 255)


def particles(name, background, palette, count, rx_range, ry_range, seed, mottle=18, extra=None):
    rng = np.random.default_rng(seed)
    img = np.zeros((N, N, 3))
    img[:] = background
    noise = periodic_noise(rng)
    img += (noise[..., None] - .5) * mottle
    for i in range(count):
        base = np.array(palette[rng.integers(len(palette))], float)
        base = base * rng.uniform(.82, 1.12)
        rx = rng.uniform(*rx_range)
        ry = rng.uniform(*ry_range)
        blob(img, rng.uniform(0, N), rng.uniform(0, N), rx, ry, rng.uniform(0, math.pi), base, rng)
        if extra:
            extra(img, rng, i)
    return img


def save(img, out, name):
    Image.fromarray(np.clip(img, 0, 255).astype(np.uint8), "RGB").save(out / f"{name}.png", optimize=True)
    print(f"MIRAS_TEXTURE={out / (name + '.png')}")


def terrazzo(out):
    """Light beige polished terrazzo, 120 cm = 2 x 2 tiles of 60 cm with thin joints."""
    rng = np.random.default_rng(2011)
    img = np.zeros((N, N, 3))
    img[:] = (206, 194, 173)
    img += (periodic_noise(rng)[..., None] - .5) * 14
    # slight tone difference per tile
    for tx in range(2):
        for ty in range(2):
            img[ty * 512:(ty + 1) * 512, tx * 512:(tx + 1) * 512] *= rng.uniform(.97, 1.03)
    chips = [(245, 242, 235), (160, 150, 138), (120, 112, 104), (228, 206, 170), (182, 160, 130), (90, 86, 82)]
    for _ in range(5200):
        c = np.array(chips[rng.integers(len(chips))], float)
        r = rng.uniform(1.2, 4.5)
        blob(img, rng.uniform(0, N), rng.uniform(0, N), r, r * rng.uniform(.6, 1), rng.uniform(0, math.pi), c, rng, shade=.9, highlight=0, edge_dark=0)
    joint = (150, 140, 125)
    for p in (0, 512):
        img[:, max(p - 2, 0):p + 2] = joint
        img[max(p - 2, 0):p + 2, :] = joint
    img[:, N - 2:] = joint
    img[N - 2:, :] = joint
    save(img, out, "T_Floor_Terrazzo")


def wood(out, name, dark, light, seed, planks=4):
    """Straight-grained veneer: fine fibres along U, soft growth bands, a few plank seams."""
    rng = np.random.default_rng(seed)
    y, x = np.mgrid[0:N, 0:N] / N
    # periodic 1-D fibre profile across the grain
    profile = np.zeros(N)
    t = np.arange(N) / N
    for freq, amp in ((6, 1.0), (13, .6), (29, .45), (61, .35), (127, .25), (251, .18)):
        for _ in range(3):
            profile += amp * rng.uniform(.3, 1) * np.sin(2 * math.pi * (freq * t + rng.uniform()))
    profile = (profile - profile.min()) / (profile.max() - profile.min())
    warp = periodic_noise(rng, ((1, 1.0), (2, .6), (3, .3))) - .5
    plank = np.floor(y * planks).astype(int) % planks
    shift = rng.uniform(0, 1, size=planks)[plank]
    tone = rng.uniform(.9, 1.08, size=planks)[plank]
    idx = ((y * 2 + .035 * warp + shift) % 1.0 * N).astype(int) % N
    grain = profile[idx]
    fibres = periodic_noise(rng, ((64, 1.0), (128, .7), (256, .5)))
    v = np.clip(.75 * grain + .25 * fibres, 0, 1) ** 1.2
    img = (np.array(dark, float)[None, None, :] + (np.array(light, float) - np.array(dark, float))[None, None, :] * v[..., None]) * tone[..., None]
    seam = (np.abs((y * planks) % 1.0) < .0025) | (np.abs((y * planks) % 1.0) > .9975)
    img[seam] *= .72
    save(img, out, name)


def walnut(out):
    """Dark walnut for the feature island and counter."""
    wood(out, "T_Wood_Walnut", (46, 24, 13), (118, 70, 40), 1971)


def oak(out):
    """Honey oak: warm light grain for shelf plinths and headers."""
    wood(out, "T_Wood_Oak", (150, 98, 54), (226, 176, 116), 1983)


def macro(out):
    """Linear grey mask for large-scale tone/roughness variation (grime, sun fade, scuffs). Mean ~0.5."""
    rng = np.random.default_rng(2033)
    low = periodic_noise(rng, ((1, 1.0), (2, .8), (3, .5), (5, .35)))
    mid = periodic_noise(rng, ((8, 1.0), (13, .6), (21, .4)))
    fine = periodic_noise(rng, ((48, 1.0), (96, .6), (192, .4)))
    v = .60 * low + .33 * mid + .07 * fine
    # sparse darker blotches (scuffs, stains)
    blot = np.clip((periodic_noise(rng, ((6, 1.0), (11, .7), (17, .4))) - .72) * 3.0, 0, 1)
    v = v - .25 * blot
    v = (v - v.min()) / (v.max() - v.min())
    v = .5 + (v - v.mean()) * .9
    img = np.repeat(np.clip(v, 0, 1)[..., None] * 255, 3, axis=2)
    save(img, out, "T_Macro_Variation")


def hazelnut(out):
    img = particles("hazelnut", (52, 32, 18), [(128, 74, 36), (146, 88, 44), (112, 62, 30), (160, 104, 58)],
                    900, (40, 52), (36, 48), 7)
    save(img, out, "T_Food_Hazelnut")


def chickpea(out):
    img = particles("chickpea", (90, 64, 30), [(222, 178, 104), (210, 160, 86), (232, 192, 122), (196, 146, 76)],
                    1500, (30, 38), (28, 36), 11)
    save(img, out, "T_Food_Chickpea")


def lentil(out):
    img = particles("lentil", (110, 40, 14), [(222, 104, 38), (232, 120, 48), (206, 88, 30), (240, 138, 60)],
                    2100, (24, 30), (15, 20), 13)
    save(img, out, "T_Food_Lentil")


def pistachio(out):
    img = particles("pistachio", (70, 60, 40), [(214, 196, 150), (200, 180, 132), (226, 210, 168), (208, 190, 146), (190, 172, 128), (218, 202, 160), (130, 150, 78)],
                    900, (56, 68), (36, 44), 17)
    save(img, out, "T_Food_Pistachio")


def main():
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else ROOT / "AssetInbox" / "Textures" / "Miras"
    out.mkdir(parents=True, exist_ok=True)
    terrazzo(out)
    walnut(out)
    oak(out)
    macro(out)
    hazelnut(out)
    chickpea(out)
    lentil(out)
    pistachio(out)
    print("MIRAS_TEXTURES_READY=8")


if __name__ == "__main__":
    main()
