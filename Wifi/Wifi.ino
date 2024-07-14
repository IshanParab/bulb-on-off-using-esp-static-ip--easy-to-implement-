#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// Replace with your network credentials
const char* ssid = "ssid";
const char* password = "password";

// Static IP address configuration
IPAddress local_IP(192, 168, 0, 17); // Replace with your desired static IP address
IPAddress gateway(192, 168, 0, 1);    // Replace with your network gateway
IPAddress subnet(255, 255, 255, 0);   // Replace with your network subnet mask

// Create an instance of the ESP8266WebServer class
ESP8266WebServer server(80);

// GPIO pin connected to the LED
const int ledPin = D7; // Change D7 to the GPIO pin you connected your LED to
const int ledPin0 = D1; 
#define EEPROM_SIZE 1  // We only need 1 byte to store the LED state

void saveLEDState(bool state) {
  EEPROM.write(0, state ? 1 : 0);
  EEPROM.commit();
}

bool loadLEDState() {
  return EEPROM.read(0) == 1;
}
void handlehome() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<link rel=\"manifest\" href=\"/manifest.json\">";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f7f7f7; margin: 0; padding: 20px; }";
  html += ".container { max-width: 600px; margin: 0 auto; background: #fff; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
  html += "h1 { font-size: 24px; margin-bottom: 20px; }";
  html += "button { font-size: 20px; padding: 10px 20px; margin: 10px; cursor: pointer; border: none; border-radius: 5px; background-color: #4CAF50; color: white; }";
  html += ".bulb-symbol { font-size: 24px; margin-right: 10px; }"; // Style for the bulb symbol
  html += "</style>";
  html += "</head><body>";
  html += "<div class=\"container\">";
  html += "<h1>Welcome to ESP8266 Web Server</h1>";
  html += "<p>Control the bulb:</p>";
  html += "<a href=\"/bulb\"><button><span class=\"bulb-symbol\">&#128161;</span> Bulb Control</button></a>"; // Unicode for bulb symbol
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}


void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<link rel=\"manifest\" href=\"/manifest.json\">";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; text-align: center; background-color: #f7f7f7; margin: 0; padding: 20px; }";
  html += ".container { max-width: 600px; margin: 0 auto; background: #fff; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }";
  html += "h1 { font-size: 24px; margin-bottom: 20px; }";
  html += "button { font-size: 20px; padding: 10px 20px; margin: 10px; cursor: pointer; border: none; border-radius: 5px; }";
  html += ".on { background-color: #4CAF50; color: white; }";
  html += ".off { background-color: #f44336; color: white; }";
  html += "</style>";
  html += "<script>if('serviceWorker' in navigator){navigator.serviceWorker.register('/sw.js');}</script>";
  html += "<script>";
  html += "function toggleLED(state) {";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (xhr.readyState === XMLHttpRequest.DONE) {";
  html += "      if (xhr.status === 200) {";
  html += "        document.getElementById('led-status').innerText = xhr.responseText;";
  html += "      } else {";
  html += "        alert('Failed to toggle LED state');";
  html += "      }";
  html += "    }";
  html += "  };";
  html += "  xhr.open('GET', '/bulb/' + state, true);";
  html += "  xhr.send();";
  html += "}";
  html += "</script>";
  html += "</head><body>";
  html += "<div class=\"container\">";
  html += "<h1>Bulb Control</h1>";
  html += "<p id=\"led-status\">LED is ";
  html += (digitalRead(ledPin) == HIGH ? "on" : "off");
  html += "</p>";
  html += "<button class=\"on\" onclick=\"toggleLED('on')\">Turn Bulb On</button>";
  html += "<button class=\"off\" onclick=\"toggleLED('off')\">Turn Bulb Off</button>";
   html += "<p><a href=\"/\" class=\"button1\">Home</a></p>";
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

void handleLEDOn() {
  digitalWrite(ledPin, HIGH);
  digitalWrite(ledPin0, HIGH);
  saveLEDState(true);
  server.send(200, "text/plain", "LED is on");
}

void handleLEDOff() {
  digitalWrite(ledPin, LOW); 
  digitalWrite(ledPin0, LOW);
  saveLEDState(false);
  server.send(200, "text/plain", "LED is off");
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
pinMode(ledPin0, OUTPUT);
  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // Set the LED to the last saved state
  digitalWrite(ledPin, loadLEDState() ? HIGH : LOW);

  // Configure static IP address
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Print ESP8266 local IP address
  Serial.println(WiFi.localIP());

  // Handle HTTP requests
  server.on("/bulb", HTTP_GET, handleRoot);
  server.on("/bulb/", HTTP_GET, handleRoot);
  server.on("/bulb/on", HTTP_GET, handleLEDOn);
  server.on("/bulb/off", HTTP_GET, handleLEDOff);
 server.on("/", handlehome);
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
