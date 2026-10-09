# RC-Panzer: Film-Panther auf M24-Chaffee-Fahrwerk (1:10)

Selbstbau eines ferngesteuerten Panzers. **Alles wird selbst gedruckt** — Rahmen, Wanne, Turm,
Laufwerksräder, Aufbau. Zugekauft werden vor allem die Kette — das LEGO-Kettenglied **57518** —
sowie Motoren, Elektronik, Lager und die BB-Einheit.

Vorbild ist nicht ein echter Panther, sondern eine **Film-Attrappe**: in „Paris brûle-t-il?" /
„Is Paris Burning?" (1966) spielten optisch umgebaute M24 Chaffee die deutschen Panzer, dazu gab es
einen Panther-Nachbau. Der M24 trug schon im Dienst den Spitznamen *„Panther Pup"*, weil seine
Silhouette auf Distanz mit dem Panther verwechselt wurde. Daher auch der Projektname:
**Pa**nther + Cha**ffee**.

Diese Doku stammt aus dem früheren Projektordner `rc-tank-panther-m24` (Stand August 2026) und
wurde beim Umzug in dieses Repo auf den aktuellen Stand gebracht. Wo Doku und Firmware sich
widersprachen, gilt die Firmware.

## Maßstab: 1:10

Die Kette hat ursprünglich **1:11** nahegelegt (Analyse in
[02-massstab-analyse.md](02-massstab-analyse.md)). Gebaut wird inzwischen in **1:10**: Die Wanne
existiert im CAD mit den Maßen des M24 Chaffee in 1:10, das Laufwerk ist mehr oder weniger vom
Chaffee übernommen. Die Form der Wanne ist „pantherfiziert" — Panther-Form mit den
Seitenverhältnissen und Maßen des Chaffee.

Alles, was in dieser Doku noch in 1:11 gerechnet ist (Laufwerkslayout, Gliederzahl,
Bauschablone), ist **veraltet** und entsprechend markiert. Maßgeblich ist das CAD.

## Kerndaten

| | |
|---|---|
| Maßstab | **1:10** |
| Kette | LEGO 57518, 38,4 mm breit, 12,0 mm Teilung |
| Gliederbedarf | für 1:10 noch nicht gerechnet (die 1:11-Rechnung ergab 79 je Seite) |
| Laufwerk | wie M24: 5 Laufrollenpaare, 3 Stützrollen, Triebrad vorn, Leitrad hinten |
| Triebrad | 15 Zähne, Teilkreis Ø 57,72 mm (M24: 13 Z, Ø 584 mm → 58,4 mm in 1:10) |
| Aufstandslänge | ~285 mm (M24 2851 mm) |
| Wanne | ~503 mm lang, ~300 mm breit (M24-Maße in 1:10), Form pantherfiziert, bisher nur im CAD |
| Gesamtlänge mit Rohr | ~680 mm bei Panther-Rohrüberhang (179 mm), siehe [07](07-wanne.md) |
| Gewicht | Schätzung 5,3 kg, ausgelegt auf 7,0 kg (aus der 1:11-Auslegung) |

## Technik

| | |
|---|---|
| Akku | 3S LiPo 11,1 V, 1500 mAh (vorhandene Airsoft-Packs), Wechselschacht im Motordeck; rechnerisch ~24 min Fahrt im hohen Gang je Pack |
| Fahrantrieb | 2× JGB37-520, 12 V, 6,25:1, 1600 min⁻¹, Blockiermoment ~0,09 Nm (Herstellertabelle), Hall-Encoder; Motortreiber Cytron MDD10A, angesteuert per PWM + Richtungspin |
| Getriebe | je Seite Zweiganggetriebe **28:32** und **13:47** mit **Reibkupplungen** (mit Schleifpapier beklebt), danach Seitenvorgelege **13:47** |
| Schaltung | Getriebe links/rechts gespiegelt, **ein Servo** (Futaba S3003) dazwischen schaltet beide gleichzeitig über ein Gestänge |
| Fahrleistung | rechnerisch bei 7 kg in der Ebene: hoher Gang ~1,8 km/h (knapp über der Dauergrenze der Motoren), niedriger Gang ~1,0 km/h. Steigungen und Wenden auf der Stelle nur im niedrigen Gang — der Antrieb ist leistungsbegrenzt (~3 W je Motor) |
| Rechner | ESP32 (DevKit, `esp32dev`), Arduino-Framework; Turm-Slave ESP32-C3 über CAN geplant |
| Funk | nRF24L01+PA+LNA, eigenes Protokoll (WLAN im Fahrbetrieb aus) |
| Turm | 360° über Hohlwellen-Schleifring, NEMA-17 + TMC2209, Zahnkranz 100 Z, AS5600 |
| Rohr | Höhenrichtung −8°/+20°, 6-mm-BB-Schussfunktion |

