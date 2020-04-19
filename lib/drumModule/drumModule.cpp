#include "drumModule.h"

#define DEBUG

static const uint16_t CALIBRATION_DELAY = 2000;
static const uint16_t MAX_DECENDING_COUNTERS = 5;

elapsedMicros lastCheck;
static const uint16_t CHECK_DELAY = 200; // Micro-seconds, 1640 Hz

drumModule::drumModule()
{ 
    minThreshold = 10;
    maxThreshold = 300;
    decay = 20;
}

void drumModule::init(uint8_t _pin)
{
    pin = _pin;
    currentDrumState = drumIdle;
}

void drumModule::setMinThreshold(uint8_t _minThreshold)
{
    minThreshold = _minThreshold;
}

void drumModule::setMaxThreshold(uint8_t _maxThreshold)
{
    maxThreshold = _maxThreshold;
}

void drumModule::setDecay(uint8_t _decay)
{
    decay = _decay;
}

void drumModule::calibrate()
{
    isCalibrating = true;
    calibrationEnd = millis() + CALIBRATION_DELAY;
}

void drumModule::manage()
{
    if (lastCheck > CHECK_DELAY)
    {
        currentRaw = analogRead(pin);

        switch (currentDrumState)
        {
        case drumIdle:

            if (currentRaw > minThreshold)
            {
                maxCurrentRaw = currentRaw;
                currentDrumState = drumStroke;
#ifdef DEBUG
                Serial.print(millis());
                Serial.print(F(", Stroke at : "));
                Serial.println(currentRaw);
#endif
            }
            break;

        case drumStroke:

            if (currentRaw > maxCurrentRaw)
            {
                maxCurrentRaw = currentRaw;
            }
            else
            {
                if (decendingCounter > MAX_DECENDING_COUNTERS)
                {

                    currentDrumState = drumDecay;
                    stroke = ((float)maxCurrentRaw - minThreshold) / (maxThreshold - minThreshold);
                    if (stroke > 1.0f)
                        stroke = 1.0f;
                    
                    

                    hasStruck = true;
                    decayEnd = millis() + decay;

#ifdef DEBUG
                    Serial.print(millis());
                    Serial.print(F(", Out: "));
                    Serial.println(stroke, 3);
#endif
                }
                else
                {
                    decendingCounter++;
                }
            }

            break;

        case drumDecay:
            if (millis() > decayEnd)
            {
#ifdef DEBUG
                Serial.print(millis());
                Serial.println(F(", Idle"));
#endif
                currentDrumState = drumIdle;
            }
            break;
        }
        if (isCalibrating)
        {
            // To-Do
            isCalibrating = false;
        }

        lastCheck = 0;
    }
}

bool drumModule::isStruck()
{
    return hasStruck;
}

float drumModule::getHit()
{
    hasStruck = false;
    maxCurrentRaw = 0;

    return stroke;
}

void drumModule::setPadSound(const unsigned int *_padSound)
{
    padSound = _padSound;
}

const unsigned int *drumModule::getPadSound()
{
    return padSound;
}