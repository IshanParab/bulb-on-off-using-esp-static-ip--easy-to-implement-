#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
const char* ssid = "ssid(wifiname)";
const char* password = "password";
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi!");
}
server.on("/", handleRoot);
server.on("/led/on", handleLedOn);
server.on("/led/off", handleLedOff);
server.begin();
}
Serial.println("Web server started!");
void handleRoot() {
  server.send(200, "text/html", "<h1>Hello from ESP8266!</h1>");
}

void handleLedOn() {
  // Code to turn LED on goes here
  server.send(200, "text/plain", "LED turned on");
}

void handleLedOff() {
  // Code to turn LED off goes here
  server.send(200, "text/plain", "LED turned off");
}



void loop() {
  server.handleClient();
}
