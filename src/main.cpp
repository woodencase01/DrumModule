#include <Arduino.h>

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SD_t3.h>
#include <SerialFlash.h>

#include <drumModule.h>

/*
Audio Data	  9, 11, 13, 22, 23	
Audio Control	18, 19	            SDA, SCL (other I2C chips)
Volume Pot	  15                  (A1)
SD Card	      7, 10, 12, 14	      MOSI, MISO, SCK (other SPI chips)
*/

// définir les responsabilités
// clarifier le rôle de chacun
// diagramme de classe et texte, liste de responsabilités, nommer les cercles

const uint8_t NUMBER_OF_SOUNDS = 4;

AudioPlaySdWav sounds[NUMBER_OF_SOUNDS];

AudioMixer4 mixerRight; //xy=317,705
AudioMixer4 mixerLeft;  //xy=320,630
AudioOutputI2S Output;  //xy=488,669

AudioConnection patchCord1(sounds[0], 0, mixerLeft, 0);
AudioConnection patchCord2(sounds[0], 1, mixerRight, 0);
AudioConnection patchCord3(sounds[1], 0, mixerLeft, 1);
AudioConnection patchCord4(sounds[1], 1, mixerRight, 1);
AudioConnection patchCord5(sounds[2], 0, mixerLeft, 2);
AudioConnection patchCord6(sounds[2], 1, mixerRight, 2);
AudioConnection patchCord7(sounds[3], 0, mixerLeft, 3);
AudioConnection patchCord8(sounds[3], 1, mixerRight, 3);
AudioConnection patchCord9(mixerRight, 0, Output, 1);
AudioConnection patchCord10(mixerLeft, 0, Output, 0);
// GUItool: end automatically generated code

AudioControlSGTL5000 audioModule;

#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 7
#define SDCARD_SCK_PIN 14

#define DEBUG

#ifdef DEBUG

#include <Metro.h>

Metro cpuUsage = Metro(500);

#endif

const uint8_t NUMBER_OF_PADS = 4;

drumModule pad[NUMBER_OF_PADS];
const uint8_t PAD_PIN[NUMBER_OF_PADS] = {1, 2, 3, 6};
String PAD_NAME[NUMBER_OF_PADS] = {"BASS001.WAV", "BASS001.WAV", "BASS001.WAV", "BASS001.WAV"};

void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
  while (!Serial)
    ; // Needed with Teensy
  Serial.println(F("Drum Module Started..."));
#endif

  AudioMemory(12);
  audioModule.enable();
  audioModule.volume(0.75f);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN)))
  {
    while (1)
    {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }

  for (uint8_t i = 0; i < NUMBER_OF_PADS; i++)
  {
    pad[i].init(PAD_PIN[i]);
  }

#ifdef DEBUG
  Serial.println(F("Drum Module Ready"));
#endif
}

void loop()
{
  for (uint8_t i = 0; i < NUMBER_OF_PADS; i++)
  {
    pad[i].manage();

    if (pad[i].isStruck())
    {
#ifdef DEBUG
      Serial.println(F("Pad struck"));
#endif

      bool soundPlayed = false;
      bool checkingAvailable = true;
      uint8_t soundIter = 0;
      uint32_t remainingTime[NUMBER_OF_SOUNDS];
      uint32_t minimumRemainingTime = UINT32_MAX;
      uint8_t soundToPlay = 0;

      while (!soundPlayed)
      {
        if (checkingAvailable)
        {
          if (!sounds[soundIter].isPlaying())
          {
            soundToPlay = soundIter;
            checkingAvailable = false;
          }
          else
          {
            remainingTime[soundIter] = sounds[soundIter].lengthMillis() - sounds[soundIter].positionMillis();
#ifdef DEBUG
          Serial.print(F("Remaining on Channel "));
          Serial.print(soundIter);
          Serial.print(F(" : "));
          Serial.println(remainingTime[soundIter]);
#endif
            if (remainingTime[soundIter] < minimumRemainingTime)
            {
              soundToPlay = soundIter;
              minimumRemainingTime = remainingTime[soundIter];
            }

            if (soundIter == NUMBER_OF_SOUNDS - 1)
            {
              checkingAvailable = false;
            }
            else
            {
              soundIter++;
            }
          }
        }
        else
        {
          float currentGain = pad[i].getHit();
          mixerLeft.gain(0, currentGain);
          mixerRight.gain(0, currentGain);

          sounds[soundToPlay].play("BASS001.WAV");
          soundPlayed = true;

#ifdef DEBUG
          Serial.print(F("Played on Channel "));
          Serial.println(soundToPlay);
#endif
        }
      }
    }
  }

#ifdef DEBUG
  if (cpuUsage.check() == 1)
  {
    Serial.print(F("CPU: "));
    Serial.print(AudioProcessorUsageMax());
    AudioProcessorUsageMaxReset();
    Serial.print(", Mem: ");
    Serial.println(AudioMemoryUsageMax());
    AudioMemoryUsageMaxReset();
  }

#endif
}