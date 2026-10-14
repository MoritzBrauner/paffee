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

constexpr unsigned long SHIFT_TIMER_INTERVAL = 200; 

//constexpr int SAMPLE_INTERVAL_MS = 50;
constexpr int MOTOR_PWM_FREQUENCY = 20000; //Check later for Correctness 
constexpr int MOTOR_PWM_RESOLUTION_BITS = 10;

constexpr unsigned long MAX_REVMATCH_TIMEOUT_MICROS = 2000UL * 1000;

constexpr float CALIBRATION_MIN_RPM_AT_PWM_1000 = 500.0f;

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
  Messages(const Message left, const Message right) : left(left), right(right) {}
  bool areEqual() const { return (left == right);}
  Directive getDirective() const {
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

  RotationSpeeds(const float input, const float output) : input(input), output(output) {}
};

class Transmission {
  private:
    bool initialized = false;
    bool calibrated = false;

    float calibratedMotorRPMs[11] = {0};
    constexpr static uint16_t CALIBRATION_SETTLE_MS = 500;
    constexpr static uint16_t CALIBRATION_SAMPLE_INTERVAL_MS = 100;
    constexpr static uint16_t REVMATCH_TOLERANCE_RPM = 20;

    Message message = Message::Happy;

    Mode mode = Mode::Drive;
    
    Gear targetGear = Gear::First;
    float targetRatio = GEAR_RATIO_FIRST;
    Gear currentGear = Gear::First;
    float currentRatio = GEAR_RATIO_FIRST;

    uint8_t pin_motorPwm;
    uint8_t pin_motorDirection;
    uint8_t ledCChannel; 

    uint8_t pin_photoElectricSpeedSensor; 
    volatile unsigned long lastTickMicros = 0; 
    volatile unsigned long lastPeriodMicros = 0; 
    float lastMeasuredOutputRpm = 0;

    uint8_t pin_hallSensor1; 
    uint8_t pin_hallSensor2; 
    Timer hallTimer = Timer(HALL_SAMPLE_INTERVAL_MS);
    static constexpr float countsPerRev = 275.0f; 
    static constexpr unsigned long HALL_SAMPLE_INTERVAL_MS = 50;
    ESP32Encoder encoder; 
    int64_t lastCount = 0; 
    float lastMeasuredInputRpm = 0; 
    
    void write(const uint16_t pwm, const Direction direction) const {
      //Wir nehmen PWM-Werte von 0..1023 
      ledcWrite(ledCChannel, pwm); 
      digitalWrite(pin_motorDirection, direction == Direction::Forward ? LOW : HIGH); 
    }

    float getTargetPwmByRpm(const float rpm) const {
      //Interpolate between calibratedMotorRPMs and their corresponding PWM values 
      //to find the PWM value that corresponds to the desired RPM 
      for (int i = 0; (i + 1) < std::size(calibratedMotorRPMs); i++) {
        if (rpm >= calibratedMotorRPMs[i] && rpm <= calibratedMotorRPMs[i+1]) {
          const float pwm1 = i * 100;
          const float pwm2 = (i+1) * 100;
          const float rpm1 = calibratedMotorRPMs[i];
          const float rpm2 = calibratedMotorRPMs[i+1];
          if (rpm2 - rpm1 <= 0) return pwm1;
          const float pwm = pwm1 + (rpm - rpm1) * (pwm2 - pwm1) / (rpm2 - rpm1);
          return pwm;
        }
      }
      //Ziel-Drehzahl ueber dem Kalibrierbereich -> Vollgas statt Motor aus
      if (rpm > calibratedMotorRPMs[10]) return 1023;
      return 0;
    }

    static void IRAM_ATTR onSensorChangeISR(void* arg) {
      static_cast<Transmission*>(arg)->onSensorChange();
    }

    void IRAM_ATTR onSensorChange() {
      const unsigned long microsNow = micros();
      lastPeriodMicros = microsNow - lastTickMicros; 
      lastTickMicros = microsNow; 
    }

