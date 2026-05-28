#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <LoRa.h>


// LoRa pins
#define LORA_SS    5
#define LORA_RST   14
#define LORA_DIO0  26

// Wi-Fi credentials
const char* ssid = "ruski";
const char* password = "nachhoi123";

// UDP settings
const int udpPort = 4210;
WiFiUDP udp;

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200);  // UART from STM32

    // Connect to Wi-Fi
    Serial.println("Connecting to Wi-Fi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi connected!");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());

    udp.begin(udpPort);

    // LoRa Protocol

      LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa init failed");
  }
    else {
        Serial.println("LoRa init succeeded");
}


}

void loop() {
    if (Serial2.available()) {
        String data = Serial2.readStringUntil('\n');
        Serial.println("Received: " + data);

        // Broadcast over UDP
        udp.beginPacket("255.255.255.255", udpPort);
        udp.print(data);
        udp.endPacket();
    }

    // Check for incoming LoRa packets
    int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("LoRa RX: ");
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }
    Serial.print(" | RSSI: ");
    Serial.println(LoRa.packetRssi());
  }
    
}