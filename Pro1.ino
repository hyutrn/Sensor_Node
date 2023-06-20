///////////////////////////////////////////////////
// Author: Tran Trong Huy                        //
// Project: Smart Garden                         //
// Source:  MQTT Sensor Client                   //
// Description: Đọc dữ liệu từ cảm biến DHT11,   //
//              và cảm biến độ ẩm đất, in ra     //
//              oled SSD1306 và gửi tới Gateway  //
//              bằng ESP-NOW                     //
///////////////////////////////////////////////////         
#include <Wire.h>
#include <DHT.h>                                    //thu vien cho cam bien DHT11
#include <WiFi.h>
#include <Adafruit_SSD1306.h>
#include <esp_now.h>

//config cho Oled SSD1306
#define SCREEN_WIDTH 128 // Chiều rộng màn hình OLED
#define SCREEN_HEIGHT 64 // Chiều cao màn hình OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//config cho Moisture Sensor 
#define moisturePin 36
float moisture;
int valueMois;

//config cho Temperature and Humidity sensor
#define DHTPIN 4                                    // Khai báo chân kết nối của DHT11
#define DHTTYPE DHT11                               // Khai báo loại cảm biến DHT11
DHT dht(DHTPIN, DHTTYPE);
float humidity;                                     // Khai báo biến lưu trữ độ ẩm
float temperature;                                  // Khai báo biến lưu trữ nhiệt độ

//Config ESP-NOW

const char* ssid = "QiQi";
const char* password = "UyenLe2401";

#define CHANNEL 1 // Kênh ESP-NOW
typedef struct __attribute__((packed)) {
  float temperature;
  float humidity;
  float moisture;
} SensorData;
SensorData sensorData;
// Địa chỉ MAC của ESP32 nhận dữ liệu 3C:E9:0E:86:C4:B4
uint8_t receiverMACAddress[] = {0x3C, 0xE9, 0x0E, 0x86, 0xC4, 0xB4}; // Thay bằng địa chỉ MAC thực của ESP32 nhận
// Khai báo hàm callback gửi dữ liệu qua ESP-NOW
void sendData(const uint8_t *macAddr, esp_now_send_status_t status) {
  // Không cần thực hiện thêm hành động trong callback này
}


void setup() {
  Serial.begin(115200);
  //Khoi tao Oled 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  //Moisture sensor
  analogReadResolution(12);                         // set ADC resolution to 12 bits (0-4095) 
  pinMode(moisturePin, INPUT);
  //Tempurate sensor 
  dht.begin();
  //ESP-NOW
  WiFi.mode(WIFI_STA); // Chế độ Station (Client)
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
  esp_now_register_send_cb(sendData); // Đăng ký callback gửi dữ liệu
}

void loop() {
  //Doc nhiet do va do am tu DHT11
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT11 sensor!");
    delay(1000);
    return;
  }
  
  //Doc do am dat tu sensor
  valueMois = analogRead(moisturePin);
  moisture = (100-((valueMois/4095.00)*100));

  //Gửi dữ liệu đến ESP32 Gateway
  sensorData.temperature = temperature;
  sensorData.humidity = humidity;
  sensorData.moisture = moisture;
  sendData(NULL, ESP_NOW_SEND_SUCCESS);

  //In serial
  Serial.print("Nhiet do: ");
  Serial.print(temperature);
  Serial.println(" ºC ");
  Serial.print("Do am khong khi: ");
  Serial.print(humidity);
  Serial.println(" % ");
  Serial.print("Do am dat: ");
  Serial.print(moisture);
  Serial.println(" % ");
  Serial.println(WiFi.macAddress());
  //Hien thi Oled
  display.clearDisplay(); // Xóa màn hình

  display.setTextSize(1); // Kích thước chữ
  display.setTextColor(SSD1306_WHITE); // Màu chữ
  display.setCursor(0, 0); // Vị trí con trỏ
  display.println("Nhiet do: " + String(temperature) + " C");
  display.println("Do am khong khi: " + String(humidity) + " %");
  display.println("Do am dat: " + String(moisture) + " %");
  display.display(); // Hiển thị lên màn hình
  delay(5000);
}
