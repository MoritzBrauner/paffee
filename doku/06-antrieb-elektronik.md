# Antrieb und Elektronik

Gewicht, Zugkraftbedarf, Fahrleistung, Strombudget und Laufzeiten erzeugt
[scripts/antrieb.py](scripts/antrieb.py) (Stand 1:10, JGB37-520 am 3S-Akku). Bezugsgrößen in 1:10: Triebrad 15 Z mit Teilkreis Ø 57,72 mm (wirksamer
Radius 28,86 mm), Fahrweg je Triebradumdrehung 15 × 12 = **180 mm**, Aufstandslänge ~285 mm,
Kettenspurweite ~262 mm (300 mm Breite minus eine Kettenbreite).

## Gewicht

| Position | kg |
|---|---|
| Wanne + Rahmen gedruckt (PETG/ASA, ~2,5 mm Wand) | 1,30 |
| Turm + Blende + Rohr | 0,55 |
| 10 Laufräder + Trieb-/Leiträder + Stützrollen | 0,40 |
| Kette, 158 Glieder (1:11-Wert, für 1:10 offen) | 0,45 |
| Schürzen, Panels, Details | 0,40 |
| 2× Fahrmotor mit Getriebe | 0,60 |
| Turmantrieb + Höhenrichtung | 0,20 |
| BB-Einheit | 0,15 |
| Akku (Reserve für die Ausbaustufe 3S Li-Ion; ein LiPo 1500 mAh wiegt ~0,12 kg) | 0,55 |
| Elektronik, Kabel, Lautsprecher | 0,25 |
| Wellen, Lager, Schrauben, Federstahl | 0,45 |
| **Erwartet** | **5,30** |

Die Schätzung stammt aus der 1:11-Auslegung; in 1:10 werden die gedruckten Teile eher schwerer.

**Auslegung auf 7,0 kg.** Bei gedruckten Wannen wächst die Wandstärke im Bauen fast immer, und
Zusatzmasse tief unten schadet einem Kettenfahrzeug nicht — sie bringt Traktion.

## Bedarf am Triebrad (je Seite, 7,0 kg)

Rollwiderstandsbeiwert 0,15, Querreibung beim Wenden 0,6. Die Zugkräfte hängen nur an Masse und
Beiwerten (beim Wenden am Verhältnis Aufstandslänge/Spurweite, das in 1:10 und 1:11 praktisch
gleich ist). Die Drehmomente sind mit dem 1:10-Triebrad (r = 28,86 mm) gerechnet.

| Fall | Zugkraft | Drehmoment |
|---|---|---|
| Ebene rollen | 5,2 N | 0,149 Nm |
| Steigung 30 % | 14,8 N | 0,427 Nm |
| Steigung 100 % (45°) | 27,9 N | 0,806 Nm |
| Wenden auf der Stelle | 16,4 N | 0,472 Nm |

Leistung ist hier der Engpass: Der JGB37-520 gibt am 3S-Akku höchstens ~3,1 W mechanisch ab
(siehe Fahrleistung). Steigungen und Wenden auf der Stelle gehen deshalb nur im niedrigen Gang.

## Architektur

