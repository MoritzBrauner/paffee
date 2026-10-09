# Maßstab und Fahrzeugauswahl

> **Stand:** Gebaut wird in **1:10**, nicht in 1:11. Die Wanne existiert im CAD mit den Maßen des
> M24 Chaffee in 1:10, das Laufwerk ist weitgehend vom Chaffee übernommen. Die Analyse unten ist
> die ursprüngliche Herleitung, die auf 1:11 kam; was der Wechsel für die Kette bedeutet, steht
> im Abschnitt [Stand 1:10](#stand-110).

## Methode

Der Maßstab ist bei einer Kette **doppelt bestimmt**: über die Kettenbreite und über die Teilung.
Beide müssen denselben Maßstab ergeben, sonst sehen die Glieder zu lang oder zu kurz aus.

```
Maßstab aus Breite  = Kettenbreite_real / 38,4 mm
Maßstab aus Teilung = Teilung_real     / 12,0 mm
```

Damit beides zusammenfällt, muss das Vorbild ein Verhältnis Breite : Teilung von etwa **3,20 : 1**
haben. Das ist die Proportion **moderner Kampfpanzerketten**. WWII-Ketten sind deutlich
„breiter als lang" — deshalb scheitern Tiger und Panther an diesem Kettenglied.

## Kandidaten

| Vorbild | Breite | Teilung | B/T | aus Breite | aus Teilung | Bewertung |
|---|---|---|---|---|---|---|
| Leopard 2 (Diehl 570F) | 635 | 183 | 3,47 | 1:16,5 | 1:15,3 | **1:16** — Breite −3,3 %, Teilung +4,9 % |
| Pz III/IV, StuG III, Jagdpz IV (Kgs 61/400/120) | 400 | 120 | 3,33 | 1:10,4 | 1:10,0 | **1:10** — Breite −4 %, Teilung ±0 % |
| **M24 Chaffee, T72-Kette** | **406** | **139,7** | **2,91** | 1:10,6 | 1:11,6 | **1:11** — Breite +4,0 %, Teilung −5,5 % |
| M24, T85E1 mit extended end connectors | 419 | 139,7 | 3,00 | 1:10,9 | 1:11,6 | 1:11 — Breite **+0,8 %** |
| Leopard 1 | 550 | 160 | 3,44 | 1:14,3 | 1:13,3 | 1:14, unrunder Maßstab |
| M1 Abrams (T158) | 635 | ≈172 ¹ | ≈3,7 | 1:16,5 | 1:14,3 | 1:16 möglich, Glieder ~10 % zu lang |
| T-34 | 500 | ≈172 ¹ | 2,91 | 1:13,0 | 1:14,3 | 1:13,5 brauchbar |
| M4 Sherman (VVSS) | 421 | 152 | 2,76 | 1:11,0 | 1:12,7 | mäßig, bei 1:12 Kette ~9 % zu breit |
| T-72 | 580 | ≈137 ¹ | ≈4,2 | 1:15,1 | 1:11,4 | schlecht |
| Panther (Kgs 64/660/150) | 660 | 150 | 4,40 | 1:17,2 | 1:12,5 | **geht nicht** |
| Tiger I (Kgs 63/725/130) | 725 | 130 | 5,58 | 1:18,9 | 1:10,8 | **geht nicht**, Glieder fast doppelt zu lang |
| Tiger II (Kgs 73/800/152) | 800 | 152 | 5,26 | 1:20,8 | 1:12,7 | geht nicht |

¹ Teilung nicht belastbar belegt, als Näherung behandeln. Die deutschen WWII-Werte sind sicher,
weil die Kgs-Bezeichnung Breite und Teilung direkt codiert (`Kgs Typ/Breite/Teilung`).

## Ursprüngliche Entscheidung: 1:11

**M24 Chaffee in 1:11.** Fehler unter 6 % in beiden Größen, das Modell wird angenehm groß
(Wanne 457 mm) und wegen der schmalen Kette innen geräumig (~195 mm zwischen den Ketten).

Nachteile, die damals bewusst in Kauf genommen wurden:

- 1:11 ist kein Sammlermaßstab, es gibt kein Zubehör und keine fertigen Maßzeichnungen von der
  Stange (anders als 1:16 beim Leopard 2).
- Das LEGO-Kettenrad 57519 hat 10 Zähne, der M24 braucht 13. → wird gedruckt, siehe
  [04](04-laufwerk-1zu11.md).

## Stand 1:10

Das Fahrzeug wird inzwischen in **1:10** gebaut (M24 als Vorlage im CAD, siehe
[07](07-wanne.md)). Die Kette bleibt das Glied 57518. Gegen die M24-Kette T72 gerechnet:

| | M24 real | Soll in 1:10 | LEGO 57518 | Abweichung |
|---|---|---|---|---|
| Kettenbreite | 406 mm | 40,6 mm | 38,4 mm | **−5,4 %** |
| Teilung | 139,7 mm | 13,97 mm | 12,0 mm | **−14,1 %** |

Die Kette ist also etwas zu schmal und die Glieder sind deutlich zu kurz — es braucht mehr Glieder
als das Vorbild (75 je Seite). Die Gliederzahl für 1:10 ist noch nicht gerechnet.

Das Triebrad wird über den Durchmesser angepasst, nicht über die Zähnezahl: Der M24 hat 13 Zähne
bei 139,7 mm Teilung, also einen Teilkreis von Ø 584 mm, in 1:10 Ø 58,4 mm. Mit 12 mm Teilung
kommt man mit **15 Zähnen** auf Ø 57,72 mm (siehe [06](06-antrieb-elektronik.md)).

## Verworfene Alternativen

- **Leopard 2 in 1:16** war rechnerisch die beste Kombination überhaupt (Kettenrad 57519 passt dort
  fast exakt: 621 mm gegen 650 mm real, Laufrolle 56908 mit Ø 43,2 mm trifft die 43,8 mm), fällt
  aber wegen der Vorbildwahl weg.
- **Doppelreihige Kette** (zwei 57518 nebeneinander, 76,8 mm breit, Verhältnis 6,4:1) wäre die
  einzige proportionsrichtige Route zu Tiger I/II und Panther, bei ~1:10. Sehr aufwendig.
