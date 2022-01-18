#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <USBKeyboard.h>

#define RGB_PIN 2
#define MUTE_PIN 3

Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, RGB_PIN, NEO_GRB + NEO_KHZ800);

USBKeyboard keyb;

uint8_t button_state = 0;

void setup() {
  pinMode(MUTE_PIN, INPUT_PULLUP);

  // put your setup code here, to run once:
  strip.begin();
  strip.setBrightness(50);
  strip.setPixelColor(0, strip.Color(0, 255, 0));
  strip.show();
}

void loop() {
  // put your main code here, to run repeatedly:

  button_state = button_state << 1 | digitalRead(MUTE_PIN);

  if ((button_state & 0xF) == 0b1110) {
    strip.setPixelColor(0, strip.Color(0, 255, 0));
    strip.show();
  } 
  else if ((button_state & 0xF) == 0b0001) {
    strip.setPixelColor(0, strip.Color(255, 0, 0));
    strip.show();
    //keyb._putc('A');
    keyb.key_code('m', 101);
  }
  sleep_ms(2);
}