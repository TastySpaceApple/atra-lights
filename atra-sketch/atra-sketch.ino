#include <FastLED.h>

// a message is represented using the following properties:
// channel (number): 1-14
// chunk (number): 1-8
// position (number): 0-100
// brightness (number): 0-100
// width (number): 0-100

#define COLOR_ORDER GRB
#define CHIP WS2811

unsigned long lastUpdate = 0;
const int updateInterval = 20;  // Update every 20ms

#define NUM_STRIPS 5

const int chunksLengths[NUM_STRIPS][8] = {
  {8, 6, 6, 4},
  {20, 1},
  {8, 1, 1, 1, 2},
  {8, 2, 2, 2, 2},
  {16, 10, 8}
};
int countLeds[NUM_STRIPS];

byte r=255, g=223, b=186;

CRGB* leds[NUM_STRIPS];


void countNumLeds() {
  for (int i = 0; i < NUM_STRIPS; i++) {
    int numLeds = 0;
    for (int j = 0; j < 8; j++) {
      numLeds += chunksLengths[i][j];
    }
    countLeds[i] = numLeds;
    leds[i] = new CRGB[countLeds[i]];
    // start all with black
    for (int j = 0; j < countLeds[i]; j++) {
      leds[i][j] = CRGB::Black;
    }
  }
}

void setup() {
  // Start the serial communication at 9600 baud rate
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for the serial port to connect. Needed for native USB port only
  }
  countNumLeds();

  FastLED.addLeds<CHIP, 3, COLOR_ORDER>(leds[0], countLeds[0]);
  FastLED.addLeds<CHIP, 5, COLOR_ORDER>(leds[1], countLeds[1]);
  FastLED.addLeds<CHIP, 6, COLOR_ORDER>(leds[2], countLeds[2]);
  FastLED.addLeds<CHIP, 9, COLOR_ORDER>(leds[3], countLeds[3]);
  FastLED.addLeds<CHIP, 10, COLOR_ORDER>(leds[4], countLeds[4]);
  FastLED.clear();
  FastLED.show(); // Initialize all pixels to 'off'

}

// recieves 3 bytes
void handleBrightnessMessage(byte byte1, byte byte2, byte byte3, byte byte4) {
  // the first bye is the channel and the chunk
  int channel = byte1 >> 4;
  int chunk = byte1 & 0x0F;

  int position = byte2;
  int brightness = byte3;
  int width = byte4;

  setBrightness(channel, chunk, position, brightness, width);
}

void setBrightness(int channel, int chunk, int position, int brightness, int width) {
  channel = max(channel - 1, 0);
  chunk = max(chunk - 1, 0);
  
  // we only affect the leds in the specified chunk
  int startLed = 0;
  for (int i = 0; i < chunk; i++) {
    startLed += chunksLengths[channel][i];
  }
  int chunkSize = chunksLengths[channel][chunk];
  int endLed = startLed + chunkSize;
  if(width <= 0){ width = 1; }

  for (int index = 0; index < chunkSize; index++) {
    int ledIndex = startLed + index;
    float normalizedIndex = (float)index / chunkSize * 100; // 0 - 100
    float distance = abs(normalizedIndex - position); // 0 - 100
    int ledBrightness = min(255, max(0, brightness - (distance / width * brightness)));
    leds[channel][ledIndex] = CRGB((r * ledBrightness) / 255, (g * ledBrightness) / 255, (b * ledBrightness) / 255);
  }
}

void turnChunkRed(int channel, int chunk) {
  channel = max(channel - 1, 0);
  chunk = max(chunk - 1, 0);
  
  // we only affect the leds in the specified chunk
  int startLed = 0;
  for (int i = 0; i < chunk; i++) {
    startLed += chunksLengths[channel][i];
  }
  int chunkSize = chunksLengths[channel][chunk];
  int endLed = startLed + chunkSize;

  for (int index = 0; index < chunkSize; index++) {
    int ledIndex = startLed + index;
    leds[channel][ledIndex] = CRGB(255, 0, 0);
  }
}

void handleColorMessage(byte byte1, byte byte2, byte byte3) {
  r = byte1;
  g = byte2;
  b = byte3;
}

void loop() {
  if (millis() - lastUpdate > updateInterval) {
    lastUpdate = millis();
    FastLED.show();
  }

  if (Serial.available() >= 4) {
    byte byte1 = Serial.read();
    byte byte2 = Serial.read();
    byte byte3 = Serial.read();
    byte byte4 = Serial.read();
    if(byte1 == 0){
      handleColorMessage(byte2, byte3, byte4);
    } else {
      handleBrightnessMessage(byte1, byte2, byte3, byte4);
    }
  }

}