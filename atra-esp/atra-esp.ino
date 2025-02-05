#include <FastLED.h>
#include <esp_now.h>
#include <WiFi.h>

#define LED_STRIP_PIN 13
#define NUM_LEDS 44 // Number of LEDs in the strip

CRGB leds[NUM_LEDS];

const int updateInterval = 20;  // Update every 20ms
unsigned long lastUpdate = 0;

byte r = 255, g = 255, b = 255;

typedef struct struct_message
{
  bool isColorMessage;
  uint8_t brightness;
  uint8_t position;
  uint8_t width;
} struct_message;

struct_message receivedData;

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  // Use receivedData.brightness, receivedData.position, and receivedData.width as needed
  if(receivedData.isColorMessage) {
    setColor(receivedData.brightness, receivedData.position, receivedData.width);
  } else {
    setBrightness(receivedData.position, receivedData.brightness, receivedData.width);
  }
}

void setColor(int brightness, int position, int width) {
  r = brightness;
  g = position;
  b = width;
}

void setup()
{
  delay(1000);
  FastLED.addLeds<NEOPIXEL, LED_STRIP_PIN>(leds, NUM_LEDS);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);
}


void setBrightness(int position, int brightness, int width) {  
  if(width <= 0){ width = 1; }
  
  for (int index = 0; index < NUM_LEDS; index++) {
    float normalizedIndex = (float)index / NUM_LEDS * 100; // 0 - 100
    float distance = abs(normalizedIndex - position); // 0 - 100
    int ledBrightness = min(255, max(0, brightness - (int)(distance / width * brightness)));
    leds[index] = CRGB((r * ledBrightness) / 255, (g * ledBrightness) / 255, (b * ledBrightness) / 255);
  }
}

void loop()
{
  if (millis() - lastUpdate > updateInterval) {
    lastUpdate = millis();
    FastLED.show();
  }

  delay(1);
}