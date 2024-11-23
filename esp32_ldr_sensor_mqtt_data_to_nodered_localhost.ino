#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>  

const int LDRPin = 34;    // Pin connected to the LDR sensor
const int LEDPin = 27;    // Pin connected to the LED

// WiFi SSID and password
const char* ssid = "okyy";               // Replace with your WiFi SSID
const char* password = "123456aerrrr";   // Replace with your WiFi password

// MQTT broker configuration
const char* mqtt_server = "192.... your localhost ip";  // Replace with your MQTT broker IP address
const int mqtt_port = 1883;                 // MQTT port number
// NO PASSWORD REQUIRED
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200); // Initialize serial communication
  pinMode(LEDPin, OUTPUT); // Set LEDPin as output
  pinMode(LDRPin, INPUT);  // Set LDRPin as input

  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password); // Start WiFi connection

  // Wait until connected to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Print the device's local IP address

  // Set MQTT broker details
  client.setServer(mqtt_server, mqtt_port);
}

void reconnect() {
  // Loop until reconnected to the MQTT broker
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Client")) { // Attempt to connect as "ESP32Client"
      Serial.println("Connected!");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state()); // Print connection error state
      Serial.println(" retrying in 5 seconds");
      delay(5000); // Wait before retrying
    }
  }
}

void loop() {
  // Ensure the MQTT connection is maintained
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read the analog value from the LDR
  int LDRValue = analogRead(LDRPin);
  Serial.print("LDR Value: ");
  Serial.println(LDRValue);

  // Create a JSON object for the payload
  StaticJsonDocument<200> doc;
  doc["ldr_value"] = LDRValue;  // Store the LDR value in the JSON object

  // Determine the status based on the LDR value
  if (LDRValue > 1000) {
    doc["status"] = "Dark";     // Status: Dark
    digitalWrite(LEDPin, HIGH); // Turn on the LED
    Serial.println("Dark - LED ON");
  } else {
    doc["status"] = "Bright";   // Status: Bright
    digitalWrite(LEDPin, LOW);  // Turn off the LED
    Serial.println("Bright - LED OFF");
  }

  // Convert the JSON object to a string
  String output;
  serializeJson(doc, output);

  // Publish the JSON payload to the MQTT broker
  client.publish("sensor/ldr", output.c_str());

  delay(500);  // Wait for 500 ms before the next reading
}
