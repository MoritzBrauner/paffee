#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class Timer {
    private:
        unsigned long interval; 
        unsigned long lastTick;  
    public: 
        Timer(unsigned long ms); 
        bool fires(); 
};

class StagedTimer: public Timer {
    private: 
        uint8_t stages;  
        uint8_t currentStage; 
    public: 
        StagedTimer(unsigned long ms, uint8_t stages);
        uint8_t getStage(); 
};

//class TimedCounter: public Timer {
//    private:
//        
//    public:
//        TimedCounter(unsigned long ms, uint8_t stages);
//        uint8_t getStage(); 
//        void reset(); 
//};

#endif