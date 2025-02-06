#include <FastLED.h>
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <espnow.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <esp_wifi.h>
  #include <esp_now.h>
#endif
#include <EEPROM.h>

#if defined(ESP8266)
  #define LED_STRIP_PIN 12 // Pin where LED strip is connected
#elif defined(ESP32)
  #define LED_STRIP_PIN 27 // Pin where LED strip is connected
#endif

#define MESSAGE_TYPE_BRIGHTNESS 0
#define MESSAGE_TYPE_COLOR 1
#define MESSAGE_TYPE_LED_NUMBER 2

int numLeds = 0;

CRGB leds[60];
int lastBrightness = 0;
int lastPosition = 0;
int lastWidth = 0;

const int updateInterval = 20;  // Update every 20ms
unsigned long lastUpdate = 0;

byte r = 255, g = 255, b = 255;

typedef struct struct_message
{
  uint8_t messageType;
  uint8_t brightness;
  uint8_t position;
  uint8_t width;
} struct_message;

struct_message receivedData;

#if defined(ESP8266)
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  handleIncoming();
}
#elif defined(ESP32)
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len)
{
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  handleIncoming();
}
#endif

void handleIncoming(){
  switch (receivedData.messageType)
  {
    case MESSAGE_TYPE_BRIGHTNESS:
      setBrightness(receivedData.position, receivedData.brightness, receivedData.width);
      break;
    case MESSAGE_TYPE_COLOR:
      setColor(receivedData.brightness, receivedData.position, receivedData.width);
      break;
    case MESSAGE_TYPE_LED_NUMBER:
      saveNumPins(receivedData.brightness);
      break;
  }
}

void setColor(int brightness, int position, int width) {
  r = brightness;
  g = position;
  b = width;

  setBrightness(lastPosition, lastBrightness, lastWidth);
}

void saveNumPins(int numPins) {
  // Save the number of pins
  EEPROM.write(0, numPins);
  EEPROM.commit();

  startingSequence();  

  // then restart
  ESP.restart();
}

void setup()
{
  Serial.begin(9600);
  delay(2000);
  EEPROM.begin(12);
  numLeds = EEPROM.read(0);
  if(isnan(numLeds) || numLeds == 0 || numLeds > 60) {
    numLeds = 20;
  }

  FastLED.addLeds<NEOPIXEL, LED_STRIP_PIN>(leds, numLeds);

  WiFi.mode(WIFI_STA);
#if defined(ESP32)
  WiFi.STA.begin();
#endif

  if (esp_now_init() != 0)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  
  esp_now_register_recv_cb(OnDataRecv);

  // Print the MAC address
  uint8_t mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC Address: ");
  for (int i = 0; i < 6; i++) {
    if (mac[i] < 16) Serial.print("0");
    Serial.print(mac[i], HEX);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  startingSequence();
}


void setBrightness(int position, int brightness, int width) {  
  if(width <= 0){ width = 1; }
  
  for (int index = 0; index < numLeds; index++) {
    float normalizedIndex = (float)index / numLeds * 100; // 0 - 100
    float distance = abs(normalizedIndex - position); // 0 - 100
    int ledBrightness = min(255, max(0, brightness - (int)(distance / width * brightness)));
    leds[index] = CRGB((r * ledBrightness) / 255, (g * ledBrightness) / 255, (b * ledBrightness) / 255);
  }

  lastBrightness = brightness;
  lastPosition = position;
  lastWidth = width;
}

void startingSequence(){
  for (int i = 0; i < numLeds; i++) {
    leds[i] = CRGB::Purple;
    FastLED.show();
    delay(100);
    leds[i] = CRGB::Black;
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