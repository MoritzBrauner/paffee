#include <Arduino.h>
#include <ESP32Servo.h>

class Timer {
  
};

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

void IRAM_ATTR onSensorChange(void *arg) {
  Lichtschranke *schranke = static_cast<Lichtschranke*>(arg);
  unsigned long microsNow = micros();
  //jetzigen Timerstand für den nächsten Durchgang speichern 
  schranke->lastPeriodMicros = microsNow - schranke->lastTickTimestampMicros;
  schranke->lastTickTimestampMicros = microsNow; 
}


// Shift Engine

//Input L:  13  
//Output L: 47
//Input H:  28
//Output H: 32

constexpr float ratioFirstGear = 13.0f / 47;
constexpr float ratioSecondGear = 28.0f / 32;   

float inputRpm; 
float outputRpm;

//int currentGear; 
//int desiredGear; 

enum class Gear {
  First, 
  Second, 
  Neutral, 
};

Gear currentGear = Gear::Neutral; 
Gear desiredGear = Gear::Neutral; 

//Left:
void shiftUp();
void shiftDown();
void doStuff () {
  switch (currentGear) {
    case Gear::First: {
      if (outputRpm > 400) {
        desiredGear = Gear::Second; 
        shiftUp(); 
      }
    } 
    break;
    case Gear::Second: {
      if (outputRpm <= 350) {
        desiredGear = Gear::First;
        shiftDown();  
      }
    } 
    break;
    default:; 
  }
 
  if (currentGear != desiredGear) {
    
  }
}

void shiftUp() {
  //setze Servo Mitte 
  //rechne Ziel-RPM aus 
  //Finde irgendwie raus, welcher PWM-Wert dazu korrespondiert 
  //Setze Motor RPM 
  //setze Servo auf 2. Gang 
  unsigned short motorTargetRpm = links.ausgang.getRpm() * (1/ratioSecondGear); 
}

void shiftDown() {

} 





//aktueller Gang
//gewünschter Gang -> Berechnung anhand von Ausgangsrehzahl am Getriebe
// -> runterschalten, wenn im 2. Gang die max. Drehzahl vom 1. unterschritten ist 
//revmatch -> Errechnung soll-Drehzahl per Ausgang und Eingang+PWM 
//Vllt. Speichern von "Kennlinien" vom Motor im Leerlauf (einmal komplett durchlaufen beim Einschalten) -> jeden PWM-Wert auf seine Drehzahl am Motor mappen -> später dann nurnoch die Map abfragen 

// /Shift Engine

void setup() {
  Serial.begin(115200);
  Serial.println("Setup - Start");

  links.eingang.pin = PIN_LICHTSCHRANKE_L; 

  pinMode(PIN_LICHTSCHRANKE_L, INPUT_PULLUP);

  attachInterruptArg(digitalPinToInterrupt(PIN_LICHTSCHRANKE_L), onSensorChange, &links.eingang, RISING);

  //ESP32PWM::allocateTimer(0);

  //servo.setPeriodHertz(50);  // analoger Servo -> 50 Hz. NICHT hochdrehen.
  //servo.attach(SERVO_PIN, 500, 2500);
  
  Serial.println("Setup - End");
}

void loop() {
  //TODO: RPM Berechnung hier rein: ISR wird kürzer, und 0 rpm lässt sich besser rausfinden. 
  unsigned long microsNow = micros(); 
  Serial.print("RPM: ");
  Serial.print(links.eingang.getRpm());
  Serial.print("   Pin gerade: ");
  Serial.println(digitalRead(PIN_LICHTSCHRANKE_L));
  //servo.writeMicroseconds(PULSE_MID_US);
  delay(50);
}