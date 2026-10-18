#include <ESP32Servo.h>
#include <Arduino.h>
#include <ShiftEnginge.h>
//Plan: 
//Getriebe fertig bauen 
//Ein echtes Drivetrain-Objekt basteln und das Komplettgetriebe durchtesten 
//Poti an irgendeinen Pin als Input

constexpr uint8_t PIN_MOTOR_PWM_LEFT = 0; 
constexpr uint8_t PIN_MOTOR_DIRECTION_LEFT = 0;
constexpr uint8_t LEDC_CHANNEL_LEFT = 0; 
constexpr uint8_t PIN_PHOTOSENSOR_LEFT = 0; 
constexpr uint8_t PIN_HALLSENSOR_1_LEFT = 0; 
constexpr uint8_t PIN_HALLSENSOR_2_LEFT = 0; 


constexpr uint8_t PIN_MOTOR_PWM_RIGHT = 0; 
constexpr uint8_t PIN_MOTOR_DIRECTION_RIGHT = 0;
constexpr uint8_t LEDC_CHANNEL_RIGHT = 0; 
constexpr uint8_t PIN_PHOTOSENSOR_RIGHT = 0; 
constexpr uint8_t PIN_HALLSENSOR_1_RIGHT = 0; 
constexpr uint8_t PIN_HALLSENSOR_2_RIGHT = 0; 

constexpr uint8_t PIN_SHIFT_SERVO = 0; 

Transmission leftTrans(PIN_MOTOR_PWM_LEFT, PIN_MOTOR_DIRECTION_LEFT, LEDC_CHANNEL_LEFT, PIN_PHOTOSENSOR_LEFT, PIN_HALLSENSOR_1_LEFT, PIN_HALLSENSOR_2_LEFT);
Transmission rightTrans(PIN_MOTOR_PWM_RIGHT, PIN_MOTOR_DIRECTION_RIGHT, LEDC_CHANNEL_RIGHT, PIN_PHOTOSENSOR_RIGHT, PIN_HALLSENSOR_1_RIGHT, PIN_HALLSENSOR_2_RIGHT); 
Drivetrain drivetrain(leftTrans, rightTrans, PIN_SHIFT_SERVO); 

void setup() {
    Serial.begin(115200); 
    Serial.println("Setup - Start");
   
    drivetrain.init(); 
    drivetrain.calibrate(); 

    Serial.println("Setup - End"); 
}

void loop() { 
    static uint16_t poti_input   ;
    //drivetrain.update(, 512); 
}