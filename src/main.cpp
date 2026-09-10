#include <Arduino.h>

// Drehzahlmessung per Lochscheibe + Lichtschranke (ESP32).
// Die Flanken der Lichtschranke werden per Hardware-Interrupt erfasst,
// die Ausgabe passiert entkoppelt davon in loop().

// ---------------------------------------------------------------- Konfiguration

// GPIO2 ist auf dem ESP32 ein Strapping-Pin und haengt auf vielen DevKits an
// der Onboard-LED -> der interne Pull-up kommt gegen die LED nicht an, und
// beim Flashen kann ein High-Pegel den Download-Mode stoeren.
// GPIO4 ist unkritisch. Nicht nehmen: 0, 2, 6-11, 12, 15 und 34-39
// (34-39 sind reine Eingaenge OHNE internen Pull-up).
constexpr int      SENSOR_PIN         = 4;

constexpr int      SLOTS_PER_REV      = 32;      // Schlitze in der Lochscheibe
constexpr uint32_t MIN_PULSE_US       = 200;     // Stoerimpulse kuerzer als das ignorieren
constexpr uint32_t STANDSTILL_US      = 500000;  // keine Flanke seit 0,5 s -> Drehzahl 0
constexpr uint32_t PRINT_INTERVAL_MS  = 250;

// ---------------------------------------------------------------- ISR <-> loop

// Alles, was ISR und loop() gemeinsam anfassen, muss volatile sein, sonst
// optimiert der Compiler die Zugriffe weg.
volatile uint32_t g_lastEdgeUs = 0;  // Zeitstempel der letzten gueltigen Flanke
volatile uint32_t g_periodUs   = 0;  // Dauer einer Schlitz-Periode
volatile uint32_t g_pulseCount = 0;  // gezaehlte Flanken seit Start

// Der ESP32 hat zwei Kerne: der Spinlock sorgt dafuer, dass loop() die drei
// Werte als zusammengehoerigen Schnappschuss liest.
portMUX_TYPE g_mux = portMUX_INITIALIZER_UNLOCKED;

// IRAM_ATTR: die ISR muss im RAM liegen, nicht im Flash.
void IRAM_ATTR onSensorEdge() {
  const uint32_t now = micros();
  const uint32_t dt  = now - g_lastEdgeUs;  // unsigned -> Ueberlauf rechnet sich raus

  if (dt < MIN_PULSE_US) {
    return;  // Prellen / Stoerimpuls
  }

  portENTER_CRITICAL_ISR(&g_mux);
  g_lastEdgeUs = now;
  g_periodUs   = dt;
  g_pulseCount++;
  portEXIT_CRITICAL_ISR(&g_mux);
}

// ---------------------------------------------------------------- setup / loop

void setup() {
  Serial.begin(115200);
  Serial.println("Setup - Start");
  Serial.println("Program: Paffee");

  pinMode(SENSOR_PIN, INPUT_PULLUP);

  g_lastEdgeUs = micros();

  // Nur RISING: eine Flanke pro Schlitz. Damit ist die Messung unabhaengig
  // davon, ob Schlitz und Steg gleich breit sind.
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), onSensorEdge, RISING);

  Serial.println("Setup - End");
}

void loop() {
  static uint32_t lastPrintMs = 0;

  const uint32_t nowMs = millis();
  if (nowMs - lastPrintMs < PRINT_INTERVAL_MS) {
    return;
  }
  lastPrintMs = nowMs;

  portENTER_CRITICAL(&g_mux);
  const uint32_t periodUs = g_periodUs;
  const uint32_t lastUs   = g_lastEdgeUs;
  const uint32_t count    = g_pulseCount;
  portEXIT_CRITICAL(&g_mux);

  // Eine Schlitz-Periode entspricht 1/SLOTS_PER_REV Umdrehung.
  float rpm = 0.0f;
  const bool spinning = (periodUs > 0) && ((micros() - lastUs) < STANDSTILL_US);
  if (spinning) {
    rpm = 60000000.0f / ((float)periodUs * SLOTS_PER_REV);
  }

  Serial.printf("revs: %8.2f   rpm: %7.1f\n", (float)count / SLOTS_PER_REV, rpm);
}
