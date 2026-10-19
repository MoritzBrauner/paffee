# ESP32: Board, Pinbelegung, Fernbedienung

Stand 19.10.2026. Board-Bild, Fernbedienung und GPIO-Übersicht stammen aus dem Vorgängerprojekt
`arduino-gartenbahn-rc-steuerung-receiver-esp32`, die Pinbelegung ist neu für den Panzer. Sie ist
gegen die Boot- und Peripherie-Regeln des ESP32 geprüft, aber noch nicht vollständig verdrahtet.

## Board

NodeMCU-ESP32 mit 38 Pins (ESP32-WROOM-32, in PlatformIO `board = esp32dev`). Auf einem
WROVER-Modul wären 16/17 vom PSRAM belegt, auf 30-Pin-Boards fehlt GPIO0.

![ESP32 NodeMCU Pinout](assets/esp32_pinout.png)

Im Bild ist GPIO12 als ADC1_CH5 beschriftet, richtig ist ADC2_CH5.

Pinout-Referenz: https://lastminuteengineers.com/esp32-pinout-reference/

## Annahmen

- **Turm direkt am Haupt-ESP:** kein Turm-Slave, kein CAN, kein AS5600. Das weicht von den
  Abschnitten Architektur und Turm in [06-antrieb-elektronik.md](06-antrieb-elektronik.md) ab, die
  Entscheidung ist noch offen.
- **Turmlage nur relativ:** Ohne AS5600 und ohne Referenzschalter kennt die Firmware die
  Turmlage nur über gezählte Schritte (z. B. eine PCNT-Unit am STEP-Pin 2). Absolut, etwa für
  „Turm auf 12 Uhr“, erst mit einem Referenzschalter, siehe Reserve unten.
- **TMC2209 im Standalone-Modus:** Strom per Vref, Mikroschritte per MS1/MS2, keine UART.
- **BB-Einheit mit eingebautem Mikroschalter:** Motor über MOSFET, Schalter als Eingang.
- **Getriebe links wie auf dem Prüfstand:** 32, 33, 25, 26, 27 sind dieselben Pins wie in
  [src/test_calibration_and_revmatch](../src/test_calibration_and_revmatch/main.cpp).

Nutzbar sind 24 GPIOs, ohne 1/3 (USB-Seriell) und 6–11 (Flash). Belegt sind alle 24, es gibt
keine Reserve.

## Pinbelegung an den Stiftleisten

Ansicht von oben, Antenne oben, USB unten. Zusammengehörige Signale liegen nebeneinander:

- **linke Leiste:** Akku, beide Getriebe (Encoder-Paare, PWM/DIR links, Lichtschranken), BB-Paar,
  Schaltservo
- **rechte Leiste:** PWM/DIR rechts, Höhenservo, der nRF24 als Sechserblock, der Turmtreiber als
  Dreierblock

| Belegung | Icon | Pin | | Pin | Icon | Belegung |
|---|---|---|---|---|---|---|
| Versorgung 3,3 V | ➕ | 3V3 | | GND | ➖ | Masse |
| Reset | | EN | | 23 | ⚡ | Motor-PWM rechts |
| Akku-Spannung | 🔋 | 36 | | 22 | ↔️ | Motor-DIR rechts |
| Encoder rechts A | ⚙️ | 39 | | 1 | | TX0 (USB) – frei lassen |
| Encoder rechts B | ⚙️ | 34 | | 3 | | RX0 (USB) – frei lassen |
| Lichtschranke rechts | 👁️ | 35 | | 21 | 🎯 | Höhenservo |
| Encoder links A | ⚙️ | 32 | | GND | ➖ | Masse |
| Encoder links B | ⚙️ | 33 | | 19 | 📡 | nRF24 MISO |
| Motor-PWM links | ⚡ | 25 | | 18 | 📡 | nRF24 SCK |
| Motor-DIR links | ↔️ | 26 | | 5 | 📡 | nRF24 CSN |
| Lichtschranke links | 👁️ | 27 | | 17 | 📡 | nRF24 MOSI |
| BB-Mikroschalter | 💥 | 14 | | 16 | 📡 | nRF24 CE |
| BB-MOSFET-Gate | 💥 | 12 | | 4 | 📡 | nRF24 IRQ |
| Masse | ➖ | GND | | 0 | 🔄 | Turmtreiber ENN |
| Schaltservo | 🕹️ | 13 | | 2 | 🔄 | Turm-STEP |
| Flash | ❌ | 9 | | 15 | 🔄 | Turm-DIR |
| Flash | ❌ | 10 | | 8 | ❌ | Flash |
| Flash | ❌ | 11 | | 7 | ❌ | Flash |
| Power IN | ➡️ | 5V | | 6 | ❌ | Flash |

