#include "Timer.h"

Timer::Timer(unsigned long ms) {
    interval = ms; 
    lastTick = millis(); 
}

bool Timer::fires() {
    unsigned long now = millis(); 
    if (now - lastTick >= interval) {
        lastTick = now;
        return true;  
    }
    return false; 
}

StagedTimer::StagedTimer(unsigned long ms, uint8_t stages): Timer(ms) {
    this->currentStage = 1; 
    this->stages = stages; 
}

uint8_t StagedTimer::getStage() {
    if (fires()) currentStage ++; 
    if (currentStage > stages) currentStage = 1; 
    return currentStage;  
}