    //Helper Function to get inputRpm. Blocking. 
    //We need this seperate function because we cannot use getRotationalSpeeds() in the calibration process, 
    //because it uses the lastMeasuredInputRpm which is only updated every HALL_SAMPLE_INTERVAL_MS. 
    //DO NOT CALL APART FROM CALIBRATE()
    float getInputRpmBlocking(const uint16_t measureIntervalMs) {
      const int64_t startCount = encoder.getCount();
      const unsigned long startMicros = micros();
      delay(measureIntervalMs);
      const int64_t deltaCount = encoder.getCount() - startCount;
      const unsigned long deltaMicros = micros() - startMicros;
      return (deltaCount * 60000000.0f) / (deltaMicros * countsPerRev);
    } 

  public:
    Transmission(const uint8_t pin_motorPwm, const uint8_t pin_motorDirection, const uint8_t ledCChannel, const uint8_t pin_photoElectricSpeedSensor, const uint8_t pin_hallSensor1, const uint8_t pin_hallSensor2):
      pin_motorPwm(pin_motorPwm), pin_motorDirection(pin_motorDirection), ledCChannel(ledCChannel), pin_photoElectricSpeedSensor(pin_photoElectricSpeedSensor), pin_hallSensor1(pin_hallSensor1), pin_hallSensor2(pin_hallSensor2) {
    }

    void init() {
      Serial.println("Initializing Transmission...");
      //Set Pin Modes
      pinMode(pin_motorPwm, OUTPUT); 
      pinMode(pin_motorDirection, OUTPUT); 
      pinMode(pin_photoElectricSpeedSensor, INPUT_PULLUP); 
      pinMode(pin_hallSensor1, INPUT_PULLUP); 
      pinMode(pin_hallSensor2, INPUT_PULLUP); 
      //Atach ISR for Optical Sensor
      attachInterruptArg(digitalPinToInterrupt(pin_photoElectricSpeedSensor), onSensorChangeISR, this, RISING); 
      //Attach Encoder
      ESP32Encoder::useInternalWeakPullResistors = UP;
      encoder.attachFullQuad(pin_hallSensor1, pin_hallSensor2); 
      encoder.clearCount(); 
      //Handle LedC API 
      ledcSetup(ledCChannel, MOTOR_PWM_FREQUENCY, MOTOR_PWM_RESOLUTION_BITS);
      ledcAttachPin(pin_motorPwm, ledCChannel);
      //Finalize
      this->initialized = true;
      Serial.println("Initialization completed."); 
    }

    //Calibrate Motor Speeds: 
    //Set Motor to a PWM Value an see how fast it spins at that speed 
    bool calibrate() {
        if (!initialized) {
          Serial.println("Unable to calibrate. Transmission ist not initialized");
          return false;
        }
        Serial.println("Transmission calibration started...");
        for (int i = 0; i <= 10; i++) {
          write(i * 100, Direction::Forward);
          delay(CALIBRATION_SETTLE_MS);
          calibratedMotorRPMs[i] = getInputRpmBlocking(CALIBRATION_SAMPLE_INTERVAL_MS); 
          Serial.printf("Measured Motor RPM for PWM %d: %f\n", i * 100, calibratedMotorRPMs[i]);
        }
        write(0, Direction::Forward); 
        delay(CALIBRATION_SETTLE_MS);
        this->lastCount = encoder.getCount();
        Serial.println("Transmission calibration finished. Now sanity-checking measured values...");
        //Little Sanity Check of the values:
        //If the check fails, the calibration is marked faulty
        if (calibratedMotorRPMs[0] != 0) {
          Serial.printf("Calibration Error: Motor RPM for PWM 0 (%f) is not 0\n", calibratedMotorRPMs[0]);
          return false;
        }
        if (calibratedMotorRPMs[10] < CALIBRATION_MIN_RPM_AT_PWM_1000) {
          Serial.printf("Calibration Error: Motor RPM for PWM 1000 (%f) is below the minimum threshold of %f\n", calibratedMotorRPMs[10], CALIBRATION_MIN_RPM_AT_PWM_1000);
          return false;
        }
        for (int i = 0; i < 10; i++) {
          if (calibratedMotorRPMs[i] > calibratedMotorRPMs[i+1]) {
            Serial.printf("Calibration Error: Motor RPM for PWM %d (%f) is higher than for PWM %d (%f)\n", i * 100, calibratedMotorRPMs[i], (i+1) * 100, calibratedMotorRPMs[i+1]);
            return false;
          }
        }
        Serial.println("Sanity Check passed. Transmission is calibrated.");
        this->calibrated = true;
        return true;
    }