## Pinbelegung nach Baugruppe

| Baugruppe | Signal | GPIO | Beschaltung / Hinweis |
|---|---|---|---|
| Getriebe links | Encoder A / B | 32 / 33 | Encoder an 3V3, nicht 5 V |
| | Motor-PWM / DIR | 25 / 26 | PWM: LEDC-Kanal 2, 10 kΩ Pull-down. DIR: LOW = vorwärts (am Prüfstand ermittelt) |
| | Lichtschranke | 27 | Modul an 3V3, die GPIOs sind nicht 5-V-fest |
| Getriebe rechts | Encoder A / B | 39 / 34 | Encoder an 3V3; **externe** Pull-ups 4,7–10 kΩ nach 3V3, 34–39 haben keine internen |
| | Motor-PWM / DIR | 23 / 22 | PWM: LEDC-Kanal 3, 10 kΩ Pull-down |
| | Lichtschranke | 35 | **externer** 10 kΩ Pull-up nach 3V3 |
| Schaltung | Schaltservo | 13 | LEDC-Kanal 0; 10 kΩ Pull-down |
| Turm | ENN / STEP / DIR | 0 / 2 / 15 | ENN als Open-Drain mit 10 kΩ Pull-up nach 3V3 (nicht 5 V); ein Pull-down des Treibermoduls an EN muss über ~20 kΩ liegen (messen), sonst startet der ESP im Download-Modus. STEP: LEDC-Kanal 4 |
| | Höhenservo | 21 | LEDC-Kanal 1, nur 50 Hz; 10 kΩ Pull-down |
| BB-Einheit | Mikroschalter / MOSFET-Gate | 14 / 12 | Schalter gegen GND, interner Pull-up. Gate: 10 kΩ Pull-down direkt am Gate, nie ein Modul mit Pull-up; bei PWM-Begrenzung LEDC-Kanal 6 |
| Funk nRF24 | MISO / SCK / CSN / MOSI / CE / IRQ | 19 / 18 / 5 / 17 / 16 / 4 | CSN: 10 kΩ Pull-up empfohlen. CE: 10 kΩ Pull-down. IRQ: 10 kΩ Pull-up, laut 06 per Interrupt nutzen |
| Akku | Spannung | 36 | Teiler 47 kΩ / 10 kΩ + 100 nF, hinter dem Hauptschalter |

## Gründe für die Zuordnung

- **Boot-Pins** tragen nur Signale, deren Pegel beim Reset passt:
  - GPIO0 muss beim Reset high sein, sonst Download-Modus. ENN mit Pull-up heißt: Treiber aus.
    Beim Flashen zieht die Auto-Reset-Schaltung GPIO0 kurz low, der Turmtreiber ist dann
    bestromt, bewegt sich ohne STEP aber nicht.
  - GPIO2 muss zum Flashen low sein. STEP ruht low.
  - GPIO12 muss beim Reset low sein, sonst wählt der Chip 1,8-V-Flash und bootet nicht. Gate mit
    Pull-down heißt: BB aus.
  - GPIO5 und 15 haben beim Reset einen Pull-up und steuern zusammen nur das SDIO-Timing, GPIO15
    außerdem das Boot-Log. CSN auf 5 ist beim Start damit high, der nRF bleibt abgewählt.
    Turm-DIR auf 15 ist beim Start high, das ist egal, solange ENN den Treiber abschaltet.
- **16/17** spricht der Core beim Booten kurz als PSRAM-Leitungen an. Dort liegen nRF MOSI und CE.
  Das ist harmlos: CSN ist dabei high, und beim Kaltstart ist der nRF ohnehin im Power-Down.
- **13, 14, 21–23, 25–27, 32, 33** sind ruhige Pins ohne Boot-Funktion. Darauf liegt alles, was
  beim Start nicht zucken darf: beide Motor-PWMs, der Schaltservo und der Höhenservo.
