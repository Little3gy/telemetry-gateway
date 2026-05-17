#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// wifi credentials
const char* ssid = "ruski";
const char* password = "nachhoi123";

// UDP settings
const int udpPort = 4210;
WiFiUDP udp;

void setup() {
  // put your setup code here, to run once:
  
   Serial.begin(115200);   // USB debug to laptop
    Serial2.begin(115200);  // UART from STM32
  
  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi... ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  // Start UDP
  udp.begin(udpPort);
  Serial.print("UDP server started on port ");

}


void loop() {
  // put your main code here, to run repeatedly:

  if (Serial2.available()) {
        String data = Serial2.readStringUntil('\n');
        Serial.println("Received: " + data);
  
      // Send data over UDP
      udp.beginPacket("255.255.255.255", udpPort);
      udp.print(data);
      udp.endPacket();

  }

}

// On the ESP32, UART2 defaults to GPIO16 (RX2) and GPIO17 (TX2). https://www.botnroll.com/en/esp32/3639-wemos-d1-r32-w-esp32-uno-r3-pinout.html