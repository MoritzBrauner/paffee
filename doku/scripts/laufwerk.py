#!/usr/bin/env python3
"""Laufwerkslayout und Kettenumlauf fuer den M24 in 1:11 mit LEGO-Kette 57518.

STAND: veraltet. Gebaut wird in 1:10, das Laufwerk ist im CAD festgelegt. Die Rechenmethode
gilt weiter, die Zahlen nicht - siehe doku/04-laufwerk-1zu11.md.

Der Kettenumlauf wird als straff gespannter Gurt um vier Kreise gerechnet
(Triebrad, Laufrolle 1, Laufrolle 5, Leitrad) - die Laufrollen 2-4 liegen auf der
Untertrum-Geraden und aendern die Laenge nicht. Bezugslinie ist immer die
Gelenkachslinie der Kette, nicht die Lauf- oder Bodenflaeche.

Koordinaten: Nullpunkt = Mitte Laufrolle 1, x nach hinten, y ueber Bodenlinie.
"""
import math

# --- Kette (aus der LDraw-Geometrie, siehe kettenglied_analyse.py) -----------
PITCH = 12.0            # Teilung
T_GROUND = 5.6          # Gelenkachse -> Bodenflaeche
T_INNER = 2.8           # Gelenkachse -> innere Lauffaeche

# --- Modellgeometrie 1:11 ----------------------------------------------------
SCALE = 11.0
SPROCKET_TEETH = 13
SPROCKET_R = PITCH / (2 * math.sin(math.pi / SPROCKET_TEETH))   # Teilkreisradius
SPROCKET = (-56.0, 72.0)                                        # 7 N vor / 9 N ueber Boden

ROAD_D = 647.7 / SCALE                    # Laufrolle 25,5" real (Annahme)
ROAD_R = ROAD_D / 2
ROAD_Y = T_GROUND + T_INNER + ROAD_R      # Rollenmitte ueber Boden
ROAD_RP = ROAD_R + T_INNER                # wirksamer Radius fuer die Gelenkachslinie
SPACING = 64.0                            # 8 Noppen
ROAD_X = [i * SPACING for i in range(5)]

IDLER_Y = 64.0                            # 8 N (Annahme)
IDLER_RP = ROAD_RP                        # Leitrad = Laufrollengroesse

TARGET_LINKS = 79
SUPPORT_X = (40.0, 128.0, 216.0)          # Stuetzrollen
SUPPORT_D = 20.0


def belt(idler_x):
    """Gurtlaenge um die vier Kreise, im Gegenuhrzeigersinn geordnet."""
    circles = [
        (SPROCKET[0], SPROCKET[1], SPROCKET_R),
        (ROAD_X[0], ROAD_Y, ROAD_RP),
        (ROAD_X[4], ROAD_Y, ROAD_RP),
        (idler_x, IDLER_Y, IDLER_RP),
    ]
    n = len(circles)
    normals, straights = [], []
    for i in range(n):
        xi, yi, ri = circles[i]
        xj, yj, rj = circles[(i + 1) % n]
        dx, dy = xj - xi, yj - yi
        d = math.hypot(dx, dy)
        normals.append(math.atan2(dy, dx) - math.acos(max(-1.0, min(1.0, (ri - rj) / d))))
        straights.append(math.sqrt(max(0.0, d * d - (ri - rj) ** 2)))
    arcs = [(normals[i] - normals[(i - 1) % n]) % (2 * math.pi) for i in range(n)]
    total = sum(straights) + sum(a * circles[i][2] for i, a in enumerate(arcs))
    return total, straights, arcs, circles, normals


def solve_idler(links, lo=200.0, hi=420.0):
    """Leitradposition, bei der der Umlauf genau links * PITCH ergibt."""
    for _ in range(400):
        mid = (lo + hi) / 2
        if belt(mid)[0] < links * PITCH:
            lo = mid
        else:
            hi = mid
    return (lo + hi) / 2


def tangent(circles, normals, i, j):
    """Beruehrpunkte der Geraden vom Kreis i zum Kreis j."""
    xi, yi, ri = circles[i]
    xj, yj, rj = circles[j]
    a = normals[i]
    return (xi + ri * math.cos(a), yi + ri * math.sin(a)), (xj + rj * math.cos(a), yj + rj * math.sin(a))