- **34–39** sind reine Eingänge ohne interne Pull-ups. Auf 36/39 erzeugt das Einschalten des ADC
  (Akku-Messung auf 36) Glitches von etwa 80 ns. Auf 39 liegt deshalb ein Encoder-Kanal: Der
  Zähler (PCNT) filtert Pulse unter 3 µs weg. Die Lichtschranke, die per Interrupt läuft, liegt
  auf 35, wo der Fehler nicht auftritt. Die Akku-Spannung liegt auf ADC1, weil ADC2 bei aktivem
  WLAN nicht nutzbar ist.
- **LEDC:** Servos auf 0/1 (Timer 0, 50 Hz), Motoren auf 2/3 (Timer 1, 20 kHz), STEP auf 4
  (Timer 2), BB-PWM auf 6 (Timer 3). Kanal 5 frei lassen, er teilt sich den Timer mit STEP. Der
  Höhenservo muss bei 50 Hz bleiben, sonst bleibt ESP32Servo wegen `allocateTimer(0)` in einer
  Endlosschleife hängen.

## Folgen für die Firmware

- `Transmission links(25, 26, 2, 27, 32, 33)`, `Transmission rechts(23, 22, 3, 35, 39, 34)`,
  `Drivetrain drivetrain(links, rechts, 13)`. Reihenfolge der Parameter: PWM, DIR, LEDC-Kanal,
  Lichtschranke, Encoder A, Encoder B.
- `Transmission::init()` setzt für die Lichtschranke `INPUT_PULLUP` und der Encoder seine internen
  Pull-ups. Auf 34, 35 und 39 wirkt beides nicht, dort müssen die Pull-ups extern sitzen.
- **nRF24: SPI ausdrücklich mit Pins starten, bevor der Funk startet:** `SPI.begin(18, 19, 17, 5);`,
  dann `RF24 radio(16, 5); radio.begin();`. `RF24::begin()` ruft intern `SPI.begin()` ohne Pins
  auf. Läuft das zuerst, legt es MOSI auf den Standard-Pin 23, also auf die Motor-PWM rechts. Ein
  späteres `SPI.begin()` ist wirkungslos. Gegenüber dem Gartenbahn-Projekt ändern sich damit CE,
  CSN und MOSI.
- Die Getriebe sind gespiegelt eingebaut. Ob „LOW = vorwärts“ auch rechts gilt, steht noch aus.
  Werden rechts die Motorleitungen getauscht, damit vorwärts auch vorwärts fährt, zählt der
  Encoder rückwärts, und `calibrate()` bricht an der Plausibilitätsprüfung ab. Dann auch Encoder
  A/B tauschen: `Transmission rechts(23, 22, 3, 35, 34, 39)`.
- Keine Bibliothek mit ihren Standard-Pins starten: `Wire.begin()` belegt 21/22 (Höhenservo,
  Motor-DIR rechts), `Serial2.begin()` belegt 16/17 (nRF CE/MOSI). Für I2C-Bausteine wie INA226
  oder eine IMU ist kein Pin frei, die gehen nur mit Turm-Slave, Port-Expander oder nach Wegfall
  eines Signals.
- Ein Potentiometer als Fernbedienungsersatz im Test braucht einen ADC-Pin. Ohne Funk ist 4
  (nRF24 IRQ) frei. Er liegt auf ADC2, das geht, solange das WLAN aus ist.
- Testprogramme: [src/test_servo](../src/test_servo/main.cpp) nutzt für den Servo noch GPIO5, der
  jetzt nRF24 CSN ist, und geht auf 13. Der Gang-Schalter in
  [src/test_calibration_and_revmatch](../src/test_calibration_and_revmatch/main.cpp) liegt auf 14,
  dem späteren BB-Mikroschalter. Beide sind Schalter gegen GND mit Pull-up, das passt zusammen.

Reserve gewinnen, falls später ein Turm-Referenzschalter dazukommt: den nRF-IRQ pollen statt per
Interrupt zu nutzen, wie im Gartenbahn-Code. Das weicht von der Festlegung in 06 ab und ist nur
der Notausgang. Dann wird 4 frei, ein normaler Pin mit internem Pull-up, passend für einen
Hall-Schalter. TMC-ENN fest auf GND zu legen macht zwar GPIO0 frei (der Treiber ist dann dauerhaft
bestromt, bewegt sich ohne STEP aber nicht), GPIO0 taugt aber nur für Signale, die beim Reset
sicher high sind, also nicht für einen Schalter.

