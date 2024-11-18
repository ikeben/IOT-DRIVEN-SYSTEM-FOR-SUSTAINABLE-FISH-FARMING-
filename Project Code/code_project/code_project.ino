/* 500Level Project
BERNARD JOSEPH
SAMUEL ELEMA
*/
#define trig 19
#define echo 21
#define relay1 22 // Relay or LED connected to GPIO 22
#define relay2 23 // Relay or LED connected to GPIO 23

#include <OneWire.h> // Library for the DS18B20 temperature sensor
#include <DallasTemperature.h> // Library for the DS18B20 temperature sensor
#include <ESP32Servo.h> // Library for the servo motor

// Data wire is plugged into digital pin 18 for the temperature sensor
#define ONE_WIRE_BUS 18

// Setup a OneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);

// Create servo objects for the robotic arm
Servo base_servo;
Servo right_servo;
Servo left_servo;

// Variables for pH sensor
float calibration_value = 21.34; // Calibration value for the pH sensor
int buf[10]; // Buffer for pH sensor readings

void setup() {
  base_servo.attach(15);  // Base servo connected to GPIO 15
  right_servo.attach(2);  // Right servo connected to GPIO 2
  left_servo.attach(4);   // Left servo connected to GPIO 4

  pinMode(trig, OUTPUT);   // Ultrasonic trigger pin
  pinMode(echo, INPUT);    // Ultrasonic echo pin
  pinMode(relay1, OUTPUT); // Relay 1 control pin
  pinMode(relay2, OUTPUT); // Relay 2 control pin

  sensors.begin();	// Start the temperature sensor library
  Serial.begin(9600); // Initialize Serial for debugging
}

int measure_distance() {
  // Send trigger pulse
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  // Read echo pulse duration
  long duration = pulseIn(echo, HIGH);

  // Calculate distance (in cm)
  int distance = (duration / 2) / 29.1;
  return distance; // Return the measured distance
}

float measure_ph() {
  // Collect multiple pH sensor readings
  float avgValue = 0;
  for (int i = 0; i < 10; i++) {
    buf[i] = analogRead(33); // pH sensor connected to GPIO 33
    delay(10);
  }
  for (int i = 0; i < 10; i++) {
    avgValue += buf[i];
  }
  float pHvol = avgValue * 3.3 / 4095 / 10; // Convert ADC value to voltage
  float phValue = -5.70 * pHvol + calibration_value; // Calculate pH value
  return phValue;
}

void loop() {
  delay(2000); // Wait before the loop starts

  // Move base servo from 0 to 110 degrees
  for (int pos = 0; pos <= 110; pos += 1) {
    base_servo.write(pos);
    delay(23);
  }

  // Move right servo from 60 to 135 degrees
  for (int pos = 60; pos <= 135; pos += 1) {
    right_servo.write(pos);
    delay(23);
  }

  // Move left servo from 170 to 30 degrees
  for (int pos = 170; pos >= 30; pos -= 1) {
    left_servo.write(pos);
    delay(23);
  }

  // Dip the sensor to measure pH
  for (int pos = 135; pos <= 150; pos += 1) { 
    right_servo.write(pos); // Move right servo to dipping position
    delay(23);              
  }
  
  // Measure pH while dipped
  float phValue = measure_ph();
  // Serial.print("pH Value: ");
  // Serial.println(phValue);

  // Measure temperature
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);
  // Serial.print("Temperature: ");
  // Serial.println(temperature);

  // Measure distance
  int distance = measure_distance();
  // Serial.print("Distance: ");
  // Serial.println(distance);

  delay(3000);

  Serial.print("pH Value: ");
  Serial.println(phValue);

  Serial.print("Temperature: ");
  Serial.println(temperature);

  Serial.print("Distance: ");
  Serial.println(distance);

  // Relay control logic
  if (temperature <= 20.0 || temperature >= 33.0 || phValue <= 6.5 || phValue >= 9.0) {
    digitalWrite(relay1, HIGH); // Turn ON relay1 if temperature and pH are in range
    if (distance >= 14) {
      digitalWrite(relay1, LOW); // Turn OFF relay1 if distance >= 15
      digitalWrite(relay2, HIGH); // Turn ON relay2
    } else if (distance <= 3) {
      digitalWrite(relay2, LOW); // Turn OFF relay2 if distance <= 3
    }
  } else {
    // Turn OFF both relays if conditions are not met
    digitalWrite(relay1, LOW);
    digitalWrite(relay2, LOW);
  }

  // Return arm to its initial position

  for (int pos = 30; pos <= 170; pos++) {
    left_servo.write(pos);
    delay(23);
  }
    for (int pos = 150; pos >= 135; pos -= 1) { //code swapped at 18/1124 23:16
    right_servo.write(pos);
    delay(23);
  }
  for (int pos = 135; pos >= 60; pos--) {
    right_servo.write(pos);
    delay(23);
  }
  for (int pos = 110; pos >= 0; pos--) {
    base_servo.write(pos);
    delay(23);
  }
}
