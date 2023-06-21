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
#include <esp_now.h>
#include <WiFi.h>
#include <DHT.h>                                    //thu vien cho cam bien DHT11
#include <Adafruit_SSD1306.h>

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
float temperature;     

// REPLACE WITH YOUR RECEIVER MAC Address
//24:0A:C4:C5:97:24
uint8_t broadcastAddress[] = {0x24, 0x0A, 0xC4, 0xC5, 0x97, 0x24};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  float temperature;
  float humidity;
  float moisture;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  
    //Khoi tao Oled 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  //Moisture sensor
  analogReadResolution(12);                         // set ADC resolution to 12 bits (0-4095) 
  pinMode(moisturePin, INPUT);
  
  //Tempurate sensor 
  dht.begin();
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
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
  
  // Set values to send
  myData.temperature = temperature;
  myData.humidity = humidity;
  myData.moisture = moisture;
  
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }

  //Hien thi Oled
  display.clearDisplay(); // Xóa màn hình
  display.setTextSize(1); // Kích thước chữ
  display.setTextColor(SSD1306_WHITE); // Màu chữ
  display.setCursor(0, 0); // Vị trí con trỏ
  display.println(" ");
  display.println(" ");
  display.println("Temperature: " + String(temperature) + " C");
  display.println("Humidity: " + String(humidity) + " %");
  display.println("Moisture: " + String(moisture) + " %");
  display.println("Status ESP-NOW:");
  if(result == ESP_OK) {
    display.println("Send data successfull");
  }
  else {
    display.println("Send data fail");
  }
  display.display(); // Hiển thị lên màn hình
  delay(2000);
}
