#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class Timer {
    protected:
        unsigned long interval;   // in Mikrosekunden
        unsigned long lastTick;   // micros()-Zeitstempel
    public:
        Timer(unsigned long ms);  // Intervall weiterhin in Millisekunden, intern * 1000
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
        bool paused; 
    public:
        SingleUseStagedTimer(unsigned long ms, uint8_t stages, bool enabled = true);
        uint8_t getStage(); 
        void reset(bool start); 
        //void disable(); 
        bool isEnabled();
        void pause();
        void advance();  
};

#endif