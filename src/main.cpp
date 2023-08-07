#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "USBCustomDevice.h"
//#include <USBKeyboard.h>

#define RGB_PIN 3
#define MUTE_PIN 4
#define VERSION_INFO "MUTE_MIC_1_00"

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, RGB_PIN, NEO_GRB + NEO_KHZ800);

USBCustomDevice custDev;
uint8_t button_state = 0;

void setup() {
  pinMode(MUTE_PIN, INPUT_PULLUP);

  // put your setup code here, to run once:
  strip.begin();
  strip.setBrightness(50);
  strip.setPixelColor(0, strip.Color(0, 255, 0));
  strip.show();

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  button_state = button_state << 1 | digitalRead(MUTE_PIN);

  if ((button_state & 0xF) == 0b1110) {
    //strip.setPixelColor(0, strip.Color(0, 255, 0));
    //strip.show();
    Serial.print("P");
  } 
  else if ((button_state & 0xF) == 0b0001) {
    //strip.setPixelColor(0, strip.Color(255, 0, 0));
    //strip.show();
    //custDev._putc('A');
    //custDev.key_code('m', 101);
    Serial.print("R");
  }

  if (custDev.led_status_changed())
  {
    uint8_t lockStatus = custDev.led_status();

    strip.setPixelColor(0, strip.Color((lockStatus & 0x01) > 0 ? 255 : 0, (lockStatus & 0x02) > 0 ? 255 : 0, (lockStatus & 0x04) > 0 ? 255 : 0));
    strip.show();
  }

  // Read LED Status
  if (Serial.available())
  {
    int value = Serial.read();

    if (value == '~')
    {
      Serial.println(VERSION_INFO);
    }
    else if (value == 'L') // If "L" read next byte for color info
    {
      value = Serial.read();
      strip.setPixelColor(0, strip.Color((value & 0x01) > 0 ? 255 : 0, (value & 0x02) > 0 ? 255 : 0, (value & 0x04) > 0 ? 255 : 0));
      strip.show();
    }
  }
  /*
  else {
    uint8_t * ledColor = custDev.led_status();
    strip.setPixelColor(0, strip.Color(ledColor[0], ledColor[1], ledColor[2]));
    strip.show();
  }*/
  sleep_ms(2);
}
