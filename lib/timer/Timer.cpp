#include "Timer.h"

Timer::Timer(unsigned long ms) {
    interval = ms * 1000UL;
    lastTick = micros();
}

bool Timer::fires() {
    unsigned long now = micros();
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

SingleUseStagedTimer::SingleUseStagedTimer(unsigned long ms, uint8_t stages, bool enabled): StagedTimer(ms, stages) {
    this->enabled = enabled; 
}

uint8_t SingleUseStagedTimer::getStage() {
    if (!isEnabled()) return 0;
    if (paused) return currentStage;  
    if (fires()) currentStage ++; 
    if (currentStage > stages) {
        this->enabled = false; 
        this->currentStage = 1; 
        return 0; 
    }
    return currentStage; 
}

void SingleUseStagedTimer::reset(bool start) {
    this->lastTick = micros();
    this->paused = false;  
    this->currentStage = 1; 
    if (start) this->enabled = true; 
} 
        //void disable(); 
bool SingleUseStagedTimer::isEnabled() {
    return this->enabled;
} 


void SingleUseStagedTimer::pause() {
    this->paused = true;
}

void SingleUseStagedTimer::advance() {
    this->paused = false; 
    this->getStage(); 
}  
