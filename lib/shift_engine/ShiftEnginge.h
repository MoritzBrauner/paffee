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
constexpr int MOTOR_PWM_FREQUENCY = 20000; //Check later for Correctness 

constexpr unsigned long MAX_REVMATCH_TIMEOUT_MICROS = 2000UL * 1000;

enum class Message {
  Happy,
  RequestingShiftUp,
  RequestingShiftDown,
  RevMatchCompleted,
}; 

enum class Directive {
  DoNothing, 
  ShiftUp,
  ShiftDown, 
  EngageClutch,  
};

struct Messages {
  Message left;
  Message right;
  Messages(Message left, Message right) : left(left), right(right) {}
  bool areEqual() { return (left == right);} 
  Directive getDirective() {
    if (!areEqual()) return Directive::DoNothing; 
    if (left == Message::RequestingShiftUp)
      return Directive::ShiftUp;
    if (left == Message::RequestingShiftDown)
      return Directive::ShiftDown;
    if (left == Message::RevMatchCompleted)
      return Directive::EngageClutch;
    return Directive::DoNothing;
  }
};

enum class Gear {
  First, 
  Second, 
  Neutral, 
};

enum class Mode {
  Drive,
  Shift,
  RevMatch,
};

enum class Direction {
  Forward, 
  Backward,
};

struct RotationSpeeds {
  float input = 0;   
  float output = 0; 

  RotationSpeeds(float input, float output) : input(input), output(output) {}
};

class Transmission {
  private:
    bool initialized = false;
    bool calibrated = false;

    float calibratedMotorRPMs[10] = {0};

    Message message;

    Mode mode;
    
    Gear targetGear = Gear::First;
    float targetRatio = GEAR_RATIO_FIRST;
    Gear currentGear = Gear::First;
    float currentRatio = GEAR_RATIO_FIRST;

    uint8_t pin_motorPwm;
    uint8_t pin_motorDirection;

    uint8_t pin_photoElectricSpeedSensor; 
    volatile unsigned long lastTickMicros = 0; 
    volatile unsigned long lastPeriodMicros = 0; 
    float lastMeasuredOutputRpm = 0;

    uint8_t pin_hallSensor1; 
    uint8_t pin_hallSensor2; 
    Timer hallTimer; 
    const static float countsPerRev = 275.0f; 
    const static uint32_t HALL_SAMPLE_INTERVAL_MS = 50;
    ESP32Encoder encoder; 
    int64_t lastCount = 0; 
    float lastMeasuredInputRpm = 0; 

    //RotationSpeeds rpms;

    SingleUseStagedTimer shiftTimer;
    
    void write(uint8_t pwm, Direction direction) {
      ledcWrite(0, pwm); 
      digitalWrite(pin_motorDirection, direction == Direction::Forward ? HIGH : LOW); 
    }

