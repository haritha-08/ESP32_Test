#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ---------------- WIFI ----------------
const char* ssid = "ESP32TEST";
const char* password = "12345678";

// ---------------- NODE.JS SERVER ----------------
// Replace with YOUR laptop IPv4 address
const char* serverName = "http://10.173.26.125:3000/sensor";

// ---------------- DHT22 ----------------
#define DHTPIN 4
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// ---------------- MQ135 ----------------
#define MQ_PIN 34

// ---------------- RAIN SENSOR ----------------
#define RAIN_PIN 35

// ---------------- BUZZER ----------------
#define BUZZER 26

// ---------------- LCD ----------------
LiquidCrystal_I2C lcd(0x27, 16, 4);

void setup() {

  Serial.begin(115200);

  // DHT22
  dht.begin();

  // Buzzer
  pinMode(BUZZER, OUTPUT);

  // LCD
  Wire.begin(21, 22);

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("SMART MONITOR");

  delay(2000);

  lcd.clear();

  // ---------------- WIFI CONNECT ----------------

  WiFi.begin(ssid, password);

  lcd.setCursor(0,0);
  lcd.print("Connecting WiFi");

  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");

  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("WiFi Connected");

  delay(2000);

  lcd.clear();
}

void loop() {

  // ---------------- SENSOR VALUES ----------------

  float temp = dht.readTemperature();

  int airValue = analogRead(MQ_PIN);

  int rainValue = analogRead(RAIN_PIN);

  String alert = "NORMAL";

  // ---------------- BUZZER ----------------

  digitalWrite(BUZZER, LOW);

  // HIGH TEMPERATURE
  if (temp > 35) {

    alert = "HIGH TEMP";

    digitalWrite(BUZZER, HIGH);
  }

  // BAD AIR QUALITY
  else if (airValue > 2000) {

    alert = "BAD AIR";

    digitalWrite(BUZZER, HIGH);
  }

  // RAIN DETECTED
  else if (rainValue < 2000) {

    alert = "RAIN ALERT";

    digitalWrite(BUZZER, HIGH);
  }

  // ---------------- LCD DISPLAY ----------------

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Temp:");
  lcd.print(temp);
  lcd.print(" C");

  lcd.setCursor(0,1);
  lcd.print("Air:");
  lcd.print(airValue);

  lcd.setCursor(0,2);
  lcd.print("Rain:");
  lcd.print(rainValue);

  lcd.setCursor(0,3);
  lcd.print(alert);

  // ---------------- SERIAL MONITOR ----------------

  Serial.print("Temp: ");
  Serial.print(temp);

  Serial.print(" | Air: ");
  Serial.print(airValue);

  Serial.print(" | Rain: ");
  Serial.print(rainValue);

  Serial.print(" | Alert: ");
  Serial.println(alert);

  // ---------------- SEND DATA TO NODE.JS ----------------

  if (WiFi.status() == WL_CONNECTED) {

    HTTPClient http;

    http.begin(serverName);

    http.addHeader("Content-Type", "application/json");

    // JSON DATA
    String jsonData = "{";

    jsonData += "\"temperature\":" + String(temp) + ",";
    jsonData += "\"air\":" + String(airValue) + ",";
    jsonData += "\"rain\":" + String(rainValue) + ",";
    jsonData += "\"alert\":\"" + alert + "\"";

    jsonData += "}";

    // SEND POST REQUEST
    int httpResponseCode = http.POST(jsonData);

    Serial.print("HTTP Response: ");
    Serial.println(httpResponseCode);

    http.end();
  }

  else {

    Serial.println("WiFi Disconnected");
  }

  // ---------------- UPDATE EVERY 5 SECONDS ----------------

  delay(5000);
}

