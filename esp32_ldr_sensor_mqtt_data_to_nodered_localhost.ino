#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>  

const int LDRPin = 34;   
const int LEDPin = 27;   

// SSID dan password WiFi
const char* ssid = "wifi";
const char* password = "password";

// Pengaturan broker MQTT
const char* mqtt_server = "192.168.(your ip local host)";  
const int mqtt_port = 1883;                
//NO PASSWORD
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  
  Serial.begin(115200);

  pinMode(LEDPin, OUTPUT);


  pinMode(LDRPin, INPUT);

  Serial.print("Menghubungkan ke WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nTerhubung ke WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  // Mengatur broker MQTT
  client.setServer(mqtt_server, mqtt_port);
}

void reconnect() {
  // Loop sampai terhubung kembali
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Terhubung!");
    } else {
      Serial.print("Gagal, rc=");
      Serial.print(client.state());
      Serial.println(" mencoba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

void loop() {
  // Pastikan koneksi MQTT tetap tersambung
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Membaca nilai analog dari LDR
  int LDRValue = analogRead(LDRPin);
  Serial.print("LDR Value: ");
  Serial.println(LDRValue);

  // Membuat objek JSON untuk payload
  StaticJsonDocument<200> doc;
  doc["ldr_value"] = LDRValue;  // Menyimpan nilai LDR ke JSON

  // Tentukan status berdasarkan nilai LDR
  if (LDRValue > 1000) {
    doc["status"] = "Cahaya gelap";  // Status gelap
    digitalWrite(LEDPin, HIGH);      // LED menyala
    Serial.println("Cahaya gelap - LED ON");
  } else {
    doc["status"] = "Cahaya terang"; // Status terang
    digitalWrite(LEDPin, LOW);       // LED mati
    Serial.println("Cahaya terang - LED OFF");
  }

  // Mengubah objek JSON menjadi string
  String output;
  serializeJson(doc, output);

  // Mengirimkan JSON ke broker MQTT
  client.publish("sensor/ldr", output.c_str());

  delay(500);  
}