## GPIO-Übersicht

Aus dem Gartenbahn-Projekt übernommen und korrigiert (GPIO2, 5, 14, 15, 16/17, 34–39).

| Bezeichnung | GPIO | Sicher? | Grund |
|---|---|---|---|
| D0 | 0 | ⚠️ | Muss **beim Reset high** sein (Pull-up), low startet den Download-Modus; BOOT-Taste zieht ihn auf low |
| TX0 | 1 | ❌ | TX-Pin, wird zum Flashen und Debuggen verwendet |
| D2 | 2 | ⚠️ | Muss **zum Flashen low oder offen** sein (Pull-down beim Reset); je nach Board mit On-Board-LED |
| RX0 | 3 | ❌ | RX-Pin, wird zum Flashen und Debuggen verwendet |
| D4 | 4 | ✅ | — |
| D5 | 5 | ⚠️ | Strapping-Pin mit Pull-up beim Reset; steuert nur das SDIO-Timing, für den Boot egal |
| D6–D11 | 6–11 | ❌ | Mit dem Flash-Speicher verbunden |
| D12 | 12 | ⚠️ | Muss **beim Reset low** sein, sonst 1,8-V-Flash und der ESP bootet nicht |
| D13 | 13 | ✅ | — |
| D14 | 14 | ✅ | Floatet beim Reset |
| D15 | 15 | ⚠️ | Pull-up beim Reset; steuert mit GPIO5 das SDIO-Timing, low schaltet das Boot-Log ab |
| RX2 | 16 | ⚠️ | Auf WROOM frei, wird beim Booten kurz vom PSRAM-Test angesprochen; auf WROVER belegt |
| TX2 | 17 | ⚠️ | wie 16 |
| D18 | 18 | ✅ | — |
| D19 | 19 | ✅ | — |
| D21 | 21 | ✅ | — |
| D22 | 22 | ✅ | — |
| D23 | 23 | ✅ | — |
| D25 | 25 | ✅ | — |
| D26 | 26 | ✅ | — |
| D27 | 27 | ✅ | — |
| D32 | 32 | ✅ | — |
| D33 | 33 | ✅ | — |
| D34 | 34 | ⚠️ | Nur Eingang, keine internen Pull-ups |
| D35 | 35 | ⚠️ | Nur Eingang, keine internen Pull-ups |
| VP | 36 | ⚠️ | Nur Eingang, keine internen Pull-ups; Glitch beim Einschalten des ADC |
| VN | 39 | ⚠️ | Nur Eingang, keine internen Pull-ups; Glitch beim Einschalten des ADC |

ADC2 (GPIO 0, 2, 4, 12–15, 25–27) ist bei aktivem WLAN nicht als Analogeingang nutzbar, nur ADC1
(GPIO 32–39).

## Fernbedienung

![Fernbedienung](assets/fernbedienung.jpeg)

Der Sender aus dem Gartenbahn-Projekt, ein umgebauter MTX-47. Der Aufdruck „DSM“ stammt vom
Original, gefunkt wird per nRF24. Das ist aus dem Gartenbahn-Empfänger erschlossen, der nur RF24
nutzt. Die Sender-Firmware liegt in keinem der beiden Repos.

Basis für den Panzer ist das Gartenbahn-Paket: Adresse `00100`, 10 Byte, RF24-Standardwerte
(1 Mbps, Kanal 76):

```cpp
struct __attribute__((packed)) Data_Package {
  uint16_t lx;  //Stick links X, 0..1023
  uint16_t ly;  //Stick links Y
  uint8_t  lz;  //Schalter links
  uint16_t rx;  //Stick rechts X
  uint16_t ry;  //Stick rechts Y
  uint8_t  rz;  //Schalter rechts
};
```

Für den Failsafe aus [06-antrieb-elektronik.md](06-antrieb-elektronik.md) muss es um
Sequenznummer, Arm-Kommando und BB-Freigabebit erweitert werden; Datenrate und Kanal ebenfalls
laut 06. Die Belegung für den Panzer ist noch offen:

| Bedienelement | Belegung Panzer |
|---|---|
| Stick links, X-Achse | offen |
| Stick links, Y-Achse | offen |
| Stick rechts, X-Achse | offen |
| Stick rechts, Y-Achse | offen |
| Schalter links | offen |
| Schalter rechts | offen |
