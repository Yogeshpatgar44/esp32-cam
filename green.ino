#include <WiFi.h>  
#include <FirebaseESP32.h>
#include <DHT.h>

// Replace with your Wi-Fi credentials
const char* ssid = "Yogesh";
const char* password = "20240324";

// Firebase configuration
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

// DHT sensor setup
#define DHTPIN 4  // DHT22 sensor pin
DHT dht(DHTPIN, DHT22);

// Actuators setup
#define FAN_PIN 13        // Exhaust fan pin
#define LED_PIN 12        // LED pin
#define PUMP_PIN 14       // Water pump pin

// Thresholds
#define TEMP_THRESHOLD 25 // Temperature below 25°C, turn on LED
#define HUMIDITY_THRESHOLD 40  // Humidity below 40%, turn on exhaust fan
#define SOIL_MOISTURE_THRESHOLD 500 // Soil moisture below 500, turn on water pump
#define GAS_THRESHOLD 300  // Gas level threshold to activate the exhaust fan

// MQ2 gas sensor setup
#define GAS_SENSOR_PIN 34  // Pin for MQ2 sensor

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Set Firebase host and authentication token
  config.host = "final-a34b8-default-rtdb.asia-southeast1.firebasedatabase.app";
  config.signer.tokens.legacy_token = "YgAQq7nVLmcq0LTAXw1fNlMRU5abrKvcejl7WSGPRT";
  
  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Initialize DHT sensor
  dht.begin();

  // Initialize actuators
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(GAS_SENSOR_PIN, INPUT);  // Initialize MQ2 sensor pin
}

void loop() {
  // Read temperature and humidity
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Read moisture from soil sensor (use appropriate pin for your soil sensor)
  int soil_moisture = analogRead(36);  // Example pin for soil sensor

  // Read gas level from MQ2 sensor
  int gas_level = analogRead(GAS_SENSOR_PIN);

  // Send sensor data to Firebase
  Firebase.setFloat(firebaseData, "greenhouse/sensors/temperature", temperature);
  Firebase.setFloat(firebaseData, "greenhouse/sensors/humidity", humidity);
  Firebase.setInt(firebaseData, "greenhouse/sensors/soil_moisture", soil_moisture);
  Firebase.setInt(firebaseData, "greenhouse/sensors/gas_level", gas_level);  // Send gas level to Firebase

  // Actuator control logic:
  // 1. Soil Moisture Control
  if (soil_moisture < SOIL_MOISTURE_THRESHOLD) {  // Soil is dry, turn on water pump
    digitalWrite(PUMP_PIN, HIGH);
    Firebase.setBool(firebaseData, "greenhouse/actuators/pump", true);
  } else {  // Soil is wet enough, turn off water pump
    digitalWrite(PUMP_PIN, LOW);
    Firebase.setBool(firebaseData, "greenhouse/actuators/pump", false);
  }

  // 2. Temperature Control
  if (temperature < TEMP_THRESHOLD) {  // Temperature is low, turn on LED
    digitalWrite(LED_PIN, HIGH);
    Firebase.setBool(firebaseData, "greenhouse/actuators/led", true);
  } else {  // Temperature is fine, turn off LED
    digitalWrite(LED_PIN, LOW);
    Firebase.setBool(firebaseData, "greenhouse/actuators/led", false);
  }

  // 3. Humidity Control
  if (humidity < HUMIDITY_THRESHOLD) {  // If humidity is low, turn on exhaust fan
    digitalWrite(FAN_PIN, HIGH);
    Firebase.setBool(firebaseData, "greenhouse/actuators/fan", true);
  } else {  // Conditions normal, turn off exhaust fan
    digitalWrite(FAN_PIN, LOW);
    Firebase.setBool(firebaseData, "greenhouse/actuators/fan", false);
  }

  // 4. Gas Level Control
  if (gas_level > GAS_THRESHOLD) {  // Gas level is high, turn on exhaust fan
    digitalWrite(FAN_PIN, HIGH);
    Firebase.setBool(firebaseData, "greenhouse/actuators/fan", true);
  } else {  // Gas level is normal, turn off exhaust fan
    digitalWrite(FAN_PIN, LOW);
    Firebase.setBool(firebaseData, "greenhouse/actuators/fan", false);
  }

  // Delay between readings
  delay(2000);  // Send data every 2 seconds
}
