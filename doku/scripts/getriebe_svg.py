#!/usr/bin/env python3
"""Erzeugt doku/assets/getriebe_konzept.svg - Konzeptblatt des Antriebsstrangs.

Panel 1: kinematisches Schema einer Fahrzeugseite. Motor -> Zweiganggetriebe mit
Reibkupplung -> Durchtrieb durch die Wannenseite -> aussenliegendes Seitenvorgelege ->
Triebrad. In Richtung der Achsabstaende maszstaeblich, axial gestreckt. Die Gegenseite
ist gespiegelt aufgebaut, ein Servo dazwischen schaltet beide ueber ein Gestaenge.
Panel 2: Kupplungsstellung und Kraftfluss je Gang, plus Kenndaten.

Getriebe MOD 1,5: 13:47 (1. Gang, niedrig) und 28:32 (2. Gang, hoch), Achsabstand 45 mm.
Aus Platzgruenden sitzt der 2. Gang zur Wannenseite hin, der 1. Gang zur Motorseite.
Seitenvorgelege 13:47 aussen am Bug, die Getriebewelle (Durchtrieb) durchdringt die
Wannenseite. Modul nicht festgelegt, gezeichnet mit MOD 1,0 (Achsabstand 30 mm). Lager und
Wellen stammen aus dem frueheren Endantrieb-Konzept und sind nicht bestaetigt.
Triebrad 15 Z, Teilkreis 57,72 mm.

Fahrleistung rechnerisch am 3S-Akku: Motordaten (Herstellertabelle JGB37-520 1600 1/min,
lineares Motormodell), Wirkungsgrad und Uebersetzungen kommen aus antrieb.py.
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from antrieb import (  # noqa: E402
    ETA, I_SV, MOTOR_M_STALL_REF, MOTOR_N0_REF, MOTOR_U_REF, TRAVEL_PER_REV, U_BAT, motor,
)

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets",
                   "getriebe_konzept.svg")
MG, ME = 1.5, 1.0
D28, D32, D13, D47 = 28 * MG, 32 * MG, 13 * MG, 47 * MG
DE13, DE47, DSPR = 13 * ME, 47 * ME, 57.72
A_G, A_E = 45.0, 30.0

N_MOT, M_MOT, _ = motor(U_BAT)          # Leerlauf 1/min, Blockiermoment Nm am Akku
YE, YA, YS = 0.0, -A_G, -A_G - A_E

X = dict(m0=2, m1=50, m2=72, li=80, p1a=88, p1b=98, mua=104, mub=116,
         p2a=122, p2b=132, la=140, w1=148, w2=152, fa=156, fb=166, sa=170, sb=182)
W, H = 660, 520
OX, OY, SC = 132, 205, 1.15
o = []


def x(v): return OX + v * SC
def y(v): return OY - v * SC
def add(s): o.append(s)


def rect(a, b, yc, d, cls, rx=0):
    add('<rect class="%s" x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="%d"/>'
        % (cls, x(a), y(yc + d / 2), (b - a) * SC, d * SC, rx))


def txt(px, py, s, cls="l", an="start"):
    add('<text class="%s" x="%.1f" y="%.1f" text-anchor="%s">%s</text>' % (cls, px, py, an, s))


def teeth(a, b, yc, d, n=8):
    for s in (+1, -1):
        yy = y(yc + s * d / 2)
        for i in range(n):
            xx = x(a) + (x(b) - x(a)) * (i + 0.5) / n
            add('<line class="t" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (xx, yy, xx, yy - s * 2))


def de(v, nd):
    return ("%%.%df" % nd % v).replace(".", ",")


def brg(xc, yc):
    for s in (-1, 1):
        add('<rect class="brg" x="%.1f" y="%.1f" width="5.5" height="6.5"/>'
            % (x(xc) - 2.75, y(yc) + (1 if s > 0 else -7.5)))


add('<svg xmlns="http://www.w3.org/2000/svg" width="%dmm" height="%dmm" viewBox="0 0 %d %d">' % (W, H, W, H))
add('<style>text{font:7px sans-serif;fill:#222}.l{font:7px sans-serif}'
    '.s{font:6px sans-serif;fill:#666}.b{font:7.5px sans-serif;font-weight:bold;fill:#111}'
    '.hd{font:11.5px sans-serif;font-weight:bold;fill:#111}'
    '.h2{font:8px sans-serif;font-weight:bold;fill:#111}'
    '.gear{fill:#e4ecf3;stroke:#2b4a63;stroke-width:1}'
    '.gearf{fill:#fbf0dc;stroke:#8a5a12;stroke-width:1}'
    '.mot{fill:#ededed;stroke:#333;stroke-width:1}'
    '.kupplung{fill:#d8ecdb;stroke:#1d6b2e;stroke-width:1.2}'
    '.lining{fill:#b08a4a;stroke:#5c4317;stroke-width:.5}'
    '.shaft{stroke:#111;stroke-width:2.4;stroke-linecap:round}'
    '.axis{stroke:#aaa;stroke-width:.5;stroke-dasharray:8 3 2 3}'
    '.t{stroke:#2b4a63;stroke-width:.6}.brg{fill:#fff;stroke:#111;stroke-width:.8}'
    '.wall{fill:#dcdcdc;stroke:#333;stroke-width:.9}'
    '.hous{fill:none;stroke:#555;stroke-width:1;stroke-dasharray:4 3}'
    '.fork{fill:none;stroke:#1d6b2e;stroke-width:1.5}'
    '.dim{stroke:#a00;stroke-width:.6}.flow{fill:none;stroke:#c00;stroke-width:2}'
    '.box{fill:none;stroke:#ccc;stroke-width:.8}</style>')
add('<rect width="%d" height="%d" fill="white"/>' % (W, H))
txt(22, 24, "Antriebsstrang: Zweiganggetriebe mit Reibkupplung + Seitenvorgelege", "hd")
txt(22, 35, "eine Fahrzeugseite, 1:10, Gegenseite gespiegelt. Quer maszstaeblich (Achsabstaende, "
            "Teilkreise), axial gestreckt. Lager und Wellen aus frueherem Konzept, nicht bestaetigt.", "s")
txt(22, 46, "Blau = fest auf der Welle   |   Orange = frei drehend auf Lager   |   Gruen = Schaltelement", "s")

for yy in (YE, YA, YS):
    add('<line class="axis" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (x(-6), y(yy), x(192), y(yy)))
txt(x(-8), y(YE) - 2, "Eingangswelle", "s", "end")
txt(x(-8), y(YE) + 7, "O12 gedruckt + M5", "s", "end")
txt(x(-8), y(YA) - 2, "Abtrieb = Durchtrieb", "s", "end")
txt(x(-8), y(YA) + 7, "O12 gedruckt + M5", "s", "end")
txt(x(-8), y(YS) - 2, "Triebradwelle", "s", "end")
txt(x(-8), y(YS) + 7, "O10 gedruckt + M5", "s", "end")

rect(X["m0"], X["m1"], YE, 37, "mot", 3)
rect(X["m1"], X["m2"], YE, 26, "mot", 2)
txt(x(6), y(YE) - 9, "JGB37-520", "l")
txt(x(6), y(YE) - 1, "6,25:1, %s V" % de(MOTOR_U_REF, 0), "s")
txt(x(6), y(YE) + 7, "%s min-1" % de(MOTOR_N0_REF, 0), "s")
txt(x(6), y(YE) + 15, "Stall %s Nm" % de(MOTOR_M_STALL_REF, 3), "s")

add('<line class="shaft" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (x(X["m2"]), y(YE), x(138), y(YE)))
add('<line class="shaft" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (x(76), y(YA), x(X["fb"] + 3), y(YA)))
add('<line class="shaft" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (x(X["fa"] - 5), y(YS), x(X["sb"] + 3), y(YS)))

# 1. Gang (niedrig), Motorseite
rect(X["p1a"], X["p1b"], YE, D13, "gear"); teeth(X["p1a"], X["p1b"], YE, D13, 5)
rect(X["p1a"], X["p1b"], YA, D47, "gearf"); teeth(X["p1a"], X["p1b"], YA, D47)
txt(x((X["p1a"] + X["p1b"]) / 2), y(YE + D13 / 2) - 5, "13 Z", "b", "middle")
txt(x((X["p1a"] + X["p1b"]) / 2), y(YA - D47 / 2) + 10, "47 Z", "b", "middle")
# 2. Gang (hoch), aus Platzgruenden zur Wannenseite
rect(X["p2a"], X["p2b"], YE, D28, "gear"); teeth(X["p2a"], X["p2b"], YE, D28)
rect(X["p2a"], X["p2b"], YA, D32, "gearf"); teeth(X["p2a"], X["p2b"], YA, D32)
txt(x((X["p2a"] + X["p2b"]) / 2), y(YE + D28 / 2) - 5, "28 Z", "b", "middle")
txt(x((X["p2a"] + X["p2b"]) / 2), y(YA - D32 / 2) + 10, "32 Z", "b", "middle")

# Reibkupplung: Schiebeteil mit Reibbelag (Schleifpapier) auf beiden Stirnseiten
rect(X["mua"], X["mub"], YA, 21, "kupplung", 1)
for xx in (x(X["mua"]) - 2.5, x(X["mub"])):
    add('<rect class="lining" x="%.1f" y="%.1f" width="2.5" height="%.1f"/>' % (xx, y(YA + 10.5), 21 * SC))
add('<path class="fork" d="M %.1f %.1f l 0 16 l %.1f 0 l 0 -16"/>'
    % (x(X["mua"]) - 3, y(YA - 11), (X["mub"] - X["mua"]) * SC + 6))
xm = x((X["mua"] + X["mub"]) / 2)
add('<line class="fork" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (xm, y(YA - 11) + 16, xm, y(YA - 11) + 40))
add('<line class="fork" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (xm - 70, y(YA - 11) + 40, xm, y(YA - 11) + 40))
txt(xm - 73, y(YA - 11) + 42.5, "zum Servo", "s", "end")
txt(xm - 40, y(YA - 11) + 50, "Reibkupplung mit Schleifpapier-Belag", "s", "middle")
txt(xm - 40, y(YA - 11) + 59, "Gestaenge zum Servo, schaltet beide Seiten", "s", "middle")

brg(X["li"], YA); brg(X["la"], YA)
txt(x(X["li"]) + 2, y(YA) + 22, "2x 6001ZZ", "s", "end")
brg(X["fa"] - 5, YS); brg(X["sa"] - 3, YS)
txt(x(X["sa"] + 6), y(YS) + 44, "2x 6700ZZ", "s", "middle")

rect(X["w1"], X["w2"], -48, 168, "wall")
txt(x(X["w1"]) + 3, y(45), "Wannenseite", "s", "middle")
add('<rect class="hous" x="%.1f" y="%.1f" width="%.1f" height="%.1f"/>'
    % (x(X["w2"] + 1), y(YA + 11), (X["sa"] - X["w2"] - 2) * SC, (11 + A_E + DE47 / 2 + 3) * SC))
txt(x(X["sb"]) + 4, y(YA) - 16, "Seitenvorgelege 13:47,", "s")
txt(x(X["sb"]) + 4, y(YA) - 7, "aussen am Bug", "s")

rect(X["fa"], X["fb"], YA, DE13, "gear"); teeth(X["fa"], X["fb"], YA, DE13, 5)
rect(X["fa"], X["fb"], YS, DE47, "gear"); teeth(X["fa"], X["fb"], YS, DE47)
txt(x((X["fa"] + X["fb"]) / 2), y(YA + 11) - 3, "13 Z", "b", "middle")
txt(x((X["fa"] + X["fb"]) / 2), y(YS - DE47 / 2) + 10, "47 Z", "b", "middle")
rect(X["sa"], X["sb"], YS, DSPR, "gear"); teeth(X["sa"], X["sb"], YS, DSPR, 6)
txt(x(X["sb"]) + 18, y(YS) - 2, "Triebrad 15 Z", "b")
txt(x(X["sb"]) + 18, y(YS) + 7, "Teilkreis 57,72", "s")

for y0, y1, lab, xx, an in ((YE, YA, "45", 80, "end"), (YA, YS, "30", 188, "start")):
    add('<line class="dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (x(xx), y(y0), x(xx), y(y1)))
    for yy in (y0, y1):
        add('<line class="dim" x1="%.1f" y1="%.1f" x2="%.1f" y2="%.1f"/>' % (x(xx) - 3.5, y(yy), x(xx) + 3.5, y(yy)))
    txt(x(xx) + (-5 if an == "end" else 4), (y(y0) + y(y1)) / 2 + 2, lab, "s", an)

# ---------------- Panel 2 ----------------
PY0 = 400
add('<line class="box" x1="22" y1="%d" x2="%d" y2="%d"/>' % (PY0 - 18, W - 22, PY0 - 18))
txt(22, PY0 - 6, "Schaltzustaende", "h2")
txt(110, PY0 - 6, "(Mini-Schema: links Motorseite, rechts Wannenseite)", "s")
GAENGE = []
for name, z1, z2, aktiv in (("1. Gang (niedrig) - Kupplung zur Motorseite", 13, 47, 0),
                            ("2. Gang (hoch) - Kupplung zur Wannenseite", 28, 32, 1)):
    i_g = z2 / z1
    i_tot = i_g * I_SV
    v = N_MOT / i_tot * TRAVEL_PER_REV / 60
    GAENGE.append((name, "%d:%d = %s" % (z1, z2, de(i_g, 3)), de(i_tot, 2),
                   "%s m/s = %s km/h" % (de(v, 2), de(v * 3.6, 1)),
                   "%s Nm" % de(M_MOT * i_tot * ETA, 3), aktiv))
for k, (name, i_g, tot, v, Mspr, aktiv) in enumerate(GAENGE):
    bx = 30 + k * 320
    by = PY0 + 6
    add('<rect class="box" x="%d" y="%d" width="300" height="80" rx="3"/>' % (bx, by))
    txt(bx + 8, by + 14, name, "b")
    # Mini-Schema
    sx, sy = bx + 10, by + 46
    add('<line class="axis" x1="%d" y1="%d" x2="%d" y2="%d"/>' % (sx, sy - 16, sx + 120, sy - 16))
    add('<line class="axis" x1="%d" y1="%d" x2="%d" y2="%d"/>' % (sx, sy + 14, sx + 120, sy + 14))
    for j, (gx, gw, lab) in enumerate(((14, 12, "13:47"), (76, 12, "28:32"))):
        h1, h2 = (7, 28) if j == 0 else (14, 20)
        add('<rect class="gear" x="%d" y="%d" width="%d" height="%d"/>' % (sx + gx, sy - 16 - h1 / 2, gw, h1))
        add('<rect class="gearf" x="%d" y="%d" width="%d" height="%d"/>' % (sx + gx, sy + 14 - h2 / 2, gw, h2))
        txt(sx + gx + gw / 2, sy + 36, lab, "s", "middle")
    mx = sx + (60 if aktiv else 26)
    add('<rect class="kupplung" x="%d" y="%d" width="16" height="12"/>' % (mx, sy + 8))
    add('<path class="flow" d="M %d %d L %d %d L %d %d"/>'
        % (sx + (82 if aktiv else 20), sy - 16, sx + (82 if aktiv else 20), sy + 14, mx + 8, sy + 14))
    txt(bx + 150, by + 32, "Getriebe " + i_g, "l")
    txt(bx + 150, by + 44, "gesamt mit Seitenvorgelege " + tot, "l")
    txt(bx + 150, by + 56, "v leer " + v, "l")
    txt(bx + 150, by + 68, "Blockiermoment am Triebrad " + Mspr, "l")

txt(22, H - 14, "Rot: Kraftfluss. Beide Zahnradpaare sind immer im Eingriff, die Reibkupplung waehlt aus, "
                "Mittelstellung = Neutral. Rechnerisch bei %s V: Motor %s 1/min, Stall %s Nm (Herstellertabelle), "
                "Wirkungsgrad %s." % (de(U_BAT, 1), de(N_MOT, 0), de(M_MOT, 3), de(ETA, 2)), "s")
open(OUT, "w").write("\n".join(o) + "\n</svg>\n")
print("geschrieben:", os.path.normpath(OUT))
