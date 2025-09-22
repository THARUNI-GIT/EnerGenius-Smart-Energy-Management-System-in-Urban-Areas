#include <ESP8266WiFi.h>
#include <DHT.h>

#define DHTPIN 2           // GPIO2 / D4 on NodeMCU
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// // Define WiFi credentials
const char *ssid = "Thanu";
const char *password = "NSuqA2sw";

// ThingSpeak API key and Channel ID
const String apiKey = "6NDGESNQFAMH6BX6";
const String channelID = "2730516";

// Define sensor pins
const int currentSensorPin = A0; // ACS712 current sensor
const int voltageSensorPin = A0; // ZMPT101B voltage sensor pin

WiFiClient client;

void setup() {
  Serial.begin(115200);
  dht.begin();
  connectToWiFi();
}

void loop() {
  // Read temperature and humidity from DHT11
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Read current and AC voltage
  float current = getAverageCurrent();
  float voltageAC = getACVoltage();

  // Print the sensor readings to the Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Current: ");
  Serial.print(current);
  Serial.print(" A, AC Voltage: ");
  Serial.println(voltageAC);

  // Send data to ThingSpeak
  sendToThingSpeak(temperature, humidity, current, voltageAC);

  delay(20000); // Delay for 20 seconds before sending the next reading
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");
}

void sendToThingSpeak(float temperature, float humidity, float current, float voltageAC) {
  String url = "http://api.thingspeak.com/update?api_key=" + apiKey + "&field1=" + String(temperature) + "&field2=" + String(humidity) + "&field3=" + String(current) + "&field4=" + String(voltageAC);
  
  if (client.connect("api.thingspeak.com", 80)) {
    client.print("GET " + url + " HTTP/1.1\r\n");
    client.print("Host: api.thingspeak.com\r\n");
    client.print("Connection: close\r\n\r\n");
    Serial.println("Data sent to ThingSpeak");
  } else {
    Serial.println("Failed to connect to ThingSpeak");
  }

  client.stop();
}

float getAverageCurrent() {
  int numSamples = 10;
  float totalCurrent = 0;

  for (int i = 0; i < numSamples; i++) {
    int sensorValue = analogRead(currentSensorPin);
    float voltage = sensorValue * (3.3 / 1023.0);
    float current = (voltage - 2.5) / 0.185;  // ACS712 5A: 185 mV per Ampere
    totalCurrent += current;
    delay(10);
  }

  return totalCurrent / numSamples;
}

float getACVoltage() {
  int numSamples = 100;
  float sumVoltageSquared = 0;

  for (int i = 0; i < numSamples; i++) {
    int sensorValue = analogRead(voltageSensorPin);
    float voltage = (sensorValue * (3.3 / 1023.0)) - 1.65;  // Convert ADC to voltage centered at 0
    sumVoltageSquared += voltage * voltage;
    delay(1);
  }

  float rmsVoltage = sqrt(sumVoltageSquared / numSamples) * 120;
  return rmsVoltage;
}

