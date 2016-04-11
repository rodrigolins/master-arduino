// Libraries
#include <ArduinoJson.h>
#include <DHT.h>

// 0 = Normal mode (No errors detected)
// 1 = Error mode (Errors detected)
int OPERATION_MODE = 0;
const int LED2PIN = 2; // Error operation pin

// LED 1 PIN - Digital
const int LED1PIN = 13; // Keep alive

// TEMPERATURE PIN - Analog
const int TEMPERATURE1PIN = 0;

// BUTTON PIN - Digital
const int BUTTON1PIN = 12;
int button1State = 0;

// DHT PIN & TYPE - ANALOG
#define DHTPIN 11
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// TRI AXIAL - ANALOG
const int XPIN = A1;                  // x-axis of the accelerometer
const int YPIN = A2;                  // y-axis
const int ZPIN = A3;                  // z-axis (only on 3-axis models)

//zero_G is the reading we expect from the sensor when it detects
//no acceleration.  Subtract this value from the sensor reading to
//get a shifted sensor reading.
const float ZEROG = 512.0;

//scale is the number of units we expect the sensor reading to
//change when the acceleration along an axis changes by 1G.
//Divide the shifted sensor reading by scale to get acceleration in Gs.
const float SCALE = 102.3;

// PIR - DIGITAL
const int CALIBRATIONTIME = 2;  // Change back to 30 seconds

const int PIRPIN = 10;

// TONE SENSOR - DIGITAL
const int TONEPIN = 9;

// ULTRASONIC SENSOR - DIGITAL
const int ECHOPIN = 8;
const int TRIGPIN = 9;
long DURATION;
long DISTANCE;

void setup() {
  Serial.begin(9600);

  // Iniating LED
  pinMode(LED1PIN, OUTPUT);
  pinMode(LED2PIN, OUTPUT);

  // Initiating BUTTON
  pinMode(BUTTON1PIN, INPUT);

  // Initating 3 AXIS
//  analogReference(EXTERNAL);
//  pinMode(xpin, INPUT);
//  pinMode(ypin, INPUT);
//  pinMode(zpin, INPUT);

  // PIR
  pinMode(PIRPIN, INPUT);
  digitalWrite(PIRPIN, LOW);
  Serial.print("Calibrating PIR sensor");
  for(int i = 0; i < CALIBRATIONTIME; i++){
    Serial.print(".");
    delay(1000);
  }
  Serial.println("done");
  delay(50);

  // Initiating ULTRASONIC SENSOR
  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);

}

