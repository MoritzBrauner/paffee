#include <Arduino.h>
#include <ESP32Servo.h>
#include <Timer.h>

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

constexpr int SERVO_NEUTRAL_US = 1500; 
constexpr int SERVO_FIRST_US = SERVO_NEUTRAL_US - 700; 
constexpr int SERVO_SECOND_US = SERVO_NEUTRAL_US + 700; 

SingleUseStagedTimer shiftTimer(500, 4, false); 

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

constexpr float GEAR_RATIO_FIRST = 13.0f / 47;
constexpr float GEAR_RATIO_SECOND = 28.0f / 32;   

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
void doStuff () {
  if (currentGear == desiredGear) {
    switch (currentGear) {
      case Gear::First: {
        if (outputRpm > 400) {
          desiredGear = Gear::Second;
          shiftTimer.reset(true); 
        }
      } 
      break;
      case Gear::Second: {
        if (outputRpm <= 350) {
          desiredGear = Gear::First;
          shiftTimer.reset(true);
        }
      } 
      break;
      default:; 
    }  
  }
  else {
    //if (desiredGear == Gear::First) {
      switch (shiftTimer.getStage()) {
        case 1: {
          //Servo auf Mittelstellung, kurz warten 
          servo.writeMicroseconds(SERVO_NEUTRAL_US);
        } 
        break; 
        case 2: {
          shiftTimer.pause(); 
          //Motor im Leerlauf hochdrehen; revmatch
          float targetGearRatio = desiredGear == Gear::First ? GEAR_RATIO_FIRST : GEAR_RATIO_SECOND; 
          float targetRPM = links.ausgang.getRpm() * (1/targetGearRatio); 
          //Hier dann in der Liste den PWM-Wert suchen und interpolieren, der zur geforderten Drehzahl passt  
          //Irgendeine Logik noch reinmachen, die den tatsächlichen PWM-Wert überwacht und ggf. korrigiert
          //nur wenn Die Motor-RPM angeglichen ist: 
          shiftTimer.advance();  
        }
        break;  
        case 3: {
          servo.writeMicroseconds(SERVO_FIRST_US);
        }
        break; 
        case 4: {
          //Schaltvorgang als abgeschlossen markieren 
          currentGear = Gear::First; 
        }
        break;  
      }
  } 
}


//aktueller Gang
//gewünschter Gang -> Berechnung anhand von Ausgangsrehzahl am Getriebe
// -> runterschalten, wenn im 2. Gang die max. Drehzahl vom 1. unterschritten ist 
//revmatch -> Errechnung soll-Drehzahl per Ausgang und Eingang+PWM 
//Vllt. Speichern von "Kennlinien" vom Motor im Leerlauf (einmal komplett durchlaufen beim Einschalten) -> jeden PWM-Wert auf seine Drehzahl am Motor mappen -> später dann nurnoch die Map abfragen 

// /Shift Engine

//float* leftMotorRpms[10] = {NULL};
//
//void writeMotorPwmLeft(uint8_t pwm); 
//float getMotorRpmLeftLast500Millis(); 
//
//void calibrateMotors() {
//  Serial.println("Motor calibration started...");
//  StagedTimer calibrationTimer(500, 21); 
//  bool calibrationFinished = false; 
//  while (!calibrationFinished) {
//    int stage = calibrationTimer.getStage();
//    Serial.printf("Calibration step %d of %d \n", stage, 10); 
//    if (stage % 2 == 0) {
//      int pwm = stage/2 * 10;
//      Serial.printf("Setting Motor PWM to: %d\n", pwm); 
//      writeMotorPwmLeft(stage/2 * 10); 
//    } else {
//      float rpm = getMotorRpmLeftLast500Millis();
//      Serial.printf("Reading corresponding Motor RPM: %f\n", rpm);
//      int index = (stage-1)/2;
//      if (leftMotorRpms[index] == NULL) {
//        leftMotorRpms[index] = &rpm; 
//      } else {
//        break;
//      }
//    }
//    if (stage == 21) calibrationFinished = true;  
//  }
//  Serial.println("Motor calibration finished."); 
//}


float leftMotorRpms[10] = {0};
void writeMotorPwmLeft(uint8_t pwm); 
float getMotorRpmLeft();

void calibrateMotors() {
  Serial.println("Motor calibration started...");
  Serial.println("Motor Stop");
  writeMotorPwmLeft(0); 
  delay(500); 
  for (int i = 0; i < 10; i++) {
    float rpm = getMotorRpmLeft();
    leftMotorRpms[i-1] = getMotorRpmLeft(); 
    writeMotorPwmLeft(i * 10); 
    delay(500); 
  }
  Serial.println("Motor calibration finished."); 
}

void setup() {
  Serial.begin(115200);
  Serial.println("Setup - Start");

  links.eingang.pin = PIN_LICHTSCHRANKE_L; 

  pinMode(PIN_LICHTSCHRANKE_L, INPUT_PULLUP);

  attachInterruptArg(digitalPinToInterrupt(PIN_LICHTSCHRANKE_L), onSensorChange, &links.eingang, RISING);

  //ESP32PWM::allocateTimer(0);

  calibrateMotors(); 

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