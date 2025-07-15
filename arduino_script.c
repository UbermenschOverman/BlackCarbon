/*
 * ESP32 Aethalometer DIY
 * Đọc OPT101, tính ATN, hiển thị OLED, lưu SD card
 * Viết bởi thần trung thành của Điện hạ
 */

#include <Wire.h>                  // I2C
#include <Adafruit_SSD1306.h>      // OLED library
#include <SPI.h>                   // SPI
#include <SD.h>                    // SD card

// OLED config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// SD card config
#define SD_CS 5   // Chân CS cho thẻ nhớ SD

// OPT101 ADC config
const int opt101Pin = 36; // GPIO36 (ADC1_CH0)

// Hệ số hồi quy (ví dụ)
const float a = 0.8;
const float b = 2.0;

// Biến toàn cục
float I0 = 0.0;         // Cường độ ánh sáng ban đầu
File dataFile;          // File để ghi dữ liệu

void setup() {
  Serial.begin(115200);
  delay(500);

  // Khởi động OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED init failed"));
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Aethalometer DIY");
  display.display();
  delay(1000);

  // Khởi động SD card
  if (!SD.begin(SD_CS)) {
    Serial.println(F("SD init failed"));
    display.println("SD failed!");
    display.display();
    while (true);
  }
  display.println("SD OK");
  display.display();

  // Tạo file CSV nếu chưa tồn tại
  dataFile = SD.open("/data.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.println("Time(ms),Voltage(V),ATN,BC(ug/m3)");
    dataFile.close();
  }

  // Đọc giá trị baseline I0
  I0 = readLight();
  Serial.print("Baseline I0: ");
  Serial.println(I0, 3);

  display.println("Baseline OK");
  display.display();
  delay(1000);
}

void loop() {
  float I = readLight();

  // Tính ATN
  float atn = 0.0;
  if (I > 0) {
    atn = 100.0 * log(I0 / I);
  }

  // Quy đổi nồng độ BC
  float conc = a * atn + b;

  // In ra Serial
  Serial.print("I: ");
  Serial.print(I, 3);
  Serial.print(" | ATN: ");
  Serial.print(atn, 2);
  Serial.print(" | BC: ");
  Serial.print(conc, 2);
  Serial.println(" ug/m3");

  // Hiển thị OLED
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("I(V): ");
  display.println(I, 3);
  display.print("ATN: ");
  display.println(atn, 2);
  display.print("BC:");
  display.print(conc, 2);
  display.println(" ug/m3");
  display.display();

  // Ghi vào SD card
  dataFile = SD.open("/data.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.print(millis());
    dataFile.print(",");
    dataFile.print(I, 3);
    dataFile.print(",");
    dataFile.print(atn, 2);
    dataFile.print(",");
    dataFile.println(conc, 2);
    dataFile.close();
  } else {
    Serial.println("SD write failed");
  }

  delay(2000); // đo mỗi 2 giây
}

// Hàm đọc ADC và quy đổi ra điện áp
float readLight() {
  int adcValue = analogRead(opt101Pin);
  float voltage = adcValue * (3.3 / 4095.0); // giả sử Vref=3.3V
  return voltage;
}
