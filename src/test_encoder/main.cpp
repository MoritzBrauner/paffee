#include <Arduino.h>
#include <ESP32Encoder.h>

// Testprogramm fuer einen JGB37-520 Encoder.
//
//   pio run -e test_encoder -t upload -t monitor
//
// Motortreiber wird hier NICHT gebraucht - Abtriebswelle von Hand drehen.
// Verkabelung:
//   C1 (gruen) -> PIN_C1          C2 (gelb)     -> PIN_C2
//   VCC (blau) -> 3V3, NICHT 5V   GND (schwarz) -> GND
//   M1/M2 bleiben unangeschlossen.

// Beides normale GPIOs, keine Strapping-Pins, interner Pull-up vorhanden.
constexpr int PIN_C1 = 32;
constexpr int PIN_C2 = 33;

// ACHTUNG: Platzhalter. Rechnerisch 11 Impulse x 4 (Vollquadratur) x Untersetzung,
// aber die Untersetzung weicht bei diesen Motoren gern von der Katalogangabe ab.
// Mit 'n' / 'm' den echten Wert messen und hier eintragen.
float countsPerRev = 275.0f;

constexpr int      KALIBRIER_UMDREHUNGEN = 10;
constexpr uint32_t SAMPLE_INTERVAL_MS    = 50;   // so macht es der Regler spaeter auch
constexpr uint32_t PRINT_INTERVAL_MS     = 250;

ESP32Encoder encoder;

int64_t lastCount = 0;
float   rpm       = 0.0f;

void printHilfe() {
  Serial.println();
  Serial.println("Befehle:");
  Serial.println("  n  Zaehler nullen  -> danach die Welle von Hand drehen");
  Serial.printf ("  m  messen          -> Counts / %d = Counts pro Umdrehung\n",
                 KALIBRIER_UMDREHUNGEN);
  Serial.println("  ?  diese Hilfe");
  Serial.println();
}

void handleSerial() {
  if (!Serial.available()) return;

  switch (Serial.read()) {
    case 'n':
      encoder.clearCount();
      lastCount = 0;
      Serial.printf("\n>> Zaehler genullt. Jetzt die Abtriebswelle genau %d volle "
                    "Umdrehungen drehen, dann 'm' druecken.\n\n",
                    KALIBRIER_UMDREHUNGEN);
      break;

    case 'm': {
      const int64_t c = encoder.getCount();
      Serial.println();
      Serial.printf(">> %lld Counts  ->  %.1f Counts pro Umdrehung\n",
                    c, (double)c / KALIBRIER_UMDREHUNGEN);
      Serial.println("   Wert oben bei countsPerRev eintragen.");
      Serial.println("   (Minuszeichen = nur die Zaehlrichtung, kein Fehler)");
      Serial.println();
      break;
    }

    case '?':
      printHilfe();
      break;

    default:;  // Zeilenumbrueche ignorieren
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println();
  Serial.println("=== Encoder-Test JGB37-520 ===");

  // Schadet nicht, falls das Encoder-Board eigene Pull-ups hat - noetig,
  // falls die Ausgaenge offen sind. In 0.10.2 heisst der Wert UP, nicht puType::up.
  ESP32Encoder::useInternalWeakPullResistors = UP;

  // Vollquadratur: beide Flanken beider Kanaele -> vierfache Aufloesung.
  encoder.attachFullQuad(PIN_C1, PIN_C2);
  encoder.clearCount();

  Serial.printf("C1=GPIO%d   C2=GPIO%d\n", PIN_C1, PIN_C2);
  printHilfe();
}

void loop() {
  static uint32_t lastSampleMs = 0;
  static uint32_t lastPrintMs  = 0;

  handleSerial();

  const uint32_t nowMs = millis();

  // Drehzahl aus der Zaehlerdifferenz ueber ein festes Zeitfenster.
  // Besser als Periodenmessung: gleichmaessig getaktet und mittelt von selbst.
  if (nowMs - lastSampleMs >= SAMPLE_INTERVAL_MS) {
    lastSampleMs = nowMs;

    const int64_t now   = encoder.getCount();
    const int64_t delta = now - lastCount;
    lastCount = now;

    rpm = (delta * 60000.0f) / (SAMPLE_INTERVAL_MS * countsPerRev);
  }

  if (nowMs - lastPrintMs >= PRINT_INTERVAL_MS) {
    lastPrintMs = nowMs;
    Serial.printf("count=%9lld   rpm=%8.1f\n", encoder.getCount(), rpm);
  }
}
