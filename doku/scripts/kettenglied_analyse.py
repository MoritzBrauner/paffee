#!/usr/bin/env python3
"""Vermisst das LEGO-Kettenglied 57518 aus der offiziellen LDraw-Teilebibliothek.

Liefert die Zahlen aus doku/01-kettenglied-57518.md: Gesamtbreite, Teilung,
Abstand Gelenkachse -> Boden- bzw. Laufflaeche und die Innenkontur ueber der Breite.

1 LDU = 0,4 mm. In LDraw ist -Y oben: die Bodenseite des Glieds liegt bei y = -14,
die Innenseite (Laufrollenseite) bei positivem y.
"""
import collections
import urllib.request

LDU = 0.4
BASE = "https://library.ldraw.org/library/official/parts/"
FILES = ["57518.dat", "s/57518s01.dat", "s/57518s02.dat"]


def load(name):
    with urllib.request.urlopen(BASE + name) as r:
        return r.read().decode("utf-8", "replace")


def polygons(text):
    """Alle Dreiecke und Vierecke einer .dat-Datei als Punktlisten."""
    out = []
    for line in text.splitlines():
        p = line.split()
        if p and p[0] in ("3", "4"):
            n = int(p[0])
            v = [float(x) for x in p[2:2 + 3 * n]]
            out.append([(v[3 * i], v[3 * i + 1], v[3 * i + 2]) for i in range(n)])
    return out


def main():
    polys = []
    for f in FILES:
        polys += polygons(load(f))
    # s57518s02 ist nur eine Haelfte des Gelenks, in s01 gespiegelt referenziert
    polys += [[(-x, y, z) for (x, y, z) in poly] for poly in polys]

    xs = [x for poly in polys for (x, _, _) in poly]
    ys = [y for poly in polys for (_, y, _) in poly]

    print("Breite gesamt          : %.1f LDU = %.2f mm" % (max(xs) - min(xs), (max(xs) - min(xs)) * LDU))
    print("Gelenkachse -> Boden   : %.1f LDU = %.2f mm" % (-min(ys), -min(ys) * LDU))
    print("Gelenkachse -> Innen   : %.1f LDU = %.2f mm" % (max(ys), max(ys) * LDU))
    print("Gliedhoehe             : %.1f LDU = %.2f mm" % (max(ys) - min(ys), (max(ys) - min(ys)) * LDU))
    print("Teilung                : 30 LDU = 12.00 mm  (Gelenkzylinder bei z=0 und z=-30)")
    print()
    print("Innenkontur (innerste Flaeche je x-Band, Abstand von der Mitte):")
    grid = collections.defaultdict(list)
    for poly in polys:
        for (x, y, _) in poly:
            grid[round(abs(x) / 2) * 2].append(y)
    for band in sorted(grid):
        print("  %5.1f mm : y = %+5.1f LDU (%+5.2f mm)"
              % (band * LDU, max(grid[band]), max(grid[band]) * LDU))


if __name__ == "__main__":
    main()