Details und Rechenwege: [06-antrieb-elektronik.md](06-antrieb-elektronik.md). Die Firmware zur
Schaltautomatik liegt in [lib/shift_engine/](../lib/shift_engine/).

## Inhalt

| Datei | Inhalt |
|---|---|
| [01-kettenglied-57518.md](01-kettenglied-57518.md) | Vermessung des LEGO-Kettenglieds 57518 aus der LDraw-Geometrie: Breite, Teilung, Innenkontur |
| [02-massstab-analyse.md](02-massstab-analyse.md) | Welches Fahrzeug passt in welchem Maßstab zur Kette — Kandidatenvergleich, Wechsel auf 1:10 |
| [03-vorbild-m24-panther.md](03-vorbild-m24-panther.md) | M24-Vorbilddaten, Größenvergleich zum Panther, Filmhintergrund |
| [04-laufwerk-1zu11.md](04-laufwerk-1zu11.md) | Laufwerk in 1:11 durchgerechnet — **veraltet**, was davon noch gilt, steht oben in der Datei |
| [05-quellen.md](05-quellen.md) | Quellen, getrennt nach belegt und angenommen |
| [06-antrieb-elektronik.md](06-antrieb-elektronik.md) | Gewicht, Zugkraftbedarf, Motoren, Akku, Getriebe und Schaltung, Turm, Funk und Failsafe |
| [07-wanne.md](07-wanne.md) | Wanne und Turm: Stand CAD, Panther-Winkel als Referenz |

| Asset | Inhalt |
|---|---|
| [assets/getriebe_konzept.svg](assets/getriebe_konzept.svg) | Konzeptblatt Antriebsstrang einer Fahrzeugseite, 1:10 |
| [assets/laufwerk_m24_1zu11.svg](assets/laufwerk_m24_1zu11.svg) | Bauschablone Laufwerk — **veraltet (1:11)** |
| [assets/panther_maße_original.jpg](assets/panther_maße_original.jpg) | Maßzeichnung Panther (russisch beschriftet): Seite, Draufsicht, Front, Heck mit Hauptmaßen, Plattenstärken und Winkeln |
| [assets/panther_maße_formular.pdf](assets/panther_maße_formular.pdf) | dieselbe Zeichnung mit ausgeblendeten Maßen, zum Eintragen eigener Werte |

## Konventionen

- Alle Maße in **mm**. „N" = LEGO-Noppe = 8 mm.
- Koordinatensystem im Laufwerk: Nullpunkt = **Mitte Laufrolle 1**, x nach hinten positiv,
  y nach oben ab **Bodenlinie**.
- „Gelenkachslinie" = die Linie durch die Kettengelenke. Sie ist die Bezugslinie für alle
  Umlaufrechnungen, nicht die Boden- oder Laufflächenkontur.

## Zahlen reproduzieren

```bash
python3 doku/scripts/kettenglied_analyse.py   # lädt 57518 aus der LDraw-Bibliothek und vermisst es
python3 doku/scripts/getriebe_svg.py          # erzeugt assets/getriebe_konzept.svg (1:10)
python3 doku/scripts/laufwerk.py              # Layout + Kettenumlauf — Stand 1:11, veraltet
python3 doku/scripts/laufwerk_svg.py          # erzeugt assets/laufwerk_m24_1zu11.svg — Stand 1:11, veraltet
python3 doku/scripts/antrieb.py               # Gewicht, Zugkraft, Fahrleistung, Strombudget, Laufzeit (1:10)
```

## Status

- [x] Kettenteil ausgemessen und Maßstab bestimmt (zunächst 1:11, jetzt 1:10)
- [x] Vorbild festgelegt
- [x] Laufwerk durchgerechnet, Bauschablone erzeugt (Stand 1:11, veraltet)
- [ ] Laufwerk in 1:10 nachrechnen (Gliederzahl), Kettenglieder 57518 beschaffen
- [x] Antrieb und Elektronik ausgelegt
- [x] Wanne im CAD (Chaffee-Maße 1:10, Form pantherfiziert)
- [ ] Draufsicht und Getriebe-Querschnitt
- [ ] Triebrad, Laufrolle, Leitrad, Stützrolle konstruieren und drucken
- [ ] Chassis-Längsträger, Drehstabfederung, Getriebegehäuse
- [ ] Getriebe bauen — in Arbeit: eins von zwei existiert und ist etwa zur Hälfte fertig, der
      Schaltservo hängt daran
- [ ] Antriebsstrang und Elektronik aufbauen, Schaltautomatik und Failsafe implementieren
      (Firmware begonnen, siehe `lib/shift_engine`)
- [ ] Wanne und Panther-Aufbau drucken
- [ ] Turm mit Schleifring, Höhenrichtung, BB-Einheit
