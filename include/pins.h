#ifndef PAFFEE_PINS_H
#define PAFFEE_PINS_H

#include <stdint.h>

//Belegung laut doku/08-esp32-pinbelegung.md. Bei Aenderungen beides anpassen.

//Getriebe links
constexpr uint8_t PIN_MOTOR_PWM_LEFT = 33;
constexpr uint8_t PIN_MOTOR_DIRECTION_LEFT = 32;
constexpr uint8_t LEDC_CHANNEL_LEFT = 2;
constexpr uint8_t PIN_PHOTOSENSOR_LEFT = 35;   //nur Eingang, Pull-up muss auf dem Modul sitzen
constexpr uint8_t PIN_HALLSENSOR_1_LEFT = 39;  //nur Eingang, Pull-up muss auf dem Modul sitzen
constexpr uint8_t PIN_HALLSENSOR_2_LEFT = 34;  //nur Eingang, Pull-up muss auf dem Modul sitzen

//Getriebe rechts
constexpr uint8_t PIN_MOTOR_PWM_RIGHT = 12;    //Strapping-Pin: 10k Pull-down Pflicht
constexpr uint8_t PIN_MOTOR_DIRECTION_RIGHT = 14;
constexpr uint8_t LEDC_CHANNEL_RIGHT = 3;
constexpr uint8_t PIN_PHOTOSENSOR_RIGHT = 27;
constexpr uint8_t PIN_HALLSENSOR_1_RIGHT = 25;
constexpr uint8_t PIN_HALLSENSOR_2_RIGHT = 26;

//Schaltung
constexpr uint8_t PIN_SHIFT_SERVO = 13;

//Akku
constexpr uint8_t PIN_BATTERY_VOLTAGE = 36;    //ADC1, geht auch mit WLAN

//Turm
constexpr uint8_t PIN_ELEVATION_SERVO = 21;
constexpr uint8_t PIN_TURRET_ENN = 0;          //Strapping-Pin: erst HIGH schreiben, dann OUTPUT_OPEN_DRAIN
constexpr uint8_t PIN_TURRET_STEP = 2;
constexpr uint8_t PIN_TURRET_DIRECTION = 15;
constexpr uint8_t LEDC_CHANNEL_TURRET_STEP = 4;

//BB-Einheit
constexpr uint8_t PIN_BB_SWITCH = 23;
constexpr uint8_t PIN_BB_GATE = 22;
constexpr uint8_t LEDC_CHANNEL_BB = 6;

//Funk nRF24: SPI.begin(PIN_NRF_SCK, PIN_NRF_MISO, PIN_NRF_MOSI, PIN_NRF_CSN) vor radio.begin()
constexpr uint8_t PIN_NRF_MISO = 19;
constexpr uint8_t PIN_NRF_SCK = 18;
constexpr uint8_t PIN_NRF_CSN = 5;
constexpr uint8_t PIN_NRF_MOSI = 17;
constexpr uint8_t PIN_NRF_CE = 16;
constexpr uint8_t PIN_NRF_IRQ = 4;             //Pegel lesen, kein Flanken-Interrupt

#endif // PAFFEE_PINS_H