    Message getMessage() const {
      return this->message;
    }

    void setMode(const Mode m) {
      this->mode = m;
      //clear message 
      this->message = Message::Happy; 
    }

    void setTargetGear(const Gear gear) {
      this->targetGear = gear; 
      this->targetRatio = gear == Gear::First ? GEAR_RATIO_FIRST : GEAR_RATIO_SECOND;
    }

    void setCurrentGear(const Gear gear) {
      this->currentGear = gear; 
      this->currentRatio = gear == Gear::First ? GEAR_RATIO_FIRST : GEAR_RATIO_SECOND;
    }

    RotationSpeeds getRotationalSpeeds() {
      //input
      if (hallTimer.fires()) {
        const int64_t now =  encoder.getCount();
        const long delta = now - lastCount;
        lastCount = now; 
        lastMeasuredInputRpm = delta * 60000.0f / (HALL_SAMPLE_INTERVAL_MS * countsPerRev);
      }
      //output
      //Erst den Zeitstempel kopieren, dann micros() lesen: Kommt die ISR dazwischen, wäre lastTickMicros sonst neuer als microsNow und die Differenz liefe über.
      const unsigned long lastTick = lastTickMicros;
      const unsigned long microsNow = micros();
      const unsigned long sinceLastTick = microsNow - lastTick;
      const unsigned long lastPeriod = lastPeriodMicros;
      //Idee dahinter: Wenn der letzte Tick länger her ist, als die Länge der letzten Periode,
      //nehmen wir die Differenz zwischen jetzt und dem letzten Tick zur Berechnung
      //Grund: vorher war fester Timeout von 1sek -> Drehzahl sprang womöglich von X direkt auf 0.
      //Jetzt ist es smoooooth
      if (lastPeriod == 0 || sinceLastTick > 1000UL * 1000) {
        lastMeasuredOutputRpm = 0;
      } else {
        const unsigned long effectivePeriod = std::max(lastPeriod, sinceLastTick);
        lastMeasuredOutputRpm = 60000000.0f / effectivePeriod / 32;
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
    void update(const uint16_t normalizedPwm, const Direction direction) {
        if (!initialized) {
            Serial.println("Unable to update. Transmission is not initialized");
            delay(1000);
            return;
        }
        if (!calibrated) {
            Serial.println("Unable to update. Transmission is not calibrated");
            delay(1000);
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
              } else {
                message = Message::Happy; 
              }
            }
            break;
            case Gear::Second: {
              if (rpms.output <= 350) {
                message = Message::RequestingShiftDown;
              } else {
                message = Message::Happy; 
              }
            }
            break;
            default: ;
          }
        } else {
          //ignore Remote Control Input, just RevMatch the output RPM to the targetGearRatio
          const float targetPwm = getTargetPwmByRpm(rpms.output * (1/targetRatio));
          write(targetPwm, direction);
          if (mode == Mode::RevMatch) {
            const bool rpmDifferenceIsTolerable =
              fabsf(rpms.input) >= rpms.output * (1/targetRatio) - REVMATCH_TOLERANCE_RPM &&
              fabsf(rpms.input) <= rpms.output * (1/targetRatio) + REVMATCH_TOLERANCE_RPM;
            if (rpmDifferenceIsTolerable) {
              message = Message::RevMatchCompleted; 
            } else {
              message = Message::Happy; 
            }
          }
        } 
    }
}; 

class Drivetrain {
    private:
      bool initialized = false;
      bool calibrated = false;

      Transmission& leftTrans; 
      Transmission& rightTrans; 

      SingleUseStagedTimer shiftTimer = SingleUseStagedTimer(SHIFT_TIMER_INTERVAL, 5, false);  