    float getTargetPwmByRpm(float rpm) {
      //Interpolate between calibratedMotorRPMs and their corresponding PWM values 
      //to find the PWM value that corresponds to the desired RPM 
      for (int i = 0; (i + 1) < std::size(calibratedMotorRPMs); i++) {
        if (rpm >= calibratedMotorRPMs[i] && rpm <= calibratedMotorRPMs[i+1]) {
          float pwm1 = i * 100; 
          float pwm2 = (i+1) * 100; 
          float rpm1 = calibratedMotorRPMs[i]; 
          float rpm2 = calibratedMotorRPMs[i+1]; 
          float pwm = pwm1 + (rpm - rpm1) * (pwm2 - pwm1) / (rpm2 - rpm1); 
          return pwm; 
        }
      }
      return 0;
    }

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
        write(0, Direction::Forward);
        delay(500);
        for (int i = 1; i <= 10; i++) {
          float rpm = getRotationalSpeeds().input;
          Serial.printf("Measured Motor RPM for Signal %d: %f", (i-1)*100, rpm);
          calibratedMotorRPMs[i-1] = rpm;
          write(i * 100, Direction::Forward);
          delay(500);
        }
        this->calibrated = true;
        Serial.println("Transmission calibration finished.");
    }

    Message getMessage() {
      return this->message;
    }

    void setMode(Mode mode) {
      this->mode = mode; 
    }

    void setTargetGear(Gear gear) {
      this->targetGear = gear; 
      this->targetRatio = gear == Gear::First ? GEAR_RATIO_FIRST : GEAR_RATIO_SECOND;
    }

    void setCurrentGear(Gear gear) {
      this->currentGear = gear; 
      this->currentRatio = gear == Gear::First ? GEAR_RATIO_FIRST : GEAR_RATIO_SECOND;
    }

    

    //void updateRotationalSpeeds() {
    //  //RotationSpeeds rpms; 
    //  unsigned long millisNow = millis(); 
    //  //input
    //  if (hallTimer.fires()) {
    //    long now = encoder.getCount(); 
    //    long delta = now - lastCount; 
    //    lastCount = now; 
    //    rpms.output = (delta * 60000.0f) / (SAMPLE_INTERVAL_MS * countsPerRev);
    //  }
    //  //output 
    //  if ((millisNow * 1000) - lastTickMicros > 1000 * 1000) {
    //    rpms.input = 0; 
    //  } else {
    //    rpms.input = lastPeriodMicros == 0 ? 0 : 1000.0f * 1000 * 60 / lastPeriodMicros / 32; 
    //  }  
    //  //return rpms; 
    //}

    RotationSpeeds getRotationalSpeeds() {
      //RotationSpeeds rpms(lastMeasuredInputRpm, 0);
      //input
      if (hallTimer.fires()) {
        long now = encoder.getCount(); 
        long delta = now - lastCount; 
        lastCount = now; 
        lastMeasuredInputRpm = (delta * 60000.0f) / (SAMPLE_INTERVAL_MS * countsPerRev);
      }
      //output
      //Erst den Zeitstempel kopieren, dann micros() lesen: Kommt die ISR dazwischen,
      //waere lastTickMicros sonst neuer als microsNow und die Differenz liefe ueber.
      unsigned long lastTick = lastTickMicros;
      unsigned long microsNow = micros();
      if (microsNow - lastTick > 1000UL * 1000) {
        lastMeasuredOutputRpm = 0;
      } else {
        lastMeasuredOutputRpm = lastPeriodMicros == 0 ? 0 : 1000.0f * 1000 * 60 / lastPeriodMicros / 32; 
      }  
      return RotationSpeeds(lastMeasuredInputRpm, lastMeasuredOutputRpm); 
    }


    /*Get both input and output RPM.
    Look for the happiness state: 
      If outPutRPM is above the threshold, request shiftUp via Message 
      If outPutRPM is above the threshold, request shiftDown
    Implement 2 Modes: 
      DriveMode -> Motor gets PWM from Remote Control, normal driving
      ShiftMode -> Motor just tries to RevMatch the output
      */
    void update(uint16_t normalizedPwm, Direction direction) {
        if (!initialized) {
            Serial.println("Unable to update. Transmission is not initialized");
            return;
        }
        if (!calibrated) {
            Serial.println("Unable to update. Transmission is not calibrated");
            return;
        }

        //updateRotationalSpeeds();

        RotationSpeeds rpms = getRotationalSpeeds();

        if (mode == Mode::Drive) {
          //Set Motor PWM according to Remote Control Input 
          write(normalizedPwm, direction);
          //View Output RPM and decide whether we are happy or not
          //  if not -> set Message to RequestingShiftUp or RequestingShiftDown
          switch (currentGear) {
            case Gear::First: {
              if (rpms.output > 400) {
                message = Message::RequestingShiftUp;
              }
            }
            break;
            case Gear::Second: {
              if (rpms.output <= 350) {
                message = Message::RequestingShiftDown;
              }
            }
            break;
            default: {
              message = Message::Happy; // :) 
            }
          }
        } else {
          //ignore Remote Control Input, just RevMatch the output RPM to the targetGearRatio
          float targetPwm = getTargetPwmByRpm(rpms.output * (1/targetRatio));
          write(targetPwm, direction);
          const uint8_t tolerance = 20;
          if (rpms.input >= rpms.output * (1/targetRatio) - tolerance && rpms.input <= rpms.output * (1/targetRatio) + tolerance) {
            message = Message::RevMatchCompleted; 
          }
        } 
    }
}; 

