# Laufwerk in 1:11

> **Veraltet.** Gebaut wird in **1:10**, das Laufwerk ist im CAD weitgehend vom M24 Chaffee
> übernommen. Layout, Koordinaten, Gliederzahl (79), Triebrad (13 Z, Ø 50,14 mm) und die
> Bauschablone unten gelten für 1:11 und sind **nicht** mehr maßgeblich. Das Triebrad hat in 1:10
> 15 Zähne, Teilkreis Ø 57,72 mm (siehe [02](02-massstab-analyse.md)). Ebenfalls überholt ist der
> Abschnitt **Antrieb** am Ende (LEGO-PU-L-Motor, Technic-Kreuzachsen) — der aktuelle Antrieb steht
> in [06](06-antrieb-elektronik.md).
>
> Weiterhin gültig, weil sie nur an der Kette hängen:
> - Rechenmethode über die Gelenkachslinie und der Gurtansatz um vier Kreise (`laufwerk.py`)
> - wirksamer Radius = Radradius + 2,80 mm, Gelenkachslinie 5,60 mm über dem Boden
> - Laufrolle als Doppelrolle mit Führungsrippe, Laufringe bei ±5,0 mm, außen nichts über ±7,2 mm
> - Triebradzähne: Flanken vom LEGO-Rad 57519 abnehmen und auf die neue Zähnezahl umlegen
> - Stützrollen 1–2 mm unter die berechnete Linie setzen

Bauschablone: [assets/laufwerk_m24_1zu11.svg](assets/laufwerk_m24_1zu11.svg) — bei 100 % gedruckt
maßstäblich, Raster 10 mm.

Koordinaten: Nullpunkt = Mitte Laufrolle 1, x nach hinten, y über Bodenlinie. N = Noppe = 8 mm.

## Eingangsgrößen

**Belegt:** Aufstandslänge 2851 mm · 5 Laufrollenpaare, 3 Stützrollen, Triebrad vorn, Leitrad
hinten · Triebrad 13 Zähne · T72-Kette 406 mm / 139,7 mm / 75 Glieder je Seite · Wanne 5030 mm ·
Bodenfreiheit 460 mm · Kettengliedgeometrie aus [01](01-kettenglied-57518.md).

**Angenommen** (nicht belegt, bewusst gesetzt):

| Größe | Wert | Begründung |
|---|---|---|
| Laufrolle Ø | 647,7 mm (25,5″) → 58,9 mm | aus dem Rollenabstand von 712,75 mm zwingend < 713 mm |
| Triebradmitte | y = 72 mm (9 N), x = −56 mm (7 N) | ergibt vorderen Anlauf von −37,7°, plausibel |
| Leitradmitte | y = 64 mm (8 N) | macht das Obertrum praktisch waagerecht (−0,13°) |
| Leitrad Ø | = Laufrolle, 58,9 mm | US-Praxis |

Diese vier Werte gehen fast ausschließlich in die Leitrad-x-Position ein
(**~1,9 mm Umlauflänge je mm Leitradverschiebung**). Bessere Quelle wären TM 9-729 oder eine
1:35-Bauanleitung (Bronco, AFV Club, Tamiya) — dort abmessen und × 3,18 rechnen, dann Layout neu
lösen.

## Layout

| Bauteil | x [mm] | x [N] | y [mm] | y [N] | wirks. Radius | Umschlingung |
|---|---|---|---|---|---|---|
| Triebrad 13 Z | **−56,0** | −7,0 | **72,0** | 9,0 | 25,07 | 142,5° = 5,1 Zähne im Eingriff |
| Laufrolle 1 | 0 | 0 | 37,84 | — | 32,24 | 37,7° |
| Laufrolle 2 | 64 | 8 | 37,84 | — | 32,24 | 0° |
| Laufrolle 3 | 128 | 16 | 37,84 | — | 32,24 | 0° |
| Laufrolle 4 | 192 | 24 | 37,84 | — | 32,24 | 0° |
| Laufrolle 5 | 256 | 32 | 37,84 | — | 32,24 | 22,7° |
| **Leitrad** | **318,4** | 39,8 | **64,0** | 8,0 | 32,24 | 157,1° |
| Stützrolle 1–3, Ø 20 | 40 / 128 / 216 | 5 / 16 / 27 | 83,9 | — | — | — |

„Wirksamer Radius" = Radius, auf dem die **Gelenkachslinie** läuft: bei Rädern Radrradius + 2,80 mm,
beim Triebrad der Teilkreisradius.

