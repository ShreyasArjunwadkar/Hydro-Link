#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h" // For token status callback
#include "addons/RTDBHelper.h"  // For real-time database helper
#include "esp_sleep.h"

// Wi-Fi and Firebase credentials
#define USER_EMAIL "user@gmail.com"
#define USER_PASSWORD "user@password"
#define API_KEY "Your-api-key"
#define DATABASE_URL "Your-database-url"

// Network credentials
const char* ssid = "your-ssid";
const char* password = "your-password";

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Ultrasonic sensor pins and constants
#define MAX_DISTANCE 400.0
#define TANK_HEIGHT 400.0
#define LED_PIN 12
#define SLEEP_DURATION 60
#define READ_DURATION 60
#define WIFI_TIMEOUT 30
HardwareSerial Ultrasonic_Sensor(2); // TX2 (pin 17), RX2 (pin 16)

// Variables to hold sensor data
unsigned char data[4] = {};
float distanceCm = MAX_DISTANCE;
float waterLevel = 0.0;
float waterPercentage = 0.0;

unsigned long sendDataPrevMillis = 0;
unsigned long startMillis = 0;
bool readingComplete = false;
int count = 0;
bool signupOK = false;
RTC_DATA_ATTR bool justWokeUp = false;  // RTC memory to track wake-up

void setup() {
  Serial.begin(115200);
  Ultrasonic_Sensor.begin(9600, SERIAL_8N1, 16, 17);  // Initialize UART for sensor
  
  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Check Wake-up Reason
  if (justWokeUp) {
    Serial.println("Just woke up from deep sleep...");
    justWokeUp = false;
  } else {
    Serial.println("Normal boot...");
  }

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  digitalWrite(LED_PIN, HIGH); // LED on while connecting

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > WIFI_TIMEOUT * 1000) {
      Serial.println("WiFi connection timed out.");
      break;
    }
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Failed to connect to WiFi.");
  }

  // Firebase configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Fetch last values from Firebase
  if (Firebase.RTDB.getFloat(&fbdo, "waterlevel/percentage")) {
    waterPercentage = fbdo.floatData();
    Serial.print("Fetched waterPercentage from Firebase: ");
    Serial.println(waterPercentage);
  } else {
    Serial.println("Failed to fetch waterPercentage from Firebase.");
    Serial.println("Reason: " + fbdo.errorReason());
  }

  if (Firebase.RTDB.getInt(&fbdo, "waterlevel/count")) {
    count = fbdo.intData();
    Serial.print("Fetched count from Firebase: ");
    Serial.println(count);
  } else {
    Serial.println("Failed to fetch count from Firebase.");
    Serial.println("Reason: " + fbdo.errorReason());
  }

  // Start Time Tracking
  startMillis = millis();
}

void loop() {
  // Check if Firebase is ready and time to update data
  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Write water level data to Firebase
    if (Firebase.RTDB.setInt(&fbdo, "waterlevel/count", count)) {
      Serial.println("Firebase Int Write PASSED");
    } else {
      Serial.println("Firebase Int Write FAILED");
      Serial.println("Reason: " + fbdo.errorReason());
    }

    if (Firebase.RTDB.setFloat(&fbdo, "waterlevel/percentage", waterPercentage)) {
      Serial.println("Firebase Float Write PASSED");
    } else {
      Serial.println("Firebase Float Write FAILED");
      Serial.println("Reason: " + fbdo.errorReason());
    }
    count++;  // Increment count after writing
  }

  // Sensor Reading Logic
  if (!readingComplete) {
    unsigned long currentMillis = millis();
    if ((currentMillis - startMillis) < (READ_DURATION * 1000)) {
      readSensorData();
    } else {
      readingComplete = true;
      Serial.println("Reading complete. Going to deep sleep...");
      startDeepSleep();
    }
  }
}

// Function to read ultrasonic sensor data
void readSensorData() {
  if (Ultrasonic_Sensor.available() >= 4) {
    for (int i = 0; i < 4; i++) {
      data[i] = Ultrasonic_Sensor.read();
    }

    // Validate sensor data packet
    if (data[0] == 0xff) {
      int sum = (data[0] + data[1] + data[2]) & 0x00FF;
      if (sum == data[3]) {
        distanceCm = ((data[1] << 8) + data[2]) / 10.0;
        distanceCm = constrain(distanceCm, 0.0, MAX_DISTANCE);
        waterLevel = MAX_DISTANCE - distanceCm;
        waterPercentage = (waterLevel / MAX_DISTANCE) * 100;
        waterPercentage = constrain(waterPercentage, 0.0, 100.0);

        Serial.print("Distance: ");
        Serial.print(distanceCm);
        Serial.print(" cm, Water Level: ");
        Serial.print(waterLevel);
        Serial.print(" cm, Water Percentage: ");
        Serial.println(waterPercentage);
      }
    }
  }
}

// Function to put ESP32 into deep sleep
void startDeepSleep() {
  digitalWrite(LED_PIN, LOW);  // Turn off LED before sleeping
  justWokeUp = true;
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 1000000);  // Convert seconds to microseconds
  esp_deep_sleep_start();
}
