#include <Arduino.h>
#include <ESP32Servo.h>

struct Lichtschranke {
  int pin; 
  volatile unsigned long lastTickTimestampMicros = 0; 
  volatile unsigned long lastPeriodMicros = 0; 
  //float rpm = 0; 
  float getRpm() {
    unsigned long microsNow = micros(); 
    if (microsNow - this->lastTickTimestampMicros > 1000 * 1000) return 0.0f;  
    return this->lastPeriodMicros == 0 ? 0 : 1000.0f * 1000 * 60 / this->lastPeriodMicros / 32; 
  }
};

struct Getriebe {
  Lichtschranke eingang; 
  Lichtschranke ausgang; 
};
Getriebe links; 

Getriebe rechts; 

const int PIN_LICHTSCHRANKE_R = 4;
const int PIN_LICHTSCHRANKE_L = 18;

constexpr int SERVO_PIN = 5; 
constexpr int PULSE_MID_US = 1500;

Servo servo;
// volatile = "Compiler, cache diese Variable nicht in einem Register."
// Ohne volatile sieht loop() die Aenderung aus der ISR unter Umstaenden nie.
//volatile uint32_t edgeCount = 0;

// Das hier ist die ISR (Interrupt Service Routine).
// IRAM_ATTR ist ESP32-spezifisch: die Funktion muss im RAM liegen, nicht im Flash.
void IRAM_ATTR onSensorChange(void *arg) {
  Lichtschranke *schranke = static_cast<Lichtschranke*>(arg);
  unsigned long microsNow = micros();
  //if (microsNow - schranke->lastTickMicros < 200) return; 

 // unsigned long timeSinceLastTick = microsNow - schranke->lastTickMicros; 
  //Serial.printf("RPM: %f\n", schranke->getRpm(microsNow));
  // -> Millisekunden -> Sekunden -> Minute / Differenz zum letzten Tick / Anzahl an Perforationen in Scheibe 
  //schranke->rpm = 1000.0f * 1000 * 60 / timeSinceLastTick / 32; 

  //jetzigen Timerstand für den nächsten Durchgang speichern 
  schranke->lastPeriodMicros = microsNow - schranke->lastTickTimestampMicros;
  schranke->lastTickTimestampMicros = microsNow; 
}

void setup() {
  Serial.begin(115200);
  Serial.println("Setup - Start");

  links.eingang.pin = PIN_LICHTSCHRANKE_L; 

  pinMode(PIN_LICHTSCHRANKE_L, INPUT_PULLUP);

  attachInterruptArg(digitalPinToInterrupt(PIN_LICHTSCHRANKE_L), onSensorChange, &links.eingang, RISING);

    ESP32PWM::allocateTimer(0);

  servo.setPeriodHertz(50);  // analoger Servo -> 50 Hz. NICHT hochdrehen.

  // Bewusst weiter aufgezogen als der Sweep-Bereich: attach() begrenzt
  // writeMicroseconds() hart. So kannst du oben PULSE_MIN/MAX aendern,
  // ohne hier nachziehen zu muessen.
  servo.attach(SERVO_PIN, 500, 2500);
  
  Serial.println("Setup - End");
}

void loop() {
  //TODO: RPM Berechnung hier rein: ISR wird kürzer, und 0 rpm lässt sich besser rausfinden. 
  unsigned long microsNow = micros(); 
  Serial.print("RPM: ");
  Serial.print(links.eingang.getRpm());
  Serial.print("   Pin gerade: ");
  Serial.println(digitalRead(PIN_LICHTSCHRANKE_L));
servo.writeMicroseconds(PULSE_MID_US);
  delay(50);
}
