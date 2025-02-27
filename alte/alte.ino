#include "Arduino.h"
#include <FS.h>
#include "Wav.h"
#include "I2S.h"
#include <SD.h>
#include <HTTPClient.h>
#include <WiFi.h>

#define LED_PIN 26

//comment the first line and uncomment the second if you use MAX9814
//#define I2S_MODE I2S_MODE_RX
#define I2S_MODE I2S_MODE_ADC_BUILT_IN

const int record_time = 10;  // second
String filename = "/sound.wav";

const int headerSize = 44;
const int numCommunicationData = 8000;
const int numPartWavData = numCommunicationData / 4;
byte header[headerSize];
char communicationData[numCommunicationData];
char partWavData[numPartWavData];
File file;
String formData;

bool isRecording = false;
unsigned long timeRecordingStart = 0;
unsigned long timeRecordingEnd = 0;

#define ULTRASONIC_PIN_TIRG 4
#define ULTRASONIC_PIN_ECHO 16
bool isSensorOn = false;

int ledValue = 0;
float timeoutLedFade = 3000;

#define SEND_FILE_HOST "atra-bce32f116e3f.herokuapp.com"
#define SEND_FILE_PAHT "/alte"
#define SEND_FILE_PORT 80

#define WIFI_NAME "tasty.space"
#define WIFI_PASSWORD "spaceisnull"

void setup() {
  Serial.begin(115200);
  if (!SD.begin(15)) Serial.println("SD begin failed");
  // while (!SD.begin(15)) {
  //   Serial.print(".");
  //   delay(500);
  // }
  I2S_Init(I2S_MODE, I2S_BITS_PER_SAMPLE_32BIT);

  // pin is dimming with pwm
  ledcAttach(LED_PIN, 5000, 8);

  // proximity sensor
  pinMode(ULTRASONIC_PIN_TIRG, OUTPUT);
  pinMode(ULTRASONIC_PIN_ECHO, INPUT);

  // connect to WIFI
  WiFi.begin(WIFI_NAME, WIFI_PASSWORD);

  // adjust sensor threshold
  adjustSensorThreshold();
}

void startRecording() {
  isRecording = true;
  timeRecordingStart = millis();

  filename = "/sound" + String(timeRecordingStart) + ".wav";

  // open file
  file = SD.open(filename, FILE_WRITE);
  if (!file) return;

  // write header
  CreateWavHeader(header, 0, 44100, 2);
  file.write((const byte *)header, headerSize);
}

void endRecording() {
  isRecording = false;
  timeRecordingEnd = millis();
  unsigned long waveDataSize = file.size() - headerSize;
  file.seek(0);
  CreateWavHeader(header, waveDataSize, 8400, 2);
  file.write((const byte *)header, headerSize);
  file.close();

  // timeDiff = millis() - timeRecordingStart;

  // if (timeDiff > record_time * 1000) {
  //   upload_file();
  // }


}

unsigned long lastSensorCheckTime = 0;
const int sensorCheckInterval = 1000;
const int touchPin = T0; // Use GPIO 4 (T0)
int threshold = 65; // Adjust based on testing

void adjustSensorThreshold() { // median for first 10 seconds
  int sum = 0;
  for (int i = 0; i < 10; i++) {
      sum += touchRead(touchPin);
      delay(1000);
  }
  threshold = sum / 10 * .92;
  Serial.print("New threshold: ");
  Serial.println(threshold);
}

void checkSensor(){
  if (millis() - lastSensorCheckTime < sensorCheckInterval) return;
  lastSensorCheckTime = millis();
  int touchValue = touchRead(touchPin); // Read capacitance
  Serial.println(touchValue);
  if(touchValue < threshold) {
    isSensorOn = true;
  } else {
    isSensorOn = false;
  }
}

void loop() {
  checkSensor();

  if (isRecording) {
    // read from i2s, write to file
    I2S_Read(communicationData, numCommunicationData);
    for (int i = 0; i < numCommunicationData / 8; ++i) {
      partWavData[2 * i] = communicationData[8 * i + 2];
      partWavData[2 * i + 1] = communicationData[8 * i + 3];
    }
    file.write((const byte *)partWavData, numPartWavData);

    // fade led
    ledValue = (millis() - timeRecordingStart) / timeoutLedFade * 255;
    if(ledValue > 255) ledValue = 255;
    ledcWrite(LED_PIN, ledValue);

    if (!isSensorOn) {
      // fade out led with delay
      for(int i = 0; i < 255; i++) {
        ledcWrite(LED_PIN, 255 - i);
        delay(10);
      }
      endRecording();
      Serial.println("Recording ended");
    }
  } else {
    if (isSensorOn) {  // sensor is on
      startRecording();
      Serial.println("Recording started");
    }
  }
}

void upload_file() {
  file = SD.open(filename, FILE_READ);  

  // Clear formData string
  formData = "";

  // Get file size
  int fileSize = file.size();
  int numIterations = fileSize / 4096;
  if (fileSize % 4096 != 0) {
    numIterations++;  // Add one more iteration for the remaining bytes
  }

  Serial.print("File size in bytes: ");
  Serial.println(fileSize);
  Serial.print("Number of iterations: ");
  Serial.println(numIterations);

  // Connect to server
  WiFiClient client;
  if (!client.connect(SEND_FILE_HOST, SEND_FILE_PORT)) {
    Serial.println("Connection failed");
    return;
  }

  String boundary = "----boundary29348---";
  String fileName = "file.wav";

  formData += "--" + boundary + "\r\n";
  formData += "Content-Disposition: form-data; name=\"file\"; filename=\"" + fileName + "\"\r\n";
  formData += "Content-Type: audio/wav\r\n\r\n";

  String footer = "\r\n--" + boundary + "--\r\n";

  int contentLength = formData.length() + file.size() + footer.length();

  // Send POST request with Content-Length header
  client.print("POST /alte HTTP/1.1\r\n");
  client.print("Host: " + String(SEND_FILE_HOST) + "\r\n");
  client.print("Cache-Control: no-cache\r\n");
  client.print("Content-Type: multipart/form-data; boundary=" + boundary + "\r\n");
  client.print("Content-Length: " + String(contentLength) + "\r\n\r\n");

  // Send form data
  client.print(formData);

  // Send file content
  const int bufferSize = 4096;
  byte buffer[bufferSize];
  int bytesRead = 0;

  int z = 0;
  Serial.println(fileName);
  while (file.available()) {
    bytesRead = file.readBytes((char *)buffer, bufferSize);
    client.write(buffer, bytesRead);
    z++;
    Serial.print(z);
    Serial.print(" ");
  }
  file.close();
  Serial.println();
  Serial.println(z);
  // Send footer
  client.print(footer);

  String line = client.readStringUntil('{');
  Serial.println(line);

  String serverRes = client.readStringUntil('}');
  Serial.println("Server response: " + serverRes);

  client.stop();

  file.close();
}
