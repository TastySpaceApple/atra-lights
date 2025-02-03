#include <FastLED.h>

#define PIN 3
#define NUM_LEDS 30 // Change this to the number of LEDs in your strip

CRGB leds[NUM_LEDS];

void setup() {
  FastLED.addLeds<WS2811, PIN>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show(); // Initialize all pixels to 'off'
}

void loop() {
  // Nothing to do here
  for (int brightness = 0; brightness <= 60; brightness++) {
    setStripBrightness(255, 255, 255, brightness);
    delay(50);
  }
  for (int brightness = 60; brightness >= 0; brightness--) {
    setStripBrightness(255, 255, 255, brightness);
    delay(50);
  }
}

void setStripBrightness(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness) {
  for (int i = 0; i < NUM_LEDS; i++) {
    if(i != 100){
      leds[i] = CRGB((red * brightness) / 255, (green * brightness) / 255, (blue * brightness) / 255);
    } else {
      leds[i] = CRGB(0, 0, 0);
    }
  }
  FastLED.show();
}