Rollenabstand **64 mm = 8 N** statt exakt 64,8 mm → Aufstandslänge 256 statt 259,2 mm (real 2816
statt 2851, −1,2 %). Dafür liegt das Laufwerk im Noppenraster.

Untertrum: die Gelenkachslinie liegt **5,60 mm** über dem Boden.

Laufwerkslänge Triebradkante bis Leitradkante = 432,9 mm, Wanne 457 mm — bleibt also Platz für
Bugüberhang und Heckplatte.

## Kettenumlauf

| Abschnitt | Länge |
|---|---|
| Gerade Triebrad → Laufrolle 1, −37,7° | 65,2 mm |
| Untertrum Laufrolle 1 → 5 | 256,0 mm |
| Gerade Laufrolle 5 → Leitrad, +22,7° | 67,6 mm |
| Obertrum Leitrad → Triebrad, −0,13° | 374,4 mm |
| Bögen Triebrad / R1 / R5 / Leitrad | 184,7 mm |
| **Summe** | **948,0 mm = 79 × 12 mm** |

**Kontrolle gegen das Original:** 75 Glieder × 139,7 mm = 10 478 mm, bei 1:11 also 952,5 mm. Die
konstruierte Geometrie liefert 948,0 mm — **0,5 % Abweichung**, und das Leitrad landet dabei auf
einer plausiblen Position (53 mm hinter Laufrolle 5, real 583 mm). Die angenommenen Höhen sind also
stimmig.

**Gliederbedarf: 79 je Seite, 158 gesamt — mit Reserve 170 bestellen.**

## Spannung

6,22 mm Leitradweg entsprechen genau einem Glied. Ein Spanner mit **±4 mm** Weg deckt damit ein
ganzes Glied ab.

| Gliederzahl je Seite | Leitradmitte x |
|---|---|
| 78 | 312,1 mm |
| **79 (Soll)** | **318,4 mm** |
| 80 | 324,6 mm |

Stützrollen **1–2 mm unter** die berechnete Linie setzen (Mitte y = 82…83 statt 83,9), damit die
Kette wirklich aufliegt. Bei 374 mm freiem Obertrum sind sie tragend, nicht dekorativ.

## Radaufhängung

Rollenmitte y = 37,84 mm liegt nicht im 8-mm-Raster. Lösung wie beim Original: Schwingarm.

- Drehpunkt y = **24 mm** (3 N), im Raster
- Armlänge **40 mm** (5 N)
- Armwinkel 20,2° → Rollenmitte y = 37,84 mm
- Nebeneffekt: echte Einzelradfederung, wie die Drehstabfederung des M24

## Druckteile

| Teil | Spezifikation |
|---|---|
| **Triebrad** | 13 Zähne, Teilkreis **Ø 50,14 mm**, Zahnwinkel 27,692°, Sehne genau 12,0 mm. Zahnflanken vom LEGO-Rad 57519 abnehmen und auf 13 Zähne umlegen (erprobter Eingriff). Zwei Seitenflansche im Abstand ~15 mm halten das Mittelband. Nabe für zwei Technic-Kreuzachsen. PETG oder ABS, Zähne massiv. |
| **Laufrolle** | Ø 58,9 mm, Doppelrolle: zwei Laufringe je ~4,2 mm breit, Mitten bei ±5,0 mm, dazwischen Führungsrippe ≤ 5 mm breit / ~2 mm hoch (läuft in der 4,8 mm tiefen Mittelnut der Kette). Außen darf nichts über ±7,2 mm hinaus tragen. |
| **Leitrad** | gleiche Kontur, Ø 58,9 mm, auf verstellbarem Arm, Nominalposition x = 318,4 mm |
| **Stützrolle** | Ø 20 mm, gleiche Doppelband-Kontur, Breite ≤ 14 mm |

## Antrieb

Eine Triebradumdrehung = 13 × 12 = **156 mm Fahrweg**.

Ein PU-L-Motor (~380 min⁻¹ Leerlauf) direkt ergäbe 3,6 km/h am Modell ≈ 39 km/h maßstäblich (echt:
56 km/h), aber ohne Drehmomentreserve. **3:1 bis 5:1** untersetzen → 0,7–1,2 km/h am Modell mit
genug Kraft für die Kettenmasse. Die 158 Glieder sind grob 0,3–0,6 kg (aus dem Volumen geschätzt,
nicht belegt).
