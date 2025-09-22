#include <DHT.h>

#define DHTPIN 2           // GPIO2 / D4 on NodeMCU
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int currentSensorPin = A0;  // ACS712 current sensor pin
const int voltageSensorPin = A0;  // ZMPT101B voltage sensor pin (you may need to change this if using separate pins)

// Calibration constants - Adjust these as per your setup
const float ACS712_MIDPOINT = 1.65;      // Midpoint for 3.3V systems
const float ACS712_SENSITIVITY = 0.185;  // Sensitivity for ACS712-5A: 185 mV per Amp
const float ZMPT101B_SCALING = 120;      // Scaling factor for ZMPT101B - adjust based on your calibration

void setup() {
  Serial.begin(115200);
  dht.begin();
}

void loop() {
  // Read temperature and humidity from DHT11
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  }

  // Read current from ACS712
  float current = getAverageCurrent();
  Serial.print("Current: ");
  Serial.print(current);
  Serial.println(" A");

  // Read AC voltage from ZMPT101B
  float voltageAC = getACVoltage();
  Serial.print("AC Voltage: ");
  Serial.print(voltageAC);
  Serial.println(" V");

  delay(2000);  // Delay between readings for sensor stability
}

// Function to get the average current using ACS712 sensor
float getAverageCurrent() {
  int numSamples = 100; // Increase sample count for better averaging
  float totalCurrent = 0;
  
  for (int i = 0; i < numSamples; i++) {
    int sensorValue = analogRead(currentSensorPin);
    float voltage = sensorValue * (3.3 / 1023.0);  // Convert ADC value to voltage
    float current = (voltage - ACS712_MIDPOINT) / ACS712_SENSITIVITY; // Convert voltage to current
    totalCurrent += current;
    delay(1);  // Reduced delay to improve sampling rate
  }
  
  // Average current calculation
  float averageCurrent = totalCurrent / numSamples;
  return max(averageCurrent, 0.0f);  // Ensure no negative values
}

// Function to get AC voltage using ZMPT101B sensor
float getACVoltage() {
  int numSamples = 300; // Further increase sample count for better RMS calculation
  float sumVoltageSquared = 0;
  
  for (int i = 0; i < numSamples; i++) {
    int sensorValue = analogRead(voltageSensorPin);
    float voltage = (sensorValue * (3.3 / 1023.0)) - 1.65; // Convert ADC to voltage, then center around 0
    sumVoltageSquared += voltage * voltage;
    delay(1);
  }
  
  // Calculate RMS voltage
  float rmsVoltage = sqrt(sumVoltageSquared / numSamples) * ZMPT101B_SCALING;
  return rmsVoltage;
}
