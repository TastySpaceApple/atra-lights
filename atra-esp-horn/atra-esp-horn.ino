#include <WiFi.h>
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

void recv(const uint8_t *mac, const uint8_t *incoming, int len)
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

void saveVolume(int volume) {
  // Save the volume
  EEPROM.write(0, volume);
  EEPROM.commit();
}

void setup()
{
  Serial.begin(115200);

  player = YX5300_ESP32(Serial2, RX, TX);

  Serial.println("ESP Board MAC Address: ");
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_recv_cb(esp_now_recv_cb_t(recv));

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
}
