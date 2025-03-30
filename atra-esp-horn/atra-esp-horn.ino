#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <YX5300_ESP32.h>
#include <EEPROM.h>

#define MESSAGE_TYPE_SOUND_START 8
#define MESSAGE_TYPE_SOUND_STOP 9
#define MESSAGE_TYPE_SOUND_VOLUME 10

// *make sure the RX on the YX5300 goes to the TX on the ESP32, and vice-versa
#define RX 16
#define TX 17

typedef struct struct_message
{
  uint8_t messageType;
  uint8_t brightness;
  uint8_t position;
  uint8_t width;
} struct_message;


struct_message receivedData;

YX5300_ESP32 player;

void recv(const esp_now_recv_info *info, const uint8_t *incoming, int len)
{
  memcpy(&receivedData, incoming, sizeof(receivedData));
  handleIncoming();
}

void handleIncoming(){
  switch (receivedData.messageType)
  {
    case MESSAGE_TYPE_SOUND_START:
      player.playTrackInLoop(receivedData.brightness);
      player.resume();
      break;
    case MESSAGE_TYPE_SOUND_STOP:
      player.pause();
      break;
    case MESSAGE_TYPE_SOUND_VOLUME:
      player.setVolume(receivedData.brightness);
      saveVolume(receivedData.brightness);
      break;
  }
}

void readMacAddress(){
  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret == ESP_OK) {
    Serial.printf("%02x:%02x:%02x:%02x:%02x:%02x\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("Failed to read MAC address");
  }
}


void saveVolume(int volume) {
  // Save the volume
  EEPROM.write(0, volume);
  EEPROM.commit();
}

void setup()
{
  Serial.begin(115200);

  delay(2000);

  player = YX5300_ESP32(Serial2, RX, TX);

  WiFi.mode(WIFI_AP_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  Serial.println("esp now started");
  
  esp_now_register_recv_cb(recv);

  int volume = EEPROM.read(0);
  if(isnan(volume) || volume == 0 || volume > 30) {
    volume = 10;
  }

  player.setVolume(volume);
  player.playTrackInLoop(1);
  player.resume();
}

void loop()
{
  delay(1000);
  saveVolume(10)
}
