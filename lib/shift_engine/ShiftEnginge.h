#ifndef SHIFTENGINE_H
#define SHIFTENGINE_H

#include <Arduino.h>
#include "Timer.h"
#include <ESP32Encoder.h>
#include <ESP32Servo.h>

constexpr float GEAR_RATIO_FIRST = 13.0f / 47;
constexpr float GEAR_RATIO_SECOND = 28.0f / 32;

constexpr int SERVO_MIDDLE_US = 1500;
constexpr int SERVO_TRAVEL_US = 700;

constexpr int SAMPLE_INTERVAL_MS = 50; 

enum class Message {
  Happy,
  RequestingShiftUp,
  RequestingShiftDown,
}; 

enum class Gear {
  First, 
  Second, 
  Neutral, 
};

enum class Mode {
  Drive,
  Shift,
};

struct RotationSpeeds {
  float input = 0;   
  float output = 0; 
};

//Periodenmessung. Drehzahl ergibt sich aus dem Interval zwischen den letzten 2 Messungen. 
//Interrupt-gesteuert.  

//class Drivetrain {
//    private:
//        Message* messageFromEngine1; 
//        Message* messageFromEngone2; 
//
//        Message* messageToEngine1;
//        Message* messageToEngine2;
//        
//        uint8_t pin;
//    public: 
//
//};

class Transmission {
  private:
    bool initialized = false;
    bool calibrated = false;

    float calibratedMotorRPMs[10] = {0};

    Message message;

    Gear desiredGear;
    Gear currentGear;

    uint8_t pin_motorPwm;
    uint8_t pin_motorDirection;

    uint8_t pin_photoElectricSpeedSensor; 
    volatile unsigned long lastTickMicros = 0; 
    volatile unsigned long lastPeriodMicros = 0; 

    uint8_t pin_hallSensor1; 
    uint8_t pin_hallSensor2; 
    Timer hallTimer; 
    const static float countsPerRev = 275.0f; 
    const static uint32_t HALL_SAMPLE_INTERVAL_MS = 50;
    ESP32Encoder encoder; 
    int64_t lastCount = 0; 

    SingleUseStagedTimer shiftTimer;
    
    void write(uint8_t pwm, bool direction = true);

    static void IRAM_ATTR onSensorChangeISR(void* arg) {
      static_cast<Transmission*>(arg)->onSensorChange();
    }

    void IRAM_ATTR onSensorChange() {
      unsigned long microsNow = micros(); 
      lastPeriodMicros = microsNow - lastTickMicros; 
      lastTickMicros = microsNow; 
    }

  public:
    void init() {
       
      Serial.println("Initializing Transmission...");
      //Set Pin Modes

      //Atach ISR for Optical Sensor
      attachInterruptArg(digitalPinToInterrupt(pin_photoElectricSpeedSensor), onSensorChangeISR, this, RISING); 
      //Attach Encoder
      encoder.attachFullQuad(pin_hallSensor1, pin_hallSensor2); 
      encoder.clearCount(); 
      //Initialize Timers
      hallTimer = Timer(HALL_SAMPLE_INTERVAL_MS); 
      //Finalize
      this->initialized = true;
      Serial.println("Initialization completed."); 
    }

    //Calibrate Motor Speeds: 
    //Set Motor to a PWM Value an see how fast it spins at that speed 
    void calibrate() {
        if (!initialized) {
          Serial.println("Unable to calibrate. ShiftEnginge ist not initialized");
          return;
        }

        Serial.println("Transmission calibration started...");
        Serial.println("Transmission Stop");
        write(0);
        delay(500);
        for (int i = 1; i <= 10; i++) {
          float rpm = getRotionalSpeeds().input;
          Serial.printf("Measured Motor RPM for Signal %d: %f", (i-1)*100, rpm);
          calibratedMotorRPMs[i-1] = rpm;
          write(i * 100);
          delay(500);
        }
        this->calibrated = true;
        Serial.println("Transmission calibration finished.");
    }

    Message getMessage() {
      return this->message;
    }

    RotationSpeeds getRotionalSpeeds() {
      RotationSpeeds rpms; 
      unsigned long millisNow = millis(); 
      //input
      if (hallTimer.fires()) {
        long now = encoder.getCount(); 
        long delta = now - lastCount; 
        lastCount = now; 
        rpms.output = (delta * 60000.0f) / (SAMPLE_INTERVAL_MS * countsPerRev);
      }
      //output 
      if ((millisNow * 1000) - lastTickMicros > 1000 * 1000) {
        rpms.input = 0; 
      } else {
        rpms.input = lastPeriodMicros == 0 ? 0 : 1000.0f * 1000 * 60 / lastPeriodMicros / 32; 
      }  
      return rpms; 
    }

    /*Get both input and output RPM.
    Look for the happiness state: 
      If outPutRPM is above the threshold, request shiftUp via Message 
      If outPutRPM is above the threshold, request shiftDown
    Execute Shifts with the timer 
      */
    void update() {
        if (!initialized) {
            Serial.println("Unable to update. Transmission is not initialized");
            return;
        }
        if (!calibrated) {
            Serial.println("Unable to update. Transmission is not calibrated");
            return;
        }

        if (currentGear == desiredGear) {
          switch (currentGear) {
            case Gear::First: {
              if (getRotionalSpeeds().output > 400) {
                desiredGear = Gear::Second;
                //shiftTimer.reset(true);
              }
            }
            break;
            case Gear::Second: {
              if (getRotionalSpeeds().output <= 350) {
                desiredGear = Gear::First;
                //shiftTimer.reset(true);
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
                //Transmission im Leerlauf hochdrehen; revmatch
                float targetGearRatio = desiredGear == Gear::First ? GEAR_RATIO_FIRST : GEAR_RATIO_SECOND;
                float targetRPM = getOutPutRpm() * (1/targetGearRatio);
                //Hier dann in der Liste den PWM-Wert suchen und interpolieren, der zur geforderten Drehzahl passt
                //Irgendeine Logik noch reinmachen, die den tatsächlichen PWM-Wert überwacht und ggf. korrigiert
                //nur wenn Die Transmission-RPM angeglichen ist:
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
}; 

class Drivetrain {
    private: 
      Transmission leftMotor; 
      Transmission rightMotor; 
      //Servo shiftServo; 

    public: 
      void init(); 
      void calibrate(); 

      void update(uint8_t stickValueX, uint8_t stickValueY) {
        //Find a way to pass Remote input into the engine (maybe with parameters of update)
        //Handle Turning, set desired PWM for both Motors accordingly 
        // ->both update() methods of the motors should only get their own PWM signal 
        //Figure out a way to smoothly set directions of motors -> esp. while turning (neutral steering)  
        //Handle Messages sent by both Engines: 
        // If 2x Happy -> :) 
        // If 2x ShiftUp -> ShiftUp 
        // If 2x ShiftDown -> ShiftDown 
        // else -> stay in current gear? -> this state is often reached while turning 
        //Pass down to engines, which gear we are in 
      }
};

#endif