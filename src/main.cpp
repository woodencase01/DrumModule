#include <Arduino.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <drumModule.h>

/*
Audio Data	  9, 11, 13, 22, 23	
Audio Control	18, 19	            SDA, SCL (other I2C chips)
Volume Pot	  15                  (A1)
SD Card	      7, 10, 12, 14	      MOSI, MISO, SCK (other SPI chips)
*/

/*
Config file
Config commands
Test WAV, RAW, V30 SD card
WAV to MEM
Hit curves
*/

//#define WAV_SOUNDS // Play sounds from SD card WAV
#define MEM_SOUNDS // Play sounds from memory

const uint8_t NUMBER_OF_SOUNDS = 4;
const uint8_t NUMBER_OF_PADS = 4;

#ifdef WAV_SOUNDS
AudioPlaySdWav sounds[NUMBER_OF_SOUNDS];
#endif

#ifdef MEM_SOUNDS
#include "Sounds/AudioSampleSnare.h"
#include "Sounds/AudioSampleTomtom.h"
#include "Sounds/AudioSampleHihat.h"
#include "Sounds/AudioSampleBd001_11khz.h"
AudioPlayMemory sounds[NUMBER_OF_SOUNDS];
const unsigned int *padReference[NUMBER_OF_PADS] = {AudioSampleBd001_11khz, AudioSampleSnare, AudioSampleTomtom, AudioSampleHihat};
#endif

AudioMixer4 mixerRight; //xy=317,705
AudioMixer4 mixerLeft;  //xy=320,630
AudioOutputI2S Output;  //xy=488,669

#ifdef WAV_SOUNDS
AudioConnection patchCord1(sounds[0], 0, mixerLeft, 0);
AudioConnection patchCord2(sounds[0], 1, mixerRight, 0);
AudioConnection patchCord3(sounds[1], 0, mixerLeft, 1);
AudioConnection patchCord4(sounds[1], 1, mixerRight, 1);
AudioConnection patchCord5(sounds[2], 0, mixerLeft, 2);
AudioConnection patchCord6(sounds[2], 1, mixerRight, 2);
AudioConnection patchCord7(sounds[3], 0, mixerLeft, 3);
AudioConnection patchCord8(sounds[3], 1, mixerRight, 3);
#endif

#ifdef MEM_SOUNDS
AudioConnection patchCord1(sounds[0], 0, mixerLeft, 0);
AudioConnection patchCord2(sounds[0], 0, mixerRight, 0);
AudioConnection patchCord3(sounds[1], 0, mixerLeft, 1);
AudioConnection patchCord4(sounds[1], 0, mixerRight, 1);
AudioConnection patchCord5(sounds[2], 0, mixerLeft, 2);
AudioConnection patchCord6(sounds[2], 0, mixerRight, 2);
AudioConnection patchCord7(sounds[3], 0, mixerLeft, 3);
AudioConnection patchCord8(sounds[3], 0, mixerRight, 3);
#endif

AudioConnection patchCord9(mixerRight, 0, Output, 1);
AudioConnection patchCord10(mixerLeft, 0, Output, 0);

AudioControlSGTL5000 audioModule;

#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 7
#define SDCARD_SCK_PIN 14

#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
#define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#include <Metro.h>
Metro cpuUsage = Metro(500);
#else
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#endif

drumModule pad[NUMBER_OF_PADS];
const uint8_t PAD_PIN[NUMBER_OF_PADS] = {1, 2, 3, 6};

#ifdef WAV_SOUNDS
String PAD_NAME[NUMBER_OF_PADS] = {"BASS001.WAV", "BASS001.WAV", "BASS001.WAV", "BASS001.WAV"};
#endif

File configFile;
const char CONFIG_HEADER[] = "Enabled,MinThreshold,MaxThreshold,SoundID";
const char CONFIG_FILE[] = "CONFIG.CFG"; 
const uint8_t HEADER_VERSION = 1;

enum configState : uint8_t
{
  padEnabled,
  minThreshold,
  maxThreshold,
  soundID,
  configStateMax
};

void initConfig()
{
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

  Serial.println("Initialization done.");

  configFile = SD.open(CONFIG_FILE, FILE_WRITE);

  if (configFile)
  {
    DEBUG_PRINTLN("CONFIG Present.");

    configFile.seek(0);

    // Check for version
    configFile.readStringUntil(',');
    int version = configFile.readStringUntil('\n').toInt();
    if (version < HEADER_VERSION)
    {
      // Wrong version, create new file
      DEBUG_PRINTLN("Too old version, recreating file...");
      SD.remove(CONFIG_FILE);
      //SD.remove("CONFIG.CFG");
      configFile = SD.open(CONFIG_FILE, FILE_WRITE);
      configFile.write("Version,");
      configFile.write(HEADER_VERSION);
      configFile.write('\n');
      configFile.write(CONFIG_HEADER);
      for (size_t i = 0; i < NUMBER_OF_PADS; i++)
      {
        configFile.write("0,20,300,");
        configFile.write(i);
        configFile.write('\n');
      }
    }
    else 
    {
      // Version is the same, read configuration
      uint8_t currentConfigStage = minThreshold;

      while (configFile.available() && currentConfigStage < configStateMax)
      {
        switch (currentConfigStage)
        {
        case padEnabled:
          /* code */
          currentConfigStage++;
          break;
        case minThreshold:
          /* code */
          currentConfigStage++;
          break;
        case maxThreshold:
          /* code */
          currentConfigStage++;
          break;
        case soundID:
          /* code */
          currentConfigStage++;
          break;
        }
      }
    }

    configFile.close();
  }
  else
  {
    Serial.println("Can't open CONFIG.TXT");
  }
}

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
  audioModule.volume(1.0f);

  initConfig();

  for (uint8_t i = 0; i < NUMBER_OF_PADS; i++)
  {
    pad[i].init(PAD_PIN[i]);
    pad[i].setPadSound(padReference[i]);
  }

  DEBUG_PRINTLN(F("Drum Module Ready"));
}

void loop()
{
  for (uint8_t i = 0; i < NUMBER_OF_PADS; i++)
  {
    pad[i].manage();

    if (pad[i].isStruck())
    {
      DEBUG_PRINTLN(F("Pad struck"));

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

            DEBUG_PRINT(F("Remaining on Channel "));
            DEBUG_PRINT(soundIter);
            DEBUG_PRINT(F(" : "));
            DEBUG_PRINTLN(remainingTime[soundIter]);

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

#ifdef WAV_SOUNDS
          sounds[soundToPlay].play("BASS001.WAV");
#endif

#ifdef MEM_SOUNDS
          sounds[soundToPlay].play(padReference[i]);
#endif
          soundPlayed = true;

          DEBUG_PRINT(F("Played on Channel "));
          DEBUG_PRINTLN(soundToPlay);
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