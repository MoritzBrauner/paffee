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
    protected:
        uint8_t stages;  
        uint8_t currentStage; 
    public: 
        StagedTimer(unsigned long ms, uint8_t stages);
        uint8_t getStage(); 
};

class SingleUseStagedTimer: public StagedTimer {
    private:
        bool enabled;  
    public:
        SingleUseStagedTimer(unsigned long ms, uint8_t stages, bool enabled = true);
        uint8_t getStage(); 
        void reset(bool start); 
        //void disable(); 
        bool isEnabled(); 
};

#endif