/* 500Level project;
BERNARD JOSEPH
SAMUEL ELEMA
*/
#include <OneWire.h> //library for the Ds18b20 temperature sensor
#include <DallasTemperature.h> //library for the Ds18b20 temperature sensor
#include <Servo.h> //library for the servo motor
#include <Wire.h>

// Data wire is plugged into digital pin 4 on the Arduino for temp sensor
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire device
OneWire oneWire(ONE_WIRE_BUS);	

// Pass oneWire reference to DallasTemperature library
DallasTemperature sensors(&oneWire);


Servo base_servo;  // create servo object to control a servo motor at the base of the arm
Servo right_servo; // create servo object for the right servo motor
Servo left_servo; // create servo object for the left servo motor
// twelve servo objects can be created on most boards

int pos = 0;    // variable to store the servo position
int pH_Value;
float Voltage;

float calibration_value = 21.34+0.99; // for the ph sensor
int phval = 0; 
unsigned long int avgval; 
int buffer_arr[10],temp;  // for the ph sensor

void setup() {
  base_servo.attach(9);  // attaches the servo on pin 9 to the servo object
  right_servo.attach(10);
  left_servo.attach(11);
  pinMode(pH_Value, INPUT);
  sensors.begin();	// Start up the library
  Serial.begin(9600);
}

void loop()
{
  delay(2000); // wait
  for (pos = 0; pos <= 110; pos += 1) { // goes from 0 degrees to 110 degrees
    // in steps of 1 degree
   base_servo.write(pos);              // tell servo to go to position in variable 'pos' ie 0 degrees to 110 degrees
    delay(23);                       // waits 23 ms for the servo to reach the position also determines how fast it goesn 
  }
  delay(1000);
 
for (pos = 60; pos <= 135; pos += 1) { // goes from 60 degrees to 100 degrees, for the right servo motor
    // in steps of 1 degree
   right_servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(23);
  }
 for (pos = 170; pos >= 30; pos -= 1) {  // activates the left servo motor
   left_servo.write(pos);              
    delay(23);
  }
for (pos = 135; pos <= 150; pos += 1) { // goes from 60 degrees to 100 degrees, for the right servo motor
    // in steps of 1 degree
   right_servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(23);
  }
for(int i=0;i<10;i++){ 
 buffer_arr[i]=analogRead(A0);
 delay(30);
 }
 for(int i=0;i<9;i++){
 for(int j=i+1;j<10;j++){
 if(buffer_arr[i]>buffer_arr[j]){
 temp=buffer_arr[i];
 buffer_arr[i]=buffer_arr[j];
 buffer_arr[j]=temp;
 }
 }
 }
 avgval=0;
 for(int i=2;i<8;i++)
 avgval+=buffer_arr[i];
 float volt=(float)avgval*5.0/1024/6; 
float ph_act = -5.70 * volt + calibration_value;
 
  delay(5000); // adding delay to allow the PH senosr record the PH of the water

  for (pos = 30; pos <= 170; pos += 1) { // goes from 110 degrees to 180 degrees
    // in steps of 1 degree
   left_servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(23);
  }

  delay(1000);

  for (pos = 150; pos >= 60; pos -= 1) { 
   right_servo.write(pos);              
    delay(23);   
 }
delay(2000);
 for (pos = 110; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    base_servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(23);                       // waits 15 ms for the servo to reach the position
  }

//ph code
 /* pH_Value = analogRead(A0); //to calibrate the sensor
  Voltage = pH_Value * (5.0 / 1023.0); 
  Serial.println(Voltage); 
  delay(500);*/






 
 
 







//temprature sensor code in here.

// Send the command to get temperatures
 sensors.requestTemperatures(); 

  //print the temperature in Celsius
  Serial.print("Temperature: ");
  Serial.print(sensors.getTempCByIndex(0));
  //Serial.print((char)176);//shows degrees character
  Serial.println("C  |  "); }

  //yesterday 04th we added the ph sensor code inbtw the robotic arm code 
