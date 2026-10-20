#include <ESP32Servo.h>
#include <Arduino.h>
#include <ShiftEnginge.h>
#include "pins.h"
#include <math.h>
//Plan: 
//Getriebe fertig bauen 
//Ein echtes Drivetrain-Objekt basteln und das Komplettgetriebe durchtesten 
//Poti an irgendeinen Pin als Input

Transmission leftTrans(PIN_MOTOR_PWM_LEFT, PIN_MOTOR_DIRECTION_LEFT, LEDC_CHANNEL_LEFT, PIN_PHOTOSENSOR_LEFT, PIN_HALLSENSOR_1_LEFT, PIN_HALLSENSOR_2_LEFT);
Transmission rightTrans(PIN_MOTOR_PWM_RIGHT, PIN_MOTOR_DIRECTION_RIGHT, LEDC_CHANNEL_RIGHT, PIN_PHOTOSENSOR_RIGHT, PIN_HALLSENSOR_1_RIGHT, PIN_HALLSENSOR_2_RIGHT); 
Drivetrain drivetrain(leftTrans, rightTrans, PIN_SHIFT_SERVO); 

//Pin 36, eigentlich Akkuspannung, aber für den Test i.O. 
constexpr uint8_t PIN_POTI = 36; 

void setup() {
    Serial.begin(115200); 
    Serial.println("Setup - Start");

    drivetrain.init(); 
    drivetrain.calibrate(); 

    pinMode(PIN_POTI, INPUT); 

    Serial.println("Setup - End"); 

    while (getPotiValue() >= 563) { 
      Serial.println("Plt set poti to 0.");  
      delay(100);
    }
}

uint16_t getPotiValue() {
    uint16_t poti_input = analogRead(PIN_POTI); 
    return map(poti_input, 0, 4096, 512, 1023); 
}

void loop() { 
    static Timer printTimer(200); 
    uint16_t potiValue = getPotiValue(); 
    drivetrain.update(512, potiValue); 
    if (printTimer.fires()) {
      const RotationSpeeds l = leftTrans.getRotationalSpeeds();
      const RotationSpeeds r = rightTrans.getRotationalSpeeds();
      Serial.printf("Poti: %4u    Links Motor: %6.0f  Abtrieb: %5.0f    Rechts Motor: %6.0f  Abtrieb: %5.0f rpm\n",           
        potiValue, l.input, l.output, r.input, r.output);
    }
}