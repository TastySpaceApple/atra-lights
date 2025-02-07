#include <esp_now.h>
#include <WiFi.h>

#define NUM_STRIPS 15

// MAC addresses of the receiver
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
struct_message myData;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  // Serial.print("\r\nLast Packet Send Status:\t");
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup()
{
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  // add peers
  for (int i = 0; i < NUM_STRIPS; i++)
  {
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, broadcastAddresses[i], 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
      Serial.println("Failed to add peer");
    }
  }
}

void sendData(uint8_t stripIndex, uint8_t messageType, uint8_t brightness, uint8_t position, uint8_t width)
{
  if(stripIndex > NUM_STRIPS) {
    Serial.println("Invalid strip index");
    return;
  }

  // Set values to myData
  myData.messageType = messageType;
  myData.brightness = brightness;
  myData.position = position;
  myData.width = width;

  if(stripIndex == 0) {
    Serial.println("Sending data to all strips");
    esp_now_send(NULL, (uint8_t *)&myData, sizeof(myData));
  } else {
    Serial.print("Sending data to strip ");
    Serial.println(stripIndex);
    esp_now_send(broadcastAddresses[stripIndex - 1], (uint8_t *)&myData, sizeof(myData));
  }
}

int pos = 0;

void loop()
{
  if (Serial.available() > 5)
  {
    uint8_t stripIndex = Serial.read();
    uint8_t messageType = Serial.read();
    uint8_t brightness = Serial.read();
    uint8_t position = Serial.read();
    uint8_t width = Serial.read();
    sendData(stripIndex, messageType, brightness, position, width);

    // dump the rest
    while (Serial.available() > 0)
    {
      Serial.read();
    }
  }
}