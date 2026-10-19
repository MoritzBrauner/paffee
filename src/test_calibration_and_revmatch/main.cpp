#include <Arduino.h>
#include <ESP32Encoder.h>
#include <Timer.h>

constexpr float GEAR_RATIO_FIRST = 13.0f / 47;
constexpr float GEAR_RATIO_SECOND = 28.0f / 32;

constexpr uint8_t CHANNEL_MOTOR = 0;
constexpr uint16_t MOTOR_PWM_FREQUENCY = 20000;

//Testaufbau: alle Signale liegen nebeneinander auf der linken Stiftleiste des ESP32-DevKitC
//(Antenne oben): 3V3, EN, 36, 39, 34, 35, [32, 33, 25, 26, 27, 14], 12, GND, 13, ...
//  32/33  Encoder A/B      Encoder an 3V3, nicht 5V
//  25     Motor-PWM        10 kOhm Pull-down nach GND empfohlen (Motor steht beim Reset)
//  26     Motor-DIR
//  27     Lichtschranke    Modul an 3V3, die GPIOs sind nicht 5-V-fest
//  14     Gang-Schalter    gegen GND, interner Pull-up; offen = 1. Gang, geschlossen = 2. Gang
//Diese Pins sind nur fuer den Pruefstand. Die finale Belegung steht in doku/08-esp32-pinbelegung.md, dort
//auch, wie der Pruefstand zum Getriebe rechts wird.
constexpr uint8_t PIN_HALL_SENSOR_1 = 32;
constexpr uint8_t PIN_HALL_SENSOR_2 = 33;
constexpr uint8_t PIN_MOTOR_PWM = 25;
constexpr uint8_t PIN_MOTOR_DIRECTION = 26;
constexpr uint8_t PIN_PHOTOELECTRIC_SENSOR = 27;
constexpr uint8_t PIN_GEAR_INPUT = 14;

constexpr uint16_t CALIBRATION_SAMPLE_INTERVAL_MS = 100;
constexpr uint16_t CALIBRATION_SETTLE_MS = 500;
constexpr float CALIBRATION_MIN_RPM_AT_PWM_1000 = 500.0f;

constexpr unsigned long HALL_SAMPLE_INTERVAL_MS = 50;
constexpr float countsPerRev = 275.0f; 
Timer hallTimer = Timer(HALL_SAMPLE_INTERVAL_MS);
ESP32Encoder encoder; 
int64_t lastCount = 0; 
float lastMeasuredInputRpm = 0; 

struct RPMs {
    float input = 0;
    float output = 0; 
    //RPMs(const float input, const float output) : input(input), output(output) {}
};
RPMs rpms; 

struct Lichtschranke {
    volatile unsigned long lastTickMicros = 0; 
    volatile unsigned long lastPeriodMicros = 0;
}; 
Lichtschranke lichtschranke;

static void IRAM_ATTR onSensorChangeISR() {
    const unsigned long microsNow = micros();
    lichtschranke.lastPeriodMicros = microsNow - lichtschranke.lastTickMicros; 
    lichtschranke.lastTickMicros = microsNow; 
}

//Gibt zurueck, ob ein neuer Encoder-Messwert vorliegt (alle HALL_SAMPLE_INTERVAL_MS)
bool updateRotationalSpeeds() {
      bool newInputSample = false;
      //input
      if (hallTimer.fires()) {
        const int64_t now =  encoder.getCount();
        const long delta = now - lastCount;
        lastCount = now;
        rpms.input = delta * 60000.0f / (HALL_SAMPLE_INTERVAL_MS * countsPerRev);
        newInputSample = true;
      }
      //output
      //Erst den Zeitstempel kopieren, dann micros() lesen: Kommt die ISR dazwischen, wäre lastTickMicros sonst neuer als microsNow und die Differenz liefe über.
      const unsigned long lastTick = lichtschranke.lastTickMicros;
      const unsigned long microsNow = micros();
      const unsigned long sinceLastTick = microsNow - lastTick;
      const unsigned long lastPeriod = lichtschranke.lastPeriodMicros;
      //Idee dahinter: Wenn der letzte Tick länger her ist, als die Länge der letzten Periode,
      //nehmen wir die Differenz zwischen jetzt und dem letzten Tick zur Berechnung
      //Grund: vorher war fester Timeout von 1sek -> Drehzahl sprang womöglich von X direkt auf 0.
      //Jetzt ist es smoooooth
      if (lastPeriod == 0 || sinceLastTick > 1000UL * 1000) {
        rpms.output = 0;
      } else {
        const unsigned long effectivePeriod = std::max(lastPeriod, sinceLastTick);
        rpms.output = 60000000.0f / effectivePeriod / 32;
      }
      return newInputSample;
}

