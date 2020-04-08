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

AudioPlaySdWav sound0;
AudioOutputI2S headphones;
AudioConnection patchCord1(sound0, 0, headphones, 0);
AudioConnection patchCord2(sound0, 1, headphones, 1);
AudioControlSGTL5000 audioModule;

#define SDCARD_CS_PIN 10
#define SDCARD_MOSI_PIN 7
#define SDCARD_SCK_PIN 14

#define DEBUG

const uint8_t NUMBER_OF_PADS = 4;

drumModule pad[NUMBER_OF_PADS];
const uint8_t PAD_PIN[NUMBER_OF_PADS] = {1, 2, 3, 6};

void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
  while (!Serial)
    ; // Needed with Teensy
  Serial.println(F("Drum Module Started..."));
#endif

  AudioMemory(8);
  audioModule.enable();
  audioModule.volume(0.75);
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
      Serial.println(F("Sound played"));
#endif
      audioModule.volume(pad[i].getHit());
      sound0.play("BASS001.WAV");
    }
  }
}