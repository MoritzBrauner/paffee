#!/usr/bin/env python3
"""Erzeugt die Bauschablone doku/assets/laufwerk_m24_1zu11.svg.

STAND: veraltet (1:11). Gebaut wird in 1:10 - siehe doku/04-laufwerk-1zu11.md.

Die Datei ist bei 100 % Ausdruck maßstaeblich (1:11), Raster 10 mm. Rote Linie =
Gelenkachslinie der Kette. Geometrie kommt aus laufwerk.py.
"""
import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from laufwerk import (  # noqa: E402
    IDLER_Y, ROAD_D, ROAD_R, ROAD_X, ROAD_Y, SPROCKET, SPROCKET_R, SPROCKET_TEETH,
    SUPPORT_D, SUPPORT_X, T_INNER, TARGET_LINKS, belt, solve_idler,
)

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets",
                   "laufwerk_m24_1zu11.svg")
W, H = 520, 190          # Blattgroesse in mm
OX, OY = 100, 150        # Nullpunkt auf dem Blatt


def main():
    idler_x = solve_idler(TARGET_LINKS)
    total, _, arcs, circles, normals = belt(idler_x)

    def X(x):
        return OX + x

    def Y(y):
        return OY - y

    # Kettenpfad: abwechselnd Bogen um den Kreis und Gerade zum naechsten
    pts = []
    for i in range(4):
        xi, yi, ri = circles[i]
        a_in, a_out = normals[(i - 1) % 4], normals[i]
        pts.append((xi + ri * math.cos(a_in), yi + ri * math.sin(a_in),
                    xi + ri * math.cos(a_out), yi + ri * math.sin(a_out)))
    d = ["M %.2f %.2f" % (X(pts[0][0]), Y(pts[0][1]))]
    for i in range(4):
        _, _, xo, yo = pts[i]
        d.append("A %.2f %.2f 0 0 0 %.2f %.2f" % (circles[i][2], circles[i][2], X(xo), Y(yo)))
        nxt = pts[(i + 1) % 4]
        d.append("L %.2f %.2f" % (X(nxt[0]), Y(nxt[1])))
    path = " ".join(d) + " Z"

    o = ['<svg xmlns="http://www.w3.org/2000/svg" width="%dmm" height="%dmm" viewBox="0 0 %d %d">'
         % (W, H, W, H),
         '<style>text{font:4px sans-serif;fill:#333}.d{font:3.2px sans-serif;fill:#a00}'
         '.g{stroke:#e8e8e8;stroke-width:.2;fill:none}.g5{stroke:#d0d0d0;stroke-width:.3}'
         '.w{fill:none;stroke:#444;stroke-width:.6}.t{fill:none;stroke:#c00;stroke-width:1.2}'
         '.c{stroke:#888;stroke-width:.25;fill:none}.gr{stroke:#000;stroke-width:.8}</style>',
         '<rect width="%d" height="%d" fill="white"/>' % (W, H)]
    for gx in range(0, W, 10):
        o.append('<line class="g %s" x1="%d" y1="0" x2="%d" y2="%d"/>'
                 % ("g5" if gx % 50 == 0 else "", gx, gx, H))
    for gy in range(0, H, 10):
        o.append('<line class="g %s" x1="0" y1="%d" x2="%d" y2="%d"/>'
                 % ("g5" if gy % 50 == 0 else "", gy, W, gy))
    o.append('<line class="gr" x1="0" y1="%.1f" x2="%d" y2="%.1f"/>' % (Y(0), W, Y(0)))
    o.append('<text x="4" y="%.1f">Boden</text>' % (Y(0) + 6))
    o.append('<path class="t" d="%s"/>' % path)

    wheels = [(SPROCKET[0], SPROCKET[1], SPROCKET_R)] \
        + [(x, ROAD_Y, ROAD_R) for x in ROAD_X] \
        + [(idler_x, IDLER_Y, ROAD_R)]
    for x, y, r in wheels:
        o.append('<circle class="w" cx="%.2f" cy="%.2f" r="%.2f"/>' % (X(x), Y(y), r))
        o.append('<circle cx="%.2f" cy="%.2f" r="0.8" fill="#444"/>' % (X(x), Y(y)))
        o.append('<line class="c" x1="%.2f" y1="%.2f" x2="%.2f" y2="%.2f"/>'
                 % (X(x), Y(y), X(x), Y(0)))
        o.append('<text class="d" x="%.2f" y="%.2f" text-anchor="middle">%.0f</text>'
                 % (X(x), Y(0) + 5, x))
    for x in SUPPORT_X:
        o.append('<circle class="w" cx="%.2f" cy="%.2f" r="%.1f"/>'
                 % (X(x), Y(83.9), SUPPORT_D / 2))
        o.append('<circle cx="%.2f" cy="%.2f" r="0.7" fill="#444"/>' % (X(x), Y(83.9)))

    o.append('<text x="%.1f" y="%.1f" text-anchor="middle">Triebrad %d Z</text>'
             % (X(SPROCKET[0]), Y(SPROCKET[1]) + 38, SPROCKET_TEETH))
    o.append('<text x="%.1f" y="%.1f" text-anchor="middle">Leitrad</text>'
             % (X(idler_x), Y(IDLER_Y) + 40))
    o.append('<text x="%.1f" y="%.1f">Stuetzrollen O%.0f, Mitte y=83,9</text>'
             % (X(60), Y(110), SUPPORT_D))
    o.append('<text x="%.1f" y="%.1f">Laufrollen O%.1f - Mitte y=%.1f - Abstand 64 (8 Noppen)</text>'
             % (X(30), Y(-20), ROAD_D, ROAD_Y))
    o.append('<text x="%.1f" y="%.1f">Kettenumlauf %.1f mm = %d x 12 mm | '
             'Aufstandslaenge %.0f mm | Modell 1:11</text>'
             % (X(-90), Y(-32), total, TARGET_LINKS, ROAD_X[4]))
    o.append('<text x="%.1f" y="%.1f">VERALTET (1:11), gebaut wird 1:10 - Raster 10 mm - Ausdruck 100%% = 1:11 Bauschablone</text>'
             % (X(-90), Y(-40)))
    o.append("</svg>")

    with open(OUT, "w") as f:
        f.write("\n".join(o))
    print("geschrieben: %s" % os.path.normpath(OUT))


if __name__ == "__main__":
    main()