      Gear currentGear = Gear::Neutral;
      Gear desiredGear = Gear::Neutral;

      Servo servo; 
      uint8_t pin_servo; 

      unsigned long revmatchStartMicros = 0;

      Directive getTransMessages() const {
        return Messages(
          leftTrans.getMessage(), 
          rightTrans.getMessage() 
        ).getDirective();
      }

    public: 
      Drivetrain(Transmission& leftTrans, Transmission& rightTrans, const uint8_t pin_servo):
        leftTrans(leftTrans), rightTrans(rightTrans), pin_servo(pin_servo) {
      };

      void init() {
        //Handle Srevo Initialization:
        ESP32PWM::allocateTimer(0);
        servo.setPeriodHertz(50);  // analoger Servo -> 50 Hz. NICHT hochdrehen.
        servo.attach(pin_servo, 500, 2500);
        servo.writeMicroseconds(SERVO_MIDDLE_US);
        //Init both transeseses
        leftTrans.init(); 
        rightTrans.init();
        initialized = true;
      }
      
      //Calibrate both transesesees
      void calibrate() {
        if (!initialized) {
          Serial.println("Unable to calibrate. ShiftEnginge ist not initialized");
          return;
        }
        Serial.println("Calibrating Drivetrain..");
        servo.writeMicroseconds(SERVO_MIDDLE_US);
        delay(500);
        Serial.println("Left Transmission..");
        if(!leftTrans.calibrate()) {
          Serial.println("Left Transmission calibration failed. Aborting Drivetrain calibration.");
          return; 
        } 
        Serial.println("Right Transmission..");
        if(!rightTrans.calibrate()) {
          Serial.println("Right Transmission calibration failed. Aborting Drivetrain calibration.");
          return; 
        }
        delay(500);
        Serial.println("Drivetrain calibration finished. Shifting into first gear.");
        initializeGearShift(Gear::First);
        calibrated = true;
      }

      void initializeGearShift(const Gear gear) {
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

      void finalizeShift(const Gear gear) {
        currentGear = gear; 
        leftTrans.setCurrentGear(gear);
        rightTrans.setCurrentGear(gear);
        leftTrans.setMode(Mode::Drive);
        rightTrans.setMode(Mode::Drive); 
      }

      void update(uint8_t stickValueX, uint8_t stickValueY) {
        if (!initialized) {
          Serial.println("Unable to update. Drivetrain is not initialized");
          delay(1000);
          return;
        }
        if (!calibrated) {
          Serial.println("Unable to update. Drivetrain is not calibrated");
          delay(1000);
          return;
        }
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

        //update both transmissions with their respective PWM values and directions
        Direction direction = Direction::Forward; //placeholder for now, later we will get this from the remote control input
        uint16_t leftTransPwm = 0; //placeholder for now, later we will get this from the remote control input
        uint16_t rightTransPwm = 0; //placeholder for now, later we will get this from the remote control input
        leftTrans.update(leftTransPwm, direction);
        rightTrans.update(rightTransPwm, direction);

        const Directive directive = getTransMessages();
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
            default: ;
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
              initializeRevMatch();
              shiftTimer.advance(); 
            }
            break; 
            case 3: {
              shiftTimer.pause(); 
              //Motor im Leerlauf hochdrehen; revmatch
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
              } else  
              if (micros() - revmatchStartMicros > MAX_REVMATCH_TIMEOUT_MICROS) {
                Serial.println("RevMatch timeout, advancing shiftTimer");
                shiftTimer.advance();  
              }
              //shiftTimer.advance();  
            }
            break;  
            case 4: {
              const int servoTargetUs = desiredGear == Gear::First ? SERVO_MIDDLE_US - SERVO_TRAVEL_US : SERVO_MIDDLE_US + SERVO_TRAVEL_US;
              servo.writeMicroseconds(servoTargetUs);
            }
            break; 
            case 5: {
              //Schaltvorgang als abgeschlossen markieren 
              finalizeShift(desiredGear);
              //currentGear = Gear::First; 
            }
            break;
            default: ;
          }
        }
      }
};

#endif