float getMotorRpmBlocking(unsigned int sampleIntervalMs) {
      const int64_t startCount = encoder.getCount();
      const unsigned long startMicros = micros();
      delay(sampleIntervalMs);
      const int64_t deltaCount = encoder.getCount() - startCount;
      const unsigned long deltaMicros = micros() - startMicros;
      return (deltaCount * 60000000.0f) / (deltaMicros * countsPerRev);
}

float calibratedMotorRPMs[11] = {0}; //Array to hold the calibrated motor RPMs for PWM values 0, 100, 200, ..., 1000
bool calibrateMotor() {
    Serial.println("Motor calibration started...");
    for (int i = 0; i <= 10; i++) {
        ledcWrite(CHANNEL_MOTOR, i * 100);
        delay(CALIBRATION_SETTLE_MS);
        calibratedMotorRPMs[i] = getMotorRpmBlocking(CALIBRATION_SAMPLE_INTERVAL_MS);
        Serial.printf("Measured Motor RPM for PWM %d: %f\n", i * 100, calibratedMotorRPMs[i]);
    }
    ledcWrite(CHANNEL_MOTOR, 0);
    ledcAttachPin(PIN_MOTOR_PWM, CHANNEL_MOTOR);
    delay(CALIBRATION_SETTLE_MS);
    lastCount = encoder.getCount();
    Serial.println("Motor calibration finished. Now sanity-checking measured values...");
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
    return true;
}

float getTargetPwmByRpm(const float rpm) {
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

//Messausgabe alle 100 ms, damit das Einschwingen sichtbar ist
Timer printTimer(100);

//Messung der Rev-Match-Dauer: Gang-Schalter umlegen, waehrend der Abtrieb dreht
constexpr float REVMATCH_TOLERANCE_RPM = 20.0f;             //wie REVMATCH_TOLERANCE_RPM in ShiftEnginge.h
constexpr uint8_t REVMATCH_STABLE_SAMPLES = 3;              //so viele Encoder-Messungen in Folge innerhalb der Toleranz
constexpr unsigned long REVMATCH_MEASURE_TIMEOUT_MS = 3000;
constexpr unsigned long GEAR_SWITCH_DEBOUNCE_MS = 20;

bool firstGearSelected = true;      //entprellter Zustand des Gang-Schalters
bool rawGearReading = true;
unsigned long rawGearChangedMs = 0;

bool revmatchMeasuring = false;
unsigned long revmatchStartMs = 0;
unsigned long toleranceStreakStartMs = 0;
uint8_t samplesInTolerance = 0;
//-----------------------------------------------------------------------------------------------------
//SETUP SETUP SETUP SETUP SETUP SETUP SETUP SETUP SETUP SETUP SETUP SETUP SETUP SETUP SETUP SETUP SETUP 
//-----------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Setup - Start");
  Serial.println("Program: Calibration and RevMatch Test");

  ledcSetup(CHANNEL_MOTOR, MOTOR_PWM_FREQUENCY, 10);
  ledcAttachPin(PIN_MOTOR_PWM, CHANNEL_MOTOR);
  
  pinMode(PIN_GEAR_INPUT, INPUT_PULLUP);

  pinMode(PIN_PHOTOELECTRIC_SENSOR, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIN_PHOTOELECTRIC_SENSOR), onSensorChangeISR, RISING); 
  
  pinMode(PIN_MOTOR_DIRECTION, OUTPUT);
  Serial.println("Setting motor direction to LOW (forward)");
  digitalWrite(PIN_MOTOR_DIRECTION, LOW);

  ESP32Encoder::useInternalWeakPullResistors = UP;  
  encoder.attachFullQuad(PIN_HALL_SENSOR_1, PIN_HALL_SENSOR_2);
  encoder.clearCount();

  if(!calibrateMotor()) {
    while(true) {
      Serial.println("Motor calibration failed");
      delay(5000);
    }
  };

  firstGearSelected = rawGearReading = digitalRead(PIN_GEAR_INPUT) == HIGH;

  Serial.println("Setup - End");
}

