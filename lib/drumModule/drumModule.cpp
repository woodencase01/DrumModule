#include "drumModule.h"

#define DEBUG

static const uint16_t CALIBRATION_DELAY = 2000;
static const uint16_t MAX_DECENDING_COUNTERS = 5;

elapsedMicros lastCheck;
static const uint16_t CHECK_DELAY = 200; // Micro-seconds, 1640 Hz

drumModule::drumModule()
{ 
    threshold = 10;
    maxRaw = 300;
    decay = 20;
}

void drumModule::init(uint8_t _pin)
{
    pin = _pin;
    currentDrumState = drumIdle;
}

void drumModule::setThreshold(uint8_t _threshold)
{
    threshold = _threshold;
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

            if (currentRaw > threshold)
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
                    stroke = ((float)maxCurrentRaw - threshold) / (maxRaw - threshold);
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