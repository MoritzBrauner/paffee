#include <Arduino.h>
#include <ESP32Servo.h>

// Testprogramm: faehrt einen Futaba S3003 langsam durch seinen Stellbereich.
//
//   pio run -e test_servo -t upload -t monitor
//
// VORSICHT: Faehrt der Servo gegen seinen mechanischen Anschlag, brummt er,
// zieht Dauerstrom und zerlegt sich auf Dauer das Getriebe. Deshalb startet
// dieser Test im konservativen Bereich 1000-2000 us. Wie du ihn sauber
// ausweitest, steht unten bei PULSE_MIN_US.

constexpr int SERVO_PIN = 5;   // freier GPIO, kein Strapping-Pin

// Der S3003 ist ein analoger Standard-Servo: 50 Hz Rahmen, Neutral bei ~1500 us.
constexpr int PULSE_MID_US = 1500;

// Stellbereich des Sweeps. 1000/2000 us sind der sichere Standardbereich
// (grob 90-100 Grad). Der S3003 kann meist mehr.
//
// Zum Ausloten der echten Endlagen: in 50-us-Schritten erweitern
// (950/2050, dann 900/2100, ...) und zuhoeren. Sobald der Servo an einer
// Endlage brummt, ohne sich weiterzudrehen, ist der Anschlag erreicht
// -> sofort wieder 50-100 us zurueck und dort belassen.
//constexpr int PULSE_MIN_US = 500  ;
//constexpr int PULSE_MAX_US = 2500;

constexpr int PULSE_MIN_US = 1500 - 670;
constexpr int PULSE_MAX_US = 1500 + 670;

constexpr int STEP_US       = 5;    // Schrittweite pro Update
constexpr int STEP_DELAY_MS = 20;   // Tempo: kleiner = schneller
constexpr int HOLD_MS       = 2000;  // Pause an den Endlagen

Servo servo;

// Faehrt den Servo in kleinen Schritten von fromUs nach toUs.
// Ohne diese Rampe wuerde er mit voller Geschwindigkeit springen.
void sweepTo(int fromUs, int toUs) {
  const int step = (toUs >= fromUs) ? STEP_US : -STEP_US;

  for (int us = fromUs; (step > 0) ? (us <= toUs) : (us >= toUs); us += step) {
    servo.writeMicroseconds(us);

    if (us % 100 == 0) {
      Serial.printf("  %4d us\n", us);
    }
    delay(STEP_DELAY_MS);
  }

  servo.writeMicroseconds(toUs);  // exakte Endlage, falls STEP_US nicht aufgeht
}

void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println();
  Serial.println("=== Servo-Test S3003 ===");
  Serial.printf("Pin: GPIO%d   Bereich: %d..%d us\n",
                SERVO_PIN, PULSE_MIN_US, PULSE_MAX_US);

  // Der ESP32 erzeugt das Signal mit einem seiner LEDC-Timer.
  ESP32PWM::allocateTimer(0);

  servo.setPeriodHertz(50);  // analoger Servo -> 50 Hz. NICHT hochdrehen.

  // Bewusst weiter aufgezogen als der Sweep-Bereich: attach() begrenzt
  // writeMicroseconds() hart. So kannst du oben PULSE_MIN/MAX aendern,
  // ohne hier nachziehen zu muessen.
  servo.attach(SERVO_PIN, 500, 2500);

  Serial.println("Fahre auf Mittelstellung...");
  servo.writeMicroseconds(PULSE_MID_US);
  delay(1000);



  Serial.println("Start.");
}

void loop() {
  Serial.println("Fahre hoch..");
  sweepTo(PULSE_MID_US, PULSE_MAX_US);
  delay(HOLD_MS);

  Serial.println("Fahre runter..");
  sweepTo(PULSE_MAX_US, PULSE_MIN_US);
  delay(HOLD_MS);

  Serial.println("In Mittelstellung.."); 
  servo.writeMicroseconds(PULSE_MID_US);
  delay(HOLD_MS);

}