def main():
    print("Kette      : Teilung %.1f mm, Gelenkachse %.1f mm ueber Boden" % (PITCH, T_GROUND))
    print("Triebrad   : %d Zaehne, Teilkreis O%.2f mm, Zahnwinkel %.3f Grad"
          % (SPROCKET_TEETH, 2 * SPROCKET_R, 360 / SPROCKET_TEETH))
    print("Laufrolle  : O%.2f mm, Mitte y=%.2f mm, Abstand %.0f mm (%.0f N)"
          % (ROAD_D, ROAD_Y, SPACING, SPACING / 8))
    print("Aufstandslaenge %.0f mm (real %.0f mm)" % (ROAD_X[4], ROAD_X[4] * SCALE))
    print()

    for links in (TARGET_LINKS - 1, TARGET_LINKS, TARGET_LINKS + 1):
        print("%d Glieder je Seite -> Leitradmitte x = %7.2f mm" % (links, solve_idler(links)))
    step = solve_idler(TARGET_LINKS + 1) - solve_idler(TARGET_LINKS)
    print("Leitradweg je Glied: %.2f mm -> Spanner mit +-4 mm genuegt" % step)
    print()

    idler_x = solve_idler(TARGET_LINKS)
    total, straights, arcs, circles, normals = belt(idler_x)
    names = ["Triebrad", "Laufrolle 1", "Laufrolle 5", "Leitrad"]
    print("=== Layout fuer %d Glieder, Umlauf %.2f mm ===" % (TARGET_LINKS, total))
    for i, (x, y, r) in enumerate(circles):
        print("  %-12s x=%8.2f (%6.2f N)  y=%6.2f (%5.2f N)  wirks. R %5.2f  Umschlingung %5.1f Grad"
              % (names[i], x, x / 8, y, y / 8, r, math.degrees(arcs[i])))
    print("  Laufrollen 2-4: x = %.0f / %.0f / %.0f mm" % tuple(ROAD_X[1:4]))
    print("  Zaehne im Eingriff: %.1f von %d"
          % (math.degrees(arcs[0]) / (360 / SPROCKET_TEETH), SPROCKET_TEETH))
    print("  Geraden: Trieb->R1 %.1f | unten %.1f | R5->Leitrad %.1f | oben %.1f mm" % tuple(straights))
    print("  Boegen gesamt: %.1f mm" % sum(a * circles[i][2] for i, a in enumerate(arcs)))
    print()

    # Kontrolle gegen das Original: 75 Glieder x 139,7 mm
    original = 75 * 139.7 / SCALE
    print("Kontrolle: Original 75 x 139,7 mm = %.1f mm bei 1:11, gerechnet %.1f mm (%.1f %%)"
          % (original, total, (total / original - 1) * 100))
    print()

    # Obertrum und Stuetzrollen
    p1, p2 = tangent(circles, normals, 3, 0)
    print("Obertrum (Gelenkachslinie): (%.1f, %.1f) -> (%.1f, %.1f), Neigung %.2f Grad"
          % (p1[0], p1[1], p2[0], p2[1], math.degrees(math.atan2(p2[1] - p1[1], p2[0] - p1[0]))))
    for x in SUPPORT_X:
        t = (x - p1[0]) / (p2[0] - p1[0])
        y = p1[1] + t * (p2[1] - p1[1])
        print("  Stuetzrolle x=%3.0f mm: Gelenkachslinie y=%.2f -> Rollenmitte y=%.2f (O%.0f)"
              % (x, y, y - T_INNER - SUPPORT_D / 2, SUPPORT_D))
    for i, j, label in ((0, 1, "vorderer Anlauf"), (2, 3, "hinterer Anlauf")):
        q1, q2 = tangent(circles, normals, i, j)
        print("  %s: (%.1f, %.1f) -> (%.1f, %.1f), %.1f Grad"
              % (label, q1[0], q1[1], q2[0], q2[1],
                 math.degrees(math.atan2(q2[1] - q1[1], q2[0] - q1[0]))))
    print()
    print("Laufwerkslaenge: %.1f bis %.1f = %.1f mm (Wanne 1:11 = 457 mm)"
          % (SPROCKET[0] - SPROCKET_R - 4, idler_x + ROAD_R,
             (idler_x + ROAD_R) - (SPROCKET[0] - SPROCKET_R - 4)))
    print("Schwingarm: Drehpunkt y=24 mm (3 N), Arm 40 mm (5 N) -> Armwinkel %.1f Grad"
          % math.degrees(math.asin((ROAD_Y - 24) / 40)))
    print("Gliederbedarf: 2 x %d = %d Stueck (mit Reserve 170 bestellen)"
          % (TARGET_LINKS, 2 * TARGET_LINKS))
    print("Fahrweg je Triebradumdrehung: %d x %.0f = %.0f mm"
          % (SPROCKET_TEETH, PITCH, SPROCKET_TEETH * PITCH))


if __name__ == "__main__":
    main()
