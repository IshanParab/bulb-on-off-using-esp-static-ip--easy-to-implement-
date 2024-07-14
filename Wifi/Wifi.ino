#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

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

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Ensure the LED is off initially

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

  // Handle HTTP GET requests
  server.on("/", HTTP_GET, []() {
    String html = "<html><body>";
    html += "<h1 style=\"font-size: 70px;\">Bulb Control</h1>";
    html += "<p style=\"font-size: 500px;\">Click <a href=\"/led/on\">here</a> to turn bulb on</p>";
    html += "<p style=\"font-size: 50px;\">Click <a href=\"/led/off\">here</a> to turn bulb off</p>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });

  // Handle HTTP POST requests to control LED
  server.on("/led/on", HTTP_GET, []() {
    digitalWrite(ledPin, HIGH);
    server.sendHeader("Location", "/");
    server.send(303); // 303 See Other to redirect to home page
  });

  server.on("/led/off", HTTP_GET, []() {
    digitalWrite(ledPin, LOW);
    server.sendHeader("Location", "/");
    server.send(303); // 303 See Other to redirect to home page
  });

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
