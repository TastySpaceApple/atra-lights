#include <ESP8266WiFi.h>
#include <espnow.h>

// MAC addresses of the receiver
// lights ----------------
// Z	8C:AA:B5:0F:55:CA	1
// Y	C4:D8:D5:37:05:68	2
// X	F4:CF:A2:DF:A1:2E	3
// W	50:02:91:D9:EE:7D	4
// V	E0:98:06:24:42:B6	5
// U	D8:BF:C0:FA:91:0B	6
// T	24:4C:AB:55:A9:F9	7
// S	8C:AA:B5:D5:3A:94	8
// R	4C:EB:D6:DE:9E:DA	9
// Q	E8:DB:84:D8:BE:5A	10
// P	A0:B7:65:04:9F:A0	11
// O	4C:EB:D6:DE:9F:2B	12
// N	4C:EB:D6:DE:9F:7C	13
// M	4C:EB:D6:DE:9E:E3	14
// F	cc:db:a7:96:4c:10	15
uint8_t broadcastAddresses[][6] = {
  {0x8C, 0xAA, 0xB5, 0x0F, 0x55, 0xCA},
  {0xC4, 0xD8, 0xD5, 0x37, 0x05, 0x68},
  {0xF4, 0xCF, 0xA2, 0xDF, 0xA1, 0x2E},
  {0x50, 0x02, 0x91, 0xD9, 0xEE, 0x7D},
  {0xE0, 0x98, 0x06, 0x24, 0x42, 0xB6},
  {0xD8, 0xBF, 0xC0, 0xFA, 0x91, 0x0B},
  {0x24, 0x4C, 0xAB, 0x55, 0xA9, 0xF9},
  {0x8C, 0xAA, 0xB5, 0xD5, 0x3A, 0x94},
  {0x4C, 0xEB, 0xD6, 0xDE, 0x9E, 0xDA},
  {0xE8, 0xDB, 0x84, 0xD8, 0xBE, 0x5A},
  {0xA0, 0xB7, 0x65, 0x04, 0x9F, 0xA0},
  {0x4C, 0xEB, 0xD6, 0xDE, 0x9F, 0x2B},
  {0x4C, 0xEB, 0xD6, 0xDE, 0x9F, 0x7C},
  {0x4C, 0xEB, 0xD6, 0xDE, 0x9E, 0xE3},
  {0xcc, 0xdb, 0xa7, 0x96, 0x4c, 0x10}
};

#define MESSAGE_TYPE_BRIGHTNESS 0
#define MESSAGE_TYPE_COLOR 1
#define MESSAGE_TYPE_LED_NUMBER 2

typedef struct struct_message
{
  uint8_t messageType;
  uint8_t brightness;
  uint8_t position;
  uint8_t width;
} struct_message;

// Create a struct_message called myData
struct_message esp_now_message;

const int buttonBrightnessPin = D1; 
const int buttonWidthPin = D2; 
const int buttonPositionPin = D3; 

const int trigPin = D6; // Pin for the ultrasonic sensor trigger
const int echoPin = D7; // Pin for the ultrasonic sensor echo

const float rangeDistanceMaxCm = 18; // Maximum distance we want to ping for (in centimeters)
const float rangeDistanceMinCm = 4; // Minimum distance we want to ping for (in centimeters)

int position = 50;
int brightness = 50;
int width = 50;

int startDistance = 0;
int startValue = 0;
bool isDragging = false;


void setup() {
  // Initialize the push button pins as inputs
  pinMode(buttonBrightnessPin, INPUT_PULLUP);
  pinMode(buttonWidthPin, INPUT_PULLUP);
  pinMode(buttonPositionPin, INPUT_PULLUP);

  // Initialize the ultrasonic sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Optionally, initialize serial communication for debugging
  Serial.begin(9600);

  // espnow init and register peers
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  for (int i = 0; i < 15; i++) {
    esp_now_add_peer(broadcastAddresses[i], ESP_NOW_ROLE_CONTROLLER, 1, NULL, 0);
  }
}

int moveDragHandValue(int currentValue){
  long duration;
  float distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2;

  if(!isDragging){
    startDistance = distance;
    startValue = currentValue;
    isDragging = true;
    return currentValue;
  } else {
    float diff = startDistance - distance;
    float diffNormalized = diff / (rangeDistanceMaxCm - rangeDistanceMinCm) * 100;
    currentValue = startValue - diffNormalized;
    if(currentValue < 0){
      currentValue = 0;
    } else if(currentValue > 100){
      currentValue = 100;
    }
    return currentValue;
  }
}

void endDrag(){
  isDragging = false;
}

void sendData(){
  esp_now_message.messageType = MESSAGE_TYPE_BRIGHTNESS;
  esp_now_message.brightness = brightness;
  esp_now_message.position = position;
  esp_now_message.width = width;

  esp_now_send(NULL, (uint8_t *)&esp_now_message, sizeof(esp_now_message));
}

void loop() {
  if (digitalRead(buttonBrightnessPin) == LOW) {
    brightness = moveDragHandValue(brightness);
    sendData();
  }
  else if (digitalRead(buttonWidthPin) == LOW) {
    width = moveDragHandValue(width);
    sendData();
  }
  else if (digitalRead(buttonPositionPin) == LOW) {
    position = moveDragHandValue(position);
    sendData();
  } 
  else {
    endDrag();
  }

  // Add a small delay to avoid flooding the serial monitor
  delay(40);
}