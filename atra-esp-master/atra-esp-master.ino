#include <esp_now.h>
#include <WiFi.h>

// MAC address of the receiver
//CC:DB:A7:96:4C:10
uint8_t broadcastAddress[] = {0xCC, 0xDB, 0xA7, 0x96, 0x4C, 0x10};

typedef struct struct_message
{
  bool isColorMessage;
  uint8_t brightness;
  uint8_t position;
  uint8_t width;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Serial.print("\r\nLast Packet Send Status:\t");
  // Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  if (Serial.available() > 5) {
    int chunk = Serial.read(); // Discard the first byte
    // Read data from Serial
    myData.isColorMessage = Serial.read();
    myData.brightness = Serial.read();
    myData.position = Serial.read();
    myData.width = Serial.read();

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    } else {
      Serial.println("Error sending the data");
    }
    
  }
}