//----------------------------------------------------------------------------------------------------
//LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP LOOP  
//----------------------------------------------------------------------------------------------------

void loop() {
    const unsigned long nowMs = millis();

    //Gang-Schalter entprellen: neuer Zustand gilt erst, wenn er GEAR_SWITCH_DEBOUNCE_MS stabil anliegt
    const bool reading = digitalRead(PIN_GEAR_INPUT) == HIGH;
    if (reading != rawGearReading) {
        rawGearReading = reading;
        rawGearChangedMs = nowMs;
    }
    if (rawGearReading != firstGearSelected && nowMs - rawGearChangedMs >= GEAR_SWITCH_DEBOUNCE_MS) {
        firstGearSelected = rawGearReading;
        Serial.printf("# Zielgang %d gewaehlt\n", firstGearSelected ? 1 : 2);
        //Nur messen, wenn der Motor wirklich nachziehen muss - sonst kaeme sofort ein Scheinwert
        const float newTargetInputRpm = rpms.output * (1/(firstGearSelected ? GEAR_RATIO_FIRST : GEAR_RATIO_SECOND));
        revmatchMeasuring = false;
        if (rpms.output == 0) {
            Serial.println("# Abtrieb steht - keine Rev-Match-Messung");
        } else if (fabsf(fabsf(rpms.input) - newTargetInputRpm) <= REVMATCH_TOLERANCE_RPM) {
            Serial.println("# schon in Toleranz - keine Rev-Match-Messung");
        } else {
            revmatchMeasuring = true;
            revmatchStartMs = nowMs;
            samplesInTolerance = 0;
        }
    }
    const float targetRatio = firstGearSelected ? GEAR_RATIO_FIRST : GEAR_RATIO_SECOND;

    //erst messen, dann stellen
    const bool newInputSample = updateRotationalSpeeds();
    const float targetInputRpm = rpms.output * (1/targetRatio);
    const uint32_t targetPwm = getTargetPwmByRpm(targetInputRpm);
    ledcWrite(CHANNEL_MOTOR, targetPwm);

    //Vorzeichen des Encoders ist nur die Zaehlrichtung -> Betrag vergleichen
    const float deviation = fabsf(rpms.input) - targetInputRpm;
    const bool inTolerance = fabsf(deviation) <= REVMATCH_TOLERANCE_RPM;

    if (revmatchMeasuring) {
        if (newInputSample) {
            if (!inTolerance) {
                samplesInTolerance = 0;
            } else if (samplesInTolerance++ == 0) {
                //Der Encoder-Wert mittelt ueber das letzte Messfenster -> Fenstermitte als Zeitpunkt
                toleranceStreakStartMs = std::max(nowMs - HALL_SAMPLE_INTERVAL_MS / 2, revmatchStartMs);
            }
        }
        if (samplesInTolerance >= REVMATCH_STABLE_SAMPLES) {
            //Genauigkeit etwa -10 bis +45 ms, weil der Encoder-Wert nur alle HALL_SAMPLE_INTERVAL_MS kommt
            Serial.printf("# Rev-Match nach %lu ms (+-%.0f rpm)\n", toleranceStreakStartMs - revmatchStartMs, REVMATCH_TOLERANCE_RPM);
            revmatchMeasuring = false;
        } else if (nowMs - revmatchStartMs > REVMATCH_MEASURE_TIMEOUT_MS) {
            Serial.printf("# Rev-Match nach %lu ms nicht erreicht\n", REVMATCH_MEASURE_TIMEOUT_MS);
            revmatchMeasuring = false;
        }
    }

    if (printTimer.fires()) {
        //Feste Feldbreiten, damit die Spalten im seriellen Monitor untereinander stehen
        Serial.printf("Abtrieb: %7.1f rpm    Soll Motor: %7.1f rpm    Ist Motor: %7.1f rpm    Abweichung: %+7.1f rpm    PWM: %4lu    %s\n",
                      rpms.output, targetInputRpm, rpms.input, deviation, (unsigned long) targetPwm, inTolerance ? "in Toleranz" : "");
    }
}