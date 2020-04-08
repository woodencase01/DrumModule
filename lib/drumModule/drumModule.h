#pragma once
#include <Arduino.h>

class drumModule
{
public:
    drumModule();
    void init(uint8_t _pin);
    void setThreshold(uint8_t _threshold);
    void setDecay(uint8_t _decay);
    void calibrate();
    void manage();
    bool isStruck();
    float getHit();

private:
    void resetDrumVariables();

    uint8_t pin, currentDrumState, decendingCounter, decay;
    uint16_t currentRaw, maxCurrentRaw, threshold, maxRaw;
    bool isCalibrating, hasStruck;
    uint32_t calibrationEnd, decayEnd;
    float stroke;

    enum drumState
    {
        drumIdle,
        drumStroke,
        drumDecay
    };

};