class Drivetrain {
    private: 
      Transmission leftTrans; 
      Transmission rightTrans; 

      SingleUseStagedTimer shiftTimer; 

      Gear currentGear;
      Gear desiredGear;

      Servo servo; 

      unsigned long revmatchStartMicros = 0;

      Directive getTransMessages() {
        return Messages(
          leftTrans.getMessage(), 
          rightTrans.getMessage() 
        ).getDirective();
      }

    public: 
      //Init both transeseses
      void init(); 
      //Calibrate both transesesees
      void calibrate(); 

      void initializeGearShift(Gear gear) {
        desiredGear = gear; 
        shiftTimer.reset(true); 
        leftTrans.setMode(Mode::Shift);
        rightTrans.setMode(Mode::Shift); 
      }

      void initializeRevMatch() {
        leftTrans.setTargetGear(desiredGear);
        rightTrans.setTargetGear(desiredGear);
        revmatchStartMicros = micros();
        leftTrans.setMode(Mode::RevMatch);
        rightTrans.setMode(Mode::RevMatch); 
      }

      void finalizeShift(Gear gear) {
        currentGear = gear; 
        leftTrans.setCurrentGear(gear);
        rightTrans.setCurrentGear(gear);
        leftTrans.setMode(Mode::Drive);
        rightTrans.setMode(Mode::Drive); 
      }

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
        Directive directive = getTransMessages(); 
        if (desiredGear == currentGear) {
          switch (directive) {
          case Directive::ShiftUp: {
            initializeGearShift(Gear::Second);
          }
          break;
          case Directive::ShiftDown: {
            initializeGearShift(Gear::First);
          }
          break;
        }
          //leftTrans.setMode(Mode::Shift);
          //rightTrans.setMode(Mode::Shift);
        } else {
          switch (shiftTimer.getStage()) {
            case 1: {
              //Servo auf Mittelstellung, kurz warten 
              servo.writeMicroseconds(SERVO_MIDDLE_US);
            } 
            break; 
            case 2: {
              shiftTimer.pause(); 
              //Motor im Leerlauf hochdrehen; revmatch
              initializeRevMatch();
              //leftTrans.setTargetGear(desiredGear);
              //rightTrans.setTargetGear(desiredGear);
              //float targetGearRatio = desiredGear == Gear::First ? GEAR_RATIO_FIRST : GEAR_RATIO_SECOND; 
              //float targetRPM = links.ausgang.getRpm() * (1/targetGearRatio); 
              //Hier dann in der Liste den PWM-Wert suchen und interpolieren, der zur geforderten Drehzahl passt  
              //Irgendeine Logik noch reinmachen, die den tatsächlichen PWM-Wert überwacht und ggf. korrigiert
              //nur wenn Die Motor-RPM angeglichen ist: 
              if (directive == Directive::EngageClutch) {
                Serial.println("RevMatch completed, advancing shiftTimer");
                shiftTimer.advance();  
              }
              if (micros() - revmatchStartMicros > MAX_REVMATCH_TIMEOUT_MICROS) {
                Serial.println("RevMatch timeout, advancing shiftTimer");
                shiftTimer.advance();  
              }
              //shiftTimer.advance();  
            }
            break;  
            case 3: {
              int servoTargetus = desiredGear == Gear::First ? SERVO_MIDDLE_US - SERVO_TRAVEL_US : SERVO_MIDDLE_US + SERVO_TRAVEL_US;
              servo.writeMicroseconds(servoTargetus);
            }
            break; 
            case 4: {
              //Schaltvorgang als abgeschlossen markieren 
              finalizeShift(desiredGear);
              //currentGear = Gear::First; 
            }
            break;  
          }
        }
      }
};

#endif