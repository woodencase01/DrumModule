#pragma once
#include <Arduino.h>

class drumModule
{
public:
    drumModule();
    void init(uint8_t _pin);
    void setMinThreshold(uint8_t _minThreshold);
    void setMaxThreshold(uint8_t _maxThreshold);
    void setDecay(uint8_t _decay);
    void calibrate();
    void manage();
    bool isStruck();
    float getHit();
    void setPadSound(const unsigned int *_padSound);
    const unsigned int *getPadSound();

private:
    void resetDrumVariables();

    uint8_t pin, currentDrumState, decendingCounter, decay;
    uint16_t currentRaw, maxCurrentRaw, minThreshold, maxThreshold;
    bool isCalibrating, hasStruck;
    uint32_t calibrationEnd, decayEnd;
    float stroke;
    const unsigned int *padSound;

    enum drumState
    {
        drumIdle,
        drumStroke,
        drumDecay
    };

};