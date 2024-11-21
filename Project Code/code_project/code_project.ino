/* 500Level Project
BERNARD JOSEPH
SAMUEL ELEMA
*/

#define trig 19
#define echo 21
#define relay1 22 // Relay for draining water
#define relay2 23 // Relay for filling water

#include <OneWire.h> // Library for the DS18B20 temperature sensor
#include <DallasTemperature.h> // Library for the DS18B20 temperature sensor
#include <ESP32Servo.h> // Library for the servo motor

#define ONE_WIRE_BUS 18 // Data wire for temperature sensor
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

Servo base_servo;
Servo right_servo;
Servo left_servo;

float calibration_value = 21.34; // Calibration value for the pH sensor
int buf[10];

void setup() {
  base_servo.attach(15);
  right_servo.attach(2);
  left_servo.attach(4);

  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);

  sensors.begin();
  Serial.begin(9600);

  digitalWrite(relay1, LOW); // Ensure relays are OFF initially
  digitalWrite(relay2, LOW);
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
}

void return_arm_to_initial() {
  // Return arm to its initial position
  for (int pos = 30; pos <= 170; pos++) {
    left_servo.write(pos);
    delay(23);
  }

  for (int pos = 150; pos >= 60; pos -= 1) {
    right_servo.write(pos);
    delay(23);
  }

  for (int pos = 110; pos >= 0; pos--) {
    base_servo.write(pos);
    delay(23);
  }
}

void loop() {
  // Move the robotic arm to dip the sensor
  move_arm_to_dip();

  // Measure pH and temperature
  float phValue = measure_ph();
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);

  // Measure distance
  int distance = measure_distance();

  // Print values to the serial monitor
  Serial.print("pH Value: ");
  Serial.println(phValue);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Distance: ");
  Serial.println(distance);

  // State 1: Drain water if critical condition
  if (temperature <= 20.0 || temperature >= 33.0 ) { // we removed the PH condition, to added later
    Serial.println("Critical condition detected. Draining water...");
    digitalWrite(relay1, HIGH); // Start draining
    digitalWrite(relay2, LOW);  // Ensure filling is off

    // Wait until the water is fully drained
    while (distance < 14) {
      distance = measure_distance();
      Serial.print("Draining... Current distance: ");
      Serial.println(distance);
      delay(500); // Small delay to avoid overloading the loop
    }

    // Stop draining
    Serial.println("Tank drained. Stopping relay1.");
    digitalWrite(relay1, LOW);
  }

  // State 2: Fill water
  if (distance >= 13) {
    Serial.println("Starting to fill the tank...");
    digitalWrite(relay2, HIGH); // Start filling

    // Wait until the tank is fully filled
    while (distance > 2) {
      distance = measure_distance();
      Serial.print("Filling... Current distance: ");
      Serial.println(distance);
      delay(500); // Small delay for stability
    }

    // Stop filling
    Serial.println("Tank filled. Stopping relay2.");
    digitalWrite(relay2, LOW);
  }

  // Move robotic arm back to the initial position
  return_arm_to_initial();

  // Wait before restarting the loop
  Serial.println("Rechecking conditions...");
  delay(2000);
}
