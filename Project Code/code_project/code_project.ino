#include <WiFi.h>
#include <HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP32Servo.h>

// WiFi credentials
const char* ssid = "**********";
const char* password = "*****";

// Django server URL
const char* serverURL = "*****/api/sensor-readings/";

// Pin definitions
#define trig 19
#define echo 21
#define relay1 22 // Relay for draining water
#define relay2 23 // Relay for filling water
#define ONE_WIRE_BUS 18 // Data wire for temperature sensor

// DS18B20 temperature sensor setup
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Servo motors
Servo base_servo;
Servo right_servo;
Servo left_servo;

// Calibration value for pH sensor
float calibration_value = 21.34;
int buf[10];

// Global variables for servo positions
int basePosition = 0;
int rightPosition = 60;
int leftPosition = 170;
int pos;

// Function prototypes
void connectToWiFi();
void sendReadingsToServer(String activityType, float phValue, float temperature, int waterLevel);
void sendContinuousData(int waterLevel, int distance, int basePosition, int rightPosition, int leftPosition);
int measure_distance();
float measure_ph();
void move_arm_to_dip();
void return_arm_to_initial();

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
}

void sendReadingsToServer(String activityType, float phValue = 0, float temperature = 0, int waterLevel = -1) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    String payload = "{";
    payload += "\"activity_type\": \"" + activityType + "\"";
    if (phValue > 0) payload += ", \"ph\": " + String(phValue);
    if (temperature > 0) payload += ", \"temperature\": " + String(temperature);
    if (waterLevel >= 0) payload += ", \"water_level\": " + String(waterLevel);
    payload += "}";

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
      Serial.println("Data sent successfully: " + http.getString());
    } else {
      Serial.println("Error sending data: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("Wi-Fi not connected");
  }
}

void sendContinuousData(int waterLevel, int distance, int basePosition, int rightPosition, int leftPosition) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    String payload = "{";
    payload += "\"water_level\": " + String(waterLevel) + ",";
    payload += "\"distance\": " + String(distance) + ",";
    payload += "\"arm_position\": {";
    payload += "\"base\": " + String(basePosition) + ",";
    payload += "\"right\": " + String(rightPosition) + ",";
    payload += "\"left\": " + String(leftPosition);
    payload += "}";
    payload += "}";

    int httpResponseCode = http.POST(payload);
    if (httpResponseCode > 0) {
      Serial.println("Continuous data sent: " + http.getString());
    } else {
      Serial.println("Error sending data: " + String(httpResponseCode));
    }
    http.end();
  } else {
    Serial.println("Wi-Fi not connected");
  }
}

int measure_distance() {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long duration = pulseIn(echo, HIGH);
  int distance = (duration / 2) / 29.1;
  return distance;
}

float measure_ph() {
  float avgValue = 0;
  for (int i = 0; i < 10; i++) {
    buf[i] = analogRead(33); // pH sensor connected to GPIO 33
    delay(10);
  }
  for (int i = 0; i < 10; i++) {
    avgValue += buf[i];
  }
  float pHvol = avgValue * 3.3 / 4095 / 10;
  float phValue = -5.70 * pHvol + calibration_value;
  return phValue;
}

void move_arm_to_dip() {
  for (pos = 0; pos <= 110; pos += 1) { 
  base_servo.write(pos);       
  basePosition = pos;       
  delay(20); }    

delay(1000);
 
for (pos = 90; pos <= 140; pos += 1) { 
  right_servo.write(pos);  
  rightPosition = pos;           
  delay(20);
  }

  delay(1000);

 for (pos = 170; pos >= 90; pos -= 1) { 
  left_servo.write(pos); 
  leftPosition = pos;             
  delay(20);
  }
  delay(1000);

for (pos = 140; pos <= 175; pos += 1) { 
   right_servo.write(pos);
    rightPosition = pos;              
    delay(20);}
}

void return_arm_to_initial() {
  for (pos = 175; pos >= 140; pos -= 1) { 
  right_servo.write(pos);
  rightPosition = pos;            
  delay(20);   
 }
 delay(1000);
  for (pos = 140; pos >= 90; pos -= 1) { 
  right_servo.write(pos);
  rightPosition = pos;            
  delay(20);   
 }

 delay(1000);
for (pos = 90; pos <= 170; pos += 1) { 
left_servo.write(pos);
leftPosition = pos;              
delay(20);
  }

delay(1000);
 for (pos = 110; pos >= 0; pos -= 1) { 
    base_servo.write(pos);
    basePosition = pos;            
    delay(23);                      
  }

}

void setup() {
  Serial.begin(9600);

  base_servo.attach(15);
  right_servo.attach(2);
  left_servo.attach(4);

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);

  sensors.begin();

  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);

  connectToWiFi();
}

void loop() {
  int distance = measure_distance();
  int waterLevel = map(distance, 3, 13, 100, 0); // Convert to percentage

  sendContinuousData(waterLevel, distance, basePosition, rightPosition, leftPosition);

  static unsigned long lastReadingTime = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastReadingTime > 10000) { // Every 10 seconds

    move_arm_to_dip();

    float phValue = measure_ph();
    sensors.requestTemperatures();
    float temperatureC; // Declare temperatureC outside the loop
    for (int i = 0; i < 25; i++) {
        temperatureC = sensors.getTempCByIndex(0); // Update its value inside the loop
        delay(5);
    }
    float temperature = temperatureC; // Now you can use it here

    

    sendReadingsToServer("measurement", phValue, temperature, waterLevel);

    // State 1: Drain water if critical condition
  if (temperature <= 20.0 || temperature >= 33.0 ) { // we removed the PH condition, to added later
    Serial.println("Critical condition detected. Draining water...");
    digitalWrite(relay1, HIGH); // Start draining
    digitalWrite(relay2, LOW);  // Ensure filling is off

    // Send "draining" activity
    sendReadingsToServer("water_draining");

    // Wait until the water is fully drained
    while (distance < 14) {
        distance = measure_distance();
        int waterLevel = map(distance, 3, 13, 100, 0); // Convert to percentage
        sendContinuousData(waterLevel, distance, basePosition, rightPosition, leftPosition);

        Serial.print("Draining... Current distance: ");
        Serial.println(distance);
        delay(500); // Small delay to avoid overloading the loop
    }

    // Stop draining
    sendReadingsToServer("water_drained");
    Serial.println("Tank drained. Stopping relay1.");
    digitalWrite(relay1, LOW);
  }

    // State 2: Fill water
  if (distance >= 13) {
    Serial.println("Starting to fill the tank...");
    digitalWrite(relay2, HIGH); // Start filling

    // Send "filling" activity
    sendReadingsToServer("water_filling");

    // Wait until the tank is fully filled
    while (distance > 2) {
        distance = measure_distance();
        int waterLevel = map(distance, 3, 13, 100, 0); // Convert to percentage
        sendContinuousData(waterLevel, distance, basePosition, rightPosition, leftPosition);
        Serial.print("Filling... Current distance: ");
        Serial.println(distance);
        delay(500); // Small delay for stability
    }

    // Stop filling
    sendReadingsToServer("water_filled");
    Serial.println("Tank filled. Stopping relay2.");
    digitalWrite(relay2, LOW);
  }

    return_arm_to_initial();
    lastReadingTime = currentTime;
  }

  delay(500);
}
