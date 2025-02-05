#include <esp_now.h>
#include <WiFi.h>

#define NUM_STRIPS 8

// MAC addresses of the receiver
// 8C:AA:B5:0F:55:CA  Z
// C4:D8:D5:37:05:68  Y
// F4:CF:A2:DF:A1:2E  X
// 50:02:91:D9:EE:7D  W
// E0:98:06:24:42:B6  V
// D8:BF:C0:FA:91:0B  U
// 24:4C:AB:55:A9:F9  T
// 8C:AA:B5:D5:3A:94  S
uint8_t broadcastAddresses[][6] = {
  {0x8C, 0xAA, 0xB5, 0x0F, 0x55, 0xCA},
  {0xC4, 0xD8, 0xD5, 0x37, 0x05, 0x68},
  {0xF4, 0xCF, 0xA2, 0xDF, 0xA1, 0x2E},
  {0x50, 0x02, 0x91, 0xD9, 0xEE, 0x7D},
  {0xE0, 0x98, 0x06, 0x24, 0x42, 0xB6},
  {0xD8, 0xBF, 0xC0, 0xFA, 0x91, 0x0B},
  {0x24, 0x4C, 0xAB, 0x55, 0xA9, 0xF9},
  {0x8C, 0xAA, 0xB5, 0xD5, 0x3A, 0x94},
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
  if(stripIndex != 0) {
    stripIndex = stripIndex - 1; // Strip 1 is actually index 0
  }

  if(stripIndex >= NUM_STRIPS) {
    Serial.println("Invalid strip index");
    return;
  }


  // Set values to myData
  myData.messageType = messageType;
  myData.brightness = brightness;
  myData.position = position;
  myData.width = width;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddresses[stripIndex], (uint8_t *)&myData, sizeof(myData));
  if(result != ESP_OK) {
    Serial.println("Error sending data to strip " + stripIndex);
  }
}

int pos = 0;

void loop()
{
  if (Serial.available() > 5)
  {
    uint8_t stripIndex = Serial.read(); // Discard the first byte
    uint8_t messageType = Serial.read();
    uint8_t brightness = Serial.read();
    uint8_t position = Serial.read();
    uint8_t width = Serial.read();
    sendData(stripIndex, messageType, brightness, position, width);
  }
  // pos++;
  // if (pos > 100) {
  //   pos = 0;
  // }

  // sendData(false, 100, pos, 50);
  // delay(100);
}