| Baugruppe | Wahl | Begründung |
|---|---|---|
| **Akku** | **3S LiPo 11,1 V, 1500 mAh (vorhandene Airsoft-Packs)**, XT60-Adapter, 20-A-Hauptsicherung (träge), Hauptschalter; bei Parallelbetrieb zusätzlich je Pack 10 A. Ausbaustufe: 3S Li-Ion 21700 | 11,1 V passt direkt zu 12-V-Getriebemotoren, kein Wandler im Antriebsstrang. Details und Abschaltschwellen im Abschnitt Akku. |
| **Fahrmotoren** | 2× JGB37-520, 12 V, 1600 min⁻¹, Untersetzung 6,25:1 (520er-Motor, 10 000 min⁻¹), Hall-Encoder (11 Impulse je Motorumdrehung laut Micro DC Motors) | Siehe [Motordaten](#motordaten-jgb37-520). Encoder für Geradeauslauf (geplant), Gangwechsel und Kalibrierung. In der Firmware steht `countsPerRev = 275` = 11 × 4 × 6,25 — passt zur Untersetzung, am eigenen Motor noch zu bestätigen ([src/test_encoder](../src/test_encoder/main.cpp)). |
| **Motortreiber** | Cytron MDD10A, 2 Kanäle, 10 A dauernd, 5–30 V | Angesteuert per PWM + Richtungspin (Sign-Magnitude), so in der Firmware vorgesehen (`write()` in ShiftEnginge.h, geplant 20 kHz); das PWM-Setup ist noch offen. |
| **Strommessung** | INA226 je Motor (optional, derzeit nicht eingeplant) | Für Telemetrie und die Lastkompensation der Akkuspannung. Die Schaltautomatik läuft nicht mehr über den Strom, sondern über die Drehzahl. Im Code noch nicht vorhanden. |
| **Schaltservo** | Futaba S3003, analog, 50 Hz | Mitte 1500 µs = Neutral, 1. Gang 800 µs, 2. Gang 2200 µs (±700 µs, ShiftEnginge.h). Der Servotest ([src/test_servo](../src/test_servo/main.cpp)) fährt derzeit ±670 µs — Endlagen am eingebauten Getriebe ausmessen und beide Werte angleichen. |
| **Drehzahlsensorik** | Je Seite Motor-Encoder (Getriebeeingang) + Lichtschranke mit 32-Schlitz-Scheibe (Getriebeausgang) | Aus dem Verhältnis beider prüft die Firmware den Drehzahlabgleich beim Schalten. Die Lichtschranke erkennt keine Drehrichtung. |
| **Rechner** | ESP32 (DevKit, `board = esp32dev`), Arduino-Framework | CAN (TWAI) für den Turm-Slave, genug Timer, eigene Logik für Mischer, Schaltautomatik, Turm, Sound. |
| **Funk** | **nRF24L01+PA+LNA**, vorhandenes eigenes Protokoll | siehe eigenen Abschnitt unten |
| **Turm-Slave** | ESP32-C3 im Turm, per CAN | halbiert die nötigen Schleifringkreise |
| **Spannungsebenen** | 6 V/5 A UBEC für Servos, 5 V/3 A für Logik, getrennte Zuleitungen ab Akku | Servo-Stromspitzen dürfen nie über die Logikschiene laufen. Motor-Encoder an **3,3 V**, nicht 5 V — die ESP32-GPIOs sind nicht 5-V-fest. |

### Motordaten JGB37-520

Ein offizielles Datenblatt gibt es nicht. Die Werte stammen aus der Herstellertabelle
NFP-JGB37-520-EN-12100 (12 V, 10 000 min⁻¹ am Motor), Zeile 6,25:1, bestätigt bei
[Precision Mini Drives](https://precisionminidrives.com/product/37mm-gear-motor-with-encoder-41mm-type-model-nfp-jgb37-520-en)
und [rcdrone](https://rcdrone.top/products/jgb37-520-dc-gear-motor) (dort als „nur Referenz"
markiert). Nicht am eigenen Motor gemessen.

| | bei 12 V (Tabelle) | bei 11,1 V (gerechnet) |
|---|---|---|
| Leerlaufdrehzahl am Motorabtrieb | 1600 min⁻¹ (Toleranz ±15 %) | ~1470 min⁻¹ |
| Leerlaufstrom | 0,15 A | 0,15 A |
| Bestpunkt (max. Wirkungsgrad) | 1200 min⁻¹, 0,023 Nm (0,23 kg·cm), 0,65 A | — |
| **Blockiermoment** | **0,088 Nm (0,9 kg·cm)** | **0,081 Nm** |
| Blockierstrom | 2,4 A | 2,2 A |
| max. mechanische Leistung | 3,7 W (gerechnet; die Tabelle nennt 7,8 W elektrische Aufnahme am Bestpunkt) | 3,1 W |

Umgerechnet mit dem linearen Gleichstrommotor-Modell (Wicklungswiderstand 12 V / 2,4 A = 5 Ω).

Einordnung:
- Der frühere Platzhalter von 0,29 Nm war gut dreimal zu hoch.
- Die Tabelle ist schematisch gerechnet und eher konservativ: Sie unterstellt dem eingebauten
  Motorgetriebe im Stillstand nur ~58 % Wirkungsgrad. Andere Tabellen nennen bis 0,12 Nm, die
  physikalische Obergrenze liegt bei ~0,15 Nm. Gerechnet wird mit 0,088 Nm.
- „JGB37-520" bauen mehrere Fabriken; Blockierstrom je nach Tabelle 2,3–3,5 A. Für Treiber und
  Sicherung **3,5 A Spitze je Motor** ansetzen.
- Der Hersteller untersagt Blockieren als Dauerzustand. Dauerbetrieb bis höchstens ~50 % des
  Blockiermoments.
- Ohne Drehmomentmessung am eigenen Motor prüfbar: Leerlaufdrehzahl (Encoder), Leerlauf- und
  Blockierstrom (Multimeter, nur kurz blockieren), Untersetzung (10 Abtriebsumdrehungen von Hand
  = 687,5 Impulse je Kanal bei 6,25:1, im Testprogramm wegen Vollquadratur 2750 Zählschritte).

### Fahrleistung

Gesamtübersetzung = Gang × Seitenvorgelege 13:47, Wirkungsgrad Getriebe + Seitenvorgelege 0,85
(Annahme), Akku 11,1 V.

| Gang | Getriebe | gesamt | v leer | Blockiermoment am Triebrad |
|---|---|---|---|---|
| **hoch** (2. Gang) | 28:32 = 1,143 | 4,13:1 | 1,07 m/s = 3,8 km/h | 0,285 Nm |
| **niedrig** (1. Gang) | 13:47 = 3,615 | 13,07:1 | 0,34 m/s = 1,2 km/h | 0,903 Nm |

Lastfälle bei 7,0 kg (Anteil am Blockiermoment, Geschwindigkeit, Strom je Motor):

| Fall | hoher Gang | niedriger Gang |
|---|---|---|
| Ebene rollen | 52 %, 1,8 km/h, 1,23 A — knapp über der Dauergrenze, nur kurzzeitig | 16 %, 1,0 km/h, 0,49 A |
| Steigung 30 % | **geht nicht** (150 %) | 47 %, 0,6 km/h, 1,13 A |
| Steigung 100 % (45°) | **geht nicht** (282 %) | 89 %, 0,1 km/h, 2,00 A — nur kurzzeitig |
| Wenden auf der Stelle | **geht nicht** (166 %) | 52 %, 0,6 km/h, 1,23 A — knapp über der Dauergrenze, nur kurzzeitig |

Mit der erwarteten Masse von 5,3 kg wird es etwas besser (hoher Gang in der Ebene 39 %, 2,3 km/h),
am Bild ändert sich nichts: Wenden auf der Stelle geht auch dann im hohen Gang nicht (125 %).

Was das heißt:
- **Der hohe Gang taugt nur für die Ebene.** Steigungen und Wenden auf der Stelle brauchen den
  niedrigen Gang. Da ein Servo beide Seiten schaltet, muss die Schaltlogik vor dem Wenden auf der
  Stelle herunterschalten.
- **Der Antrieb ist leistungsbegrenzt**, nicht drehzahlbegrenzt: In der Ebene fährt der hohe Gang
  mit 7 kg rechnerisch ~1,8 km/h (maßstäblich 18 km/h, das echte M24 fuhr 56 km/h), nicht die
  3,8 km/h Leerlaufgeschwindigkeit.
- Die Rechnung hängt stark an zwei Annahmen: Rollwiderstandsbeiwert 0,15 und Blockiermoment
  0,088 Nm. Ist der Motor stärker als die Tabelle (bis ~0,12 Nm) oder rollt das Laufwerk leichter,
  wird es entsprechend besser.

### Strombudget

| | A |
|---|---|
| 2× Fahrmotor (Auslegung 3,5 A Spitze je Motor) | 7,0 |
| Turmantrieb | 1,5 |
| BB-Einheit | 3,0 |
| Servos | 2,0 |
| Elektronik/Sound | 1,0 |
| **Spitze** | **14,5 A = 161 W bei 11,1 V** |

Fahrbetrieb mit 0,5 A Grundlast (Elektronik, Funk, Servo in Ruhe, Annahme): Ebene im hohen Gang
~3,0 A gesamt, Ebene im niedrigen Gang ~1,5 A, Steigung 30 % im niedrigen Gang ~2,8 A.

## Akku

Primär die vorhandenen **3S-LiPo-Packs aus dem Airsoft-Bestand, 11,1 V, 1500 mAh**. Strom ist
unkritisch: 1500 mAh liefern bei angenommenen 20C (Aufdruck der Packs prüfen) 30 A gegen 14,5 A
Spitzenlast. Der Engpass ist die Kapazität. Die Laufzeiten sind mit 2,96 A bzw. 1,48 A gerechnet
(antrieb.py), die Ebene im hohen Gang liegt dabei knapp über der Dauergrenze der Motoren.

| Konfiguration | Kapazität | nutzbar (80 %) | Ebene, hoher Gang (~3,0 A) | Ebene, niedriger Gang (~1,5 A) |
|---|---|---|---|---|
| **1× 1500 mAh** | 1,5 Ah | 1,2 Ah | **24 min** | 49 min |
| 2× 1500 mAh parallel | 3,0 Ah | 2,4 Ah | 49 min | 97 min |
| 2× 2200 mAh parallel | 4,4 Ah | 3,5 Ah | 71 min | 143 min |
| 3S2P Li-Ion 21700 (Ausbaustufe) | 8,4 Ah | 6,7 Ah | 136 min | 272 min |

**Abschaltschwellen sind akkutypabhängig** — als Konfigurationswert in der Firmware führen:

| | Leistung begrenzen | Abschalten |
|---|---|---|
| **LiPo 3S** | 10,5 V (3,5 V/Zelle) | 9,9 V (3,3 V/Zelle) |
| Li-Ion 3S | 9,0 V (3,0 V/Zelle) | 8,4 V (2,8 V/Zelle) |

Unter Last bricht die Spannung ein, bei alten Airsoft-Packs mit hohem Innenwiderstand deutlich.
Also entweder mit Laststrom kompensieren (U_leer ≈ U_gemessen + I × R_i, braucht die optionale
Strommessung) oder — ohne INA226 — nur in Gaspausen auswerten — sonst schaltet das Fahrzeug beim Anfahren am Hang ab, obwohl das Pack halb voll ist.

**Parallelbetrieb** nur mit identischen Packs (Kapazität, C-Rate, Modell, Alter), vor dem
Zusammenstecken auf gleiche Spannung bringen (< 0,05 V pro Zelle Differenz, sonst fließen
zweistellige Ausgleichsströme über Stecker, die dafür nicht gebaut sind), je Pack eine eigene
Sicherung, getrennt laden.

**Besser als parallel: Wechselschacht.** Ein Motordeckel wird zur Batterieklappe mit XT60 — rund 25 min
fahren, in zehn Sekunden wechseln. Gleiche Gesamtfahrzeit ohne Parallel-Risiko, und das Pack kommt
zum Laden aus dem Modell. Nie im Modell laden, nie unbeaufsichtigt, LiPo-Tasche.

Airsoft-Packs haben oft Mini-Tamiya — bei ~15 A Spitze knapp. Adapterkabel auf XT60 bauen und die
Packstecker unangetastet lassen, dann bleiben die Akkus für die Airsoft nutzbar.

Gewicht: zwei 1500er sind ~240 g statt der in der Gewichtstabelle eingeplanten 550 g. Fehlt
Traktion, kommt Ballast tief in den Wannenboden. Als Alternative zum Wechselschacht passen
Nunchuck-Packs gut: beide Hälften links und rechts neben den Getrieben auf den Wannenboden.

## Getriebe und Schaltung

Konzeptblatt: [assets/getriebe_konzept.svg](assets/getriebe_konzept.svg), erzeugt von
[scripts/getriebe_svg.py](scripts/getriebe_svg.py).

Kraftfluss je Seite: **Motor → Zweiganggetriebe → Seitenvorgelege → Triebrad.**

- Zweiganggetriebe: hoch **28:32** (1,143), niedrig **13:47** (3,615), Spreizung 3,16. MOD 1,5,
  beide Paare mit Zähnesumme 60, also gleicher Achsabstand 45 mm. In der Firmware ist der
  1. Gang der niedrige (13:47), der 2. Gang der hohe (28:32); die Konstanten sind dort als
  Drehzahlverhältnis Abtrieb/Antrieb hinterlegt (13/47 bzw. 28/32). Aus Platzgründen sitzt der
  2. Gang zur Wannenseite hin, der 1. Gang zur Motorseite.
- Danach das **Seitenvorgelege**, finale Stufe ebenfalls **13:47**. Gesamt 4,13:1 bzw. 13,07:1.
  Es sitzt außen am Bug, die Getriebewelle durchdringt dafür die Wannenseite. Lager und Wellen
  auf dem Konzeptblatt stammen aus dem früheren Endantrieb-Konzept und sind nicht bestätigt.
- Schaltelement sind **Reibkupplungen, mit Schleifpapier beklebt** für mehr Grip — keine Klauen.
  Mittelstellung = Neutral.
- Die beiden Getriebe sind **links/rechts gespiegelt**. Dazwischen sitzt **ein Servo**, der beide
  über ein Gestänge gleichzeitig schaltet — beide Seiten schalten zwangsläufig gleichzeitig,
  einfacher und funktional richtiger als zwei unabhängige Schaltungen.
- **Schaltautomatik** (Soll-Verhalten; Firmware in Arbeit,
  [lib/shift_engine](../lib/shift_engine/ShiftEnginge.h), noch in keinem Build eingebunden):
  Grundlage ist die Drehzahl am Getriebeausgang (Lichtschranke). Im 1. Gang wird bei über
  400 min⁻¹ hochgeschaltet, im 2. Gang bei 350 min⁻¹ und darunter heruntergeschaltet — nur wenn
  beide Seiten dasselbe melden, sonst (typisch in Kurven) bleibt der Gang.
  **Achtung:** Mit den Motordaten oben erreicht der Getriebeausgang im 1. Gang bei 11,1 V selbst im
  Leerlauf nur ~407 min⁻¹, in der Ebene mit 7 kg ~340 min⁻¹. Die Hochschaltschwelle von
  400 min⁻¹ wird damit praktisch nie erreicht und muss an die Motordaten angepasst werden.
- **Schaltablauf:** Servo auf Neutral → Motor im Leerlauf auf die Drehzahl des Zielgangs bringen
  (Abgleich über eine beim Start gemessene Kennlinie PWM → Motordrehzahl, Kontrolle über
  Encoder und Lichtschranke, Toleranz ±20 min⁻¹) → einkuppeln, sobald abgeglichen oder
  spätestens nach 2 s → zurück in den Fahrbetrieb.
- **Die tragenden Zahnräder nicht drucken.** Zahnräder aus Stahl oder POM von der Stange;
  gedruckt bleiben Gehäuse und Schaltmechanik. Am höchsten belastet ist das Seitenvorgelege: am
  Triebrad liegen im niedrigen Gang bis ~0,9 Nm an (Blockiermoment).

## Turm

Statt die Turmaktorik von unten zu versorgen, sitzt der **Rechner im Turm**. Über den Schleifring
gehen dann nur **Plus, Minus, CAN-H, CAN-L** plus zwei Reserveadern statt zehn Leitungen.

| Teil | Wahl |
|---|---|
| Schleifring | Hohlwellen-Kapselschleifring Ø ~30 mm, 6–8 Kreise, ≥ 2 A. Im Turmring (in 1:10 rund 150–165 mm) reichlich Platz. |
| Lager | Gedruckte 4-Punkt-Kugellaufbahn Ø ~120 mm mit 5-mm-Stahlkugeln, PTFE-Anlaufscheibe |
| Antrieb | Gedruckter Innenzahnkranz MOD 1,5, 100 Zähne (Ø 150 mm), Ritzel 15 Zähne → 6,67:1 |
| Motor | NEMA-17-Pancake + TMC2209: lautlos, hält die Position ohne Bremse, 0,017° je Mikroschritt am Turm. 20 min⁻¹ am Motor = 18°/s am Turm ≈ maßstäbliche Turmdrehgeschwindigkeit. |
| Rückmeldung | AS5600 auf der Turmachse → absolute Turmlage, ermöglicht „Turm auf 12 Uhr" und Stabilisierung per IMU |

**Höhenrichtung:** Panther-KwK-42-Bereich −8°/+20°. 25-kg·cm-HV-Servo (DS3225-Klasse) über kurzen
Hebel genügt für ein gedrucktes Rohr. Wenn das Servozittern unter Last stört: Schneckenantrieb an
der Wiege, selbsthemmend und dann ruhig.

## Schussfunktion

Fertige **1:16-BB-Einheit von Heng Long/Taigen** (Motor, Nocke, Feder, Kolben, Magazin ~50 BB,
Reichweite ~20 m). Für genau diesen Zweck gebaut und passt in einen 1:10-Turm mit viel Luft; ein
Airsoft-AEG-Getriebe wäre um Größenordnungen zu stark und zu groß. Ansteuerung über einen
MOSFET-Kanal am Turm-Slave, Nachladeerkennung per Endschalter an der Nocke.

Rahmenbedingungen: in Deutschland sind 6-mm-Softair-Geräte **bis 0,5 J** ab 18 frei; die
1:16-Einheiten liegen klar darunter, getunte Federn verlassen diesen Bereich. Unabhängig davon:
Augenschutz, nie auf Personen oder Tiere.

## Funkstrecke nRF24

Eigenes, bereits erprobtes Protokoll. Zu beachten:

| Punkt | Festlegung |
|---|---|
| **WLAN** | Im Fahrbetrieb **aus** (`esp_wifi_stop()`), nur im Pit-Modus bei stehendem Fahrzeug für Konfiguration und OTA. Der ESP32 sendet mit ~20 dBm im selben Band, der nRF24 hört bei −85 dBm — wenige Zentimeter daneben ist das Zudecken, kein Nebeneinander. |
| Modul | nRF24L01+PA+LNA mit SMA-Antenne, Chip mit echtem Nordic-Marking (Si24R1-Klone haben messbar schlechtere Empfindlichkeit) |
| Spannung | **Eigener 3,3-V-LDO ≥ 500 mA**, 100 µF Elko + 100 nF direkt an den Modulpins. PA/LNA zieht im Sendemoment > 100 mA; der LDO eines DevKits bricht dabei ein — das ist der klassische „Funk fällt nach 10 Minuten aus"-Fehler. |
| SPI | Eigener SPI-Host, nicht geteilt. IRQ-Pin verdrahten und nutzen, nicht pollen. |
| Datenrate | 250 kbps für Reichweite (~+3 dB Empfindlichkeit gegenüber 1 Mbps), 1 Mbps wenn Telemetrie per Ack-Payload mitläuft. Bei 2 Mbps mindestens 2 MHz Kanalabstand. |
| Kanäle | In die WLAN-Lücken: 2404, 2427, 2451, 2476 MHz |
| Telemetrie | Ack-Payload: Akkuspannung, Gang, Motorströme (nur mit Strommessung), Temperaturen reiten auf der Quittung mit |
| Antenne | Gedruckte Wanne ist HF-durchlässig, kein Antennenfenster nötig. Weit weg von Motorleitungen und DC-DC; Heck-Deck hinter einem Panther-Gitter ist ideal. Bei späteren Metallschürzen nach außen verlegen. |

### Failsafe (Fahrzeugseite)

Die Sicherheitssemantik hängt nicht am Protokoll, sondern am Fahrzeug. Bei 7 kg auf Ketten nicht
optional:

- **Herzschlag:** Steuerframe 50 Hz. Kein gültiger Frame für **250 ms** → Fahrmotoren auf null und
  bremsen, Turm anhalten, Gang unverändert lassen (nicht unter Last schalten), BB **entwaffnen**.
- **Sequenznummer** im Frame, alte und doppelte Frames verwerfen.
- **Arming:** Fahren und Schießen nur nach explizitem Arm-Kommando, nach Failsafe neu schärfen.
  Die BB-Einheit bekommt ein eigenes Freigabebit — ein Fahrzeug, das nach Funkabriss von allein
  schießt, wird nicht gebaut.
- **Unterspannung:** Schwellen je Akkutyp, siehe Abschnitt Akku (LiPo 10,5 / 9,9 V, Li-Ion
  9,0 / 8,4 V), Meldung per Ack-Payload.
- **Watchdog** im MCU. Treiber so verdrahten, dass ein Reset Stillstand bedeutet: Pulldown auf PWM,
  nicht Pullup.

## Elektrik-Details, die sonst später wehtun

- **3× 100 nF an jedem Motor** (Klemme–Klemme, je Klemme–Gehäuse), Motorleitungen verdrillt,
  Ferritkern. Bürstenfeuer plus nRF24 ist die Kombination, bei der man tagelang das Protokoll
  debuggt, obwohl das Problem am Motor sitzt.
- **470 µF Elko + TVS** direkt am Treibereingang gegen Rückspeisung beim Bremsen.
- Sternförmige Masse ab Akku-Minus; Logik und Leistung nicht in Reihe verdrahten.
- 14 AWG Hauptleitung, 18 AWG Motorleitungen, 20-A-Sicherung oder rücksetzbarer Automat.
- **Kühlung:** 40-mm-Lüfter im Motorraum, die Panther-Decksgitter funktional ausführen.

## Konstruktion und Werkstoffe

- **ASA oder PETG für alles Tragende, kein PLA.** Ein dunkler Panzer in der Sonne erreicht über
  60 °C, da verzieht sich PLA. Getriebegehäuse besser PC-CF oder PA-CF.
- **M3-Gewindeeinsätze** (heat-set) an allen Schraubstellen.
- **Echte Kugellager** an jedem Laufrad (2× 686ZZ, 6×13×5). 0,7 kg je Rad in gedruckten
  Gleitlagern sind nach einer Saison ausgeschlagen.
- **Rahmen und Hülle trennen:** Antriebskräfte über gedruckte Chassis-Längsträger, die Wanne
  darüber ist Verkleidung. Optik und Mechanik bleiben unabhängig überarbeitbar.
- Drehstabfederung: gedruckte Schwingarme mit 4-mm-Stahlstift im Drehpunkt, Federung über
  **Federstahldraht 2,5–3 mm** — näher am Original als Schraubenfedern und passt in den knappen
  Wannenboden.

## Upgrade-Pfad

Der nächste Schritt ist nicht ein größerer Bürstenmotor, sondern **brushless mit FOC**:
2216-Außenläufer plus 20:1-Planetengetriebe an VESC oder SimpleFOC. Bringt Stromregelung (echtes
Drehmomentkommando statt PWM-Ratespiel), lautlosen Lauf, keinen Bürstenverschleiß, Rekuperation;
kostet 4S–6S und Rechenaufwand. Deshalb die Motorbrücke im Chassis von Anfang an mit Platz für ein
**40 × 40 mm Motorgesicht und eine 8-mm-Abtriebswelle** auslegen.

## Bewusst nicht berücksichtigt

Die Belastungsgrenze der Kettenglieder und des gedruckten Triebrads ist **keine**
Auslegungsgrenze — auf Wunsch. Die Antriebsauslegung folgt der Fahrmechanik; wenn ein Kettenglied
oder ein Zahn nachgibt, wird das Teil ersetzt oder verstärkt.

## Offene Punkte

- Zahnpaare 28:32, 13:47 und das Seitenvorgelege 13:47 mit einem Verzahnungsrechner gegenprüfen
  (13 Zähne liegen unter der Unterschnittgrenze von 17 Zähnen bei 20° Eingriffswinkel)
- Motordaten des JGB37-520 am eigenen Motor gegenprüfen, soweit ohne Drehmomentmessung möglich
  (siehe [Motordaten](#motordaten-jgb37-520)), und `antrieb.py` nachziehen
- Schaltschwellen der Firmware an die Motordaten anpassen; Herunterschalten vor dem Wenden auf der
  Stelle vorsehen
- Akkuspannungsmessung (z. B. Spannungsteiler am ESP32-ADC) ist für Abschaltschwellen,
  Unterspannungs-Failsafe und Telemetrie nötig, aber noch nicht eingeplant
- Rollwiderstandsbeiwert 0,15 ist eine Annahme; am fertigen Laufwerk mit Federwaage messen und die
  Rechnung nachziehen
- Gewicht der Kettenglieder aus dem Volumen geschätzt, nicht gewogen; Gliederzahl für 1:10 offen
- Antennenposition erst nach Festlegung der Deckstruktur endgültig
