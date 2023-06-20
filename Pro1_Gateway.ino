#include <esp_now.h>
#include <WiFi.h>

const char* ssid = "QiQi";
const char* password = "UyenLe2401";

#define CHANNEL 1 // Kênh ESP-NOW
typedef struct __attribute__((packed)) {
  float temperature;
  float humidity;
  float moisture;
} SensorData;
SensorData receivedData;

void printReceivedData() {
  Serial.print("Nhiet do: ");
  Serial.print(receivedData.temperature);
  Serial.println(" C");
  Serial.print("Do am khong khi: ");
  Serial.print(receivedData.humidity);
  Serial.println(" %");
  Serial.print("Do am dat: ");
  Serial.print(receivedData.moisture);
  Serial.println(" %");
  Serial.println();
}

void onDataReceived(const uint8_t *macAddress, const uint8_t *data, int dataLength) {
  if (dataLength == sizeof(receivedData)) {
    memcpy(&receivedData, data, sizeof(receivedData));
    printReceivedData();
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_now_register_recv_cb(onDataReceived);
}

void loop() {
  if (Serial.available() > 0) {
    // Đọc dữ liệu từ Serial để tránh treo chương trình
    while (Serial.available() > 0) {
      Serial.read();
    }
  }
}
