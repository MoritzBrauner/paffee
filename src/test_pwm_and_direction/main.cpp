#include <Arduino.h>

//Litle Test-Program to test PWM and direction control of the motor driver for 1 motor 

constexpr uint8_t CHANNEL_MOTOR = 0;
constexpr uint16_t MOTOR_PWM_FREQUENCY = 20000;

constexpr uint8_t PIN_MOTOR_PWM = 25;
constexpr uint8_t PIN_MOTOR_DIRECTION = 26;

constexpr uint16_t STEP_DELAY_MS = 100;

void setup() {
  Serial.begin(115200);
  Serial.println("Setup - Start");
  Serial.println("Programm: Test für PWM und Richtung");
  pinMode(PIN_MOTOR_DIRECTION, OUTPUT);
  ledcSetup(CHANNEL_MOTOR, MOTOR_PWM_FREQUENCY, 10);
  ledcAttachPin(PIN_MOTOR_PWM, CHANNEL_MOTOR);
  Serial.println("Setup - End");
}

//direction true/false is enough for the test 
bool direction = true;

void loop() {
  Serial.printf("Richtung: %s\n", direction ? "Vorwärts" : "Rückwärts");
  digitalWrite(PIN_MOTOR_DIRECTION, direction ? HIGH : LOW);

  for (int pwm = 0; pwm < 1000; pwm += 100) {
    Serial.printf("PWM: %d\n", pwm);
    ledcWrite(CHANNEL_MOTOR, pwm);
    delay(STEP_DELAY_MS);
  }

  for (int pwm = 1000; pwm > 0; pwm -= 100) {
    Serial.printf("PWM: %d\n", pwm);
    ledcWrite(CHANNEL_MOTOR, pwm);
    delay(STEP_DELAY_MS);
  }

  direction = !direction;
}