void loop() {
  // Don't use the buffer globaly it will crash
  // Even in case to call printTo, the buffer is not empty.
  StaticJsonBuffer<4000> jsonBuffer;

  // LED 1
  digitalWrite(LED1PIN, HIGH);
  delay(50);
  digitalWrite(LED1PIN, LOW);
  delay(50);

  // LED 2
  digitalWrite(LED2PIN, LOW);
  if(OPERATION_MODE == 0) {
    digitalWrite(LED2PIN, LOW);
  }
  if(OPERATION_MODE == 1){
    digitalWrite(LED2PIN, HIGH);
  }


  // TEMPERATURE 1
  float mv = (analogRead(TEMPERATURE1PIN) / 1024.0) * 5000;
  float celcius = mv / 10;
  float fahrenheit = (celcius * 9) / 5 + 32;

  JsonObject& sensor01 = jsonBuffer.createObject();
  sensor01["id"] = 1;
  sensor01["sensorType"] = "Thermometer";
  sensor01["model"] = "LM35";
  sensor01["manufacturer"] = "Generic";
  JsonArray& properties01 = sensor01.createNestedArray("properties");
  JsonObject& value01 = jsonBuffer.createObject();
  value01["name"] = "Temperature";
  value01["value"] = celcius;
  value01["unit"] = "C";
  properties01.add(value01);
  sensor01.printTo(Serial);
  Serial.println();

  // BUTTON 1
  button1State = digitalRead(BUTTON1PIN);

  JsonObject& sensor02 = jsonBuffer.createObject();
  sensor02["id"] = 2;
  sensor02["sensorType"] = "Push button";
  sensor02["model"] = "Generic";
  sensor02["manufacturer"] = "Generic";
  JsonArray& properties02 = sensor02.createNestedArray("properties");
  JsonObject& value02 = jsonBuffer.createObject();
  value01["name"] = "State";
  if (button1State == HIGH) {
    tone(TONEPIN, 80, 1000);
    value02["value"] = 1;
  }
  else {
    value02["value"] = 0;
  }
  value02["unit"] = "boolean";
  properties02.add(value02);
  sensor02.printTo(Serial);
  Serial.println();

  // DIGITAL HUMIDITY TEMPERATURE SENSOR
  float humidity = dht.readHumidity();
  float temperatureCelcius = dht.readTemperature();
  float temperatureFahrenheit = dht.readTemperature(true);

//  if (isnan(h) || isnan(t) || isnan(f)) {
//    Serial.println("Failed to read from DHT sensor!");
//    return;
//  }
  float heatIndex = dht.computeHeatIndex(temperatureFahrenheit, humidity);
//  float heatIndexC = dht.computeHeatIndexC(temperatureCelcius, humidity);
  //Serial.println(heatIndexC);

  JsonObject& sensor03 = jsonBuffer.createObject();
  sensor03["id"] = 3;
  sensor03["sensorType"] = "Hygrometer";
  sensor03["model"] = "DHT22";
  sensor03["manufacturer"] = "Generic";
  JsonArray& properties03 = sensor03.createNestedArray("properties");
  JsonObject& value03_01 = jsonBuffer.createObject();
  value03_01["name"] = "Humidity";
  value03_01["value"] = humidity;
  value03_01["unit"] = "%";
  properties03.add(value03_01);
  JsonObject& value03_02 = jsonBuffer.createObject();
  value03_02["name"] = "Temperature";
  value03_02["value"] = temperatureCelcius;
  value03_02["unit"] = "C";
  properties03.add(value03_02);
  JsonObject& value03_03 = jsonBuffer.createObject();
  value03_03["name"] = "Heat index";
  value03_03["value"] = dht.convertFtoC(heatIndex);
  value03_03["unit"] = "C";
  properties03.add(value03_03);
  sensor03.printTo(Serial);
  Serial.println();


  // TRIAXAL ACCELEROMETER
  int x_axis = analogRead(XPIN);
  delay(2);  //add a small delay between pin readings.

  int y_axis = analogRead(YPIN);
  delay(2); //add a small delay between pin readings.

  int z_axis = analogRead(ZPIN);

  JsonObject& sensor04 = jsonBuffer.createObject();

  sensor04["id"] = 4;
  sensor04["sensorType"] = "Triaxial accelerometer";
  sensor04["model"] = "MMA7361";
  sensor04["manufacturer"] = "Geeetech";
  JsonArray& properties04 = sensor04.createNestedArray("properties");
  JsonObject& value04_01 = jsonBuffer.createObject();
  value04_01["name"] = "X axis";
  value04_01["value"] = (((float)x_axis - ZEROG) / SCALE);
  value04_01["unit"] = "g";
  properties04.add(value04_01);
  JsonObject& value04_02 = jsonBuffer.createObject();
  value04_02["name"] = "Y axis";
  value04_02["value"] = (((float) y_axis - ZEROG) / SCALE);
  value04_02["unit"] = "g";
  properties04.add(value04_02);
  JsonObject& value04_03 = jsonBuffer.createObject();
  value04_03["name"] = "Z axis";
  value04_03["value"] = (((float) z_axis - ZEROG) / SCALE);
  value04_03["unit"] = "g";
  properties04.add(value04_03);
  sensor04.printTo(Serial);
  Serial.println();


  // PIR SENSOR (PASSIVE INFRARED SENSOR)
  JsonObject& sensor05 = jsonBuffer.createObject();

  sensor05["id"] = 5;
  sensor05["sensorType"] = "Passive infrared";
  sensor05["model"] = "RE200B";
  sensor05["manufacturer"] = "Generic";
  JsonArray& properties05 = sensor05.createNestedArray("properties");
  JsonObject& value05 = jsonBuffer.createObject();
  value05["name"] = "State";
  if(digitalRead(PIRPIN) == HIGH) {
    value05["value"] = 1;
  } else {
    value05["value"] = 0;
  }
  value05["unit"] = "boolean";
  properties05.add(value05);
  sensor05.printTo(Serial);
  Serial.println();

  // ULTRASONIC SENSOR
  JsonObject& sensor06 = jsonBuffer.createObject();

  sensor06["id"] = 6;
  sensor06["sensorType"] = "Ultrasonic";
  sensor06["model"] = "HC-SR04";
  sensor06["manufacturer"] = "Generic";
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIGPIN, LOW);
  DURATION = pulseIn(ECHOPIN, HIGH);

  DISTANCE = DURATION/58.2;

  JsonArray& properties06 = sensor06.createNestedArray("properties");
  JsonObject& value06 = jsonBuffer.createObject();
  value06["name"] = "Distance";
  value06["value"] = DISTANCE;
  value06["unit"] = "cm";
  properties06.add(value06);
  sensor06.printTo(Serial);
  Serial.println();

  // SERIAL READ
  if (Serial.available() > 0) {
    String content = "";
    char character;
    while(Serial.available()) {
      character = Serial.read();
      content.concat(character);
    }
    if(content.startsWith("{") && content.endsWith("}")) {

    }
    Serial.print("echo: ");
    Serial.println(content);
  }

  delay(1000);
}
