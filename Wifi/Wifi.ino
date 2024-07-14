#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// Replace with your network credentials
const char* ssid = "ssid";
const char* password = "pass";

// Static IP address configuration
IPAddress local_IP(192, 168, 0, 17); // Replace with your desired static IP address
IPAddress gateway(192, 168, 0, 1);    // Replace with your network gateway
IPAddress subnet(255, 255, 255, 0);   // Replace with your network subnet mask

// Create an instance of the ESP8266WebServer class
ESP8266WebServer server(80);

// GPIO pins connected to the LEDs
const int ledPin = D7; // Change D7 to the GPIO pin you connected your LED to
const int ledPin0 = D1;

#define EEPROM_SIZE 1  // We only need 1 byte to store the LED state

unsigned long timerDuration = 0;
unsigned long timerStartTime = 0;
bool isTimerRunning = false;

void saveLEDState(bool state) {
  EEPROM.write(0, state ? 1 : 0);
  EEPROM.commit();
}

bool loadLEDState() {
  return EEPROM.read(0) == 1;
}

void handleHome() {
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
  // Calculate time remaining on the timer if it's running
  unsigned long timeRemaining = 0;
  if (isTimerRunning && timerDuration > 0) {
    unsigned long elapsedTime = millis() - timerStartTime;
    if (elapsedTime < timerDuration) {
      timeRemaining = (timerDuration - elapsedTime) / 1000; // Convert to seconds
    }
  }

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
  html += "        if (state === 'on') {";
  html += "          document.getElementById('timer-status').innerText = '';"; // Clear timer status if turning on
  html += "        }";
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
  html += "<p id=\"timer-status\">";
  if (timeRemaining > 0) {
    html += "Timer remaining: " + String(timeRemaining) + " seconds";
  }
  html += "</p>";
  html += "<button class=\"on\" onclick=\"toggleLED('on')\">Turn Bulb On</button>";
  html += "<button class=\"off\" onclick=\"toggleLED('off')\">Turn Bulb Off</button>";
  html += "<p><a href=\"/timer\" class=\"button1\">Set Timer</a></p>";
  html += "<p><a href=\"/\" class=\"button1\">Home</a></p>"; // Home button added
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

void handleLEDOn() {
  digitalWrite(ledPin, HIGH);
  digitalWrite(ledPin0, HIGH);
  saveLEDState(true);
  isTimerRunning = true;
  timerStartTime = millis(); // Start the timer
  server.send(200, "text/plain", "LED is on");
}

void handleLEDOff() {
  digitalWrite(ledPin, LOW); 
  digitalWrite(ledPin0, LOW);
  saveLEDState(false);
  isTimerRunning = false; // Stop the timer
  server.send(200, "text/plain", "LED is off");
}

void handleSetTimer() {
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
  html += "function setTimer() {";
  html += "  var minutes = document.getElementById('timer-input').value;";
  html += "  var xhr = new XMLHttpRequest();";
  html += "  xhr.onreadystatechange = function() {";
  html += "    if (xhr.readyState === XMLHttpRequest.DONE) {";
  html += "      if (xhr.status === 200) {";
  html += "        document.getElementById('timer-status').innerText = 'Timer set for ' + minutes + ' minutes';";
  html += "      } else {";
  html += "        alert('Failed to set timer');";
  html += "      }";
  html += "    }";
  html += "  };";
  html += "  xhr.open('GET', '/set_timer?minutes=' + minutes, true);";
  html += "  xhr.send();";
  html += "}";
  html += "</script>";
  html += "</head><body>";
  html += "<div class=\"container\">";
  html += "<h1>Set Timer</h1>";
  html += "<input type=\"number\" id=\"timer-input\" placeholder=\"Enter minutes\">";
  html += "<button class=\"on\" onclick=\"setTimer()\">Set Timer</button>";
  html += "<p id=\"timer-status\"></p>";
  html += "<p><a href=\"/\" class=\"button1\">Home</a></p>";
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

void handleSetTimerRequest() {
  if (server.hasArg("minutes")) {
    int minutes = server.arg("minutes").toInt();
    if (minutes > 0) {
      timerDuration = minutes * 60000; // Convert minutes to milliseconds
      server.send(200, "text/plain", "Timer set for " + String(minutes) + " minutes");
    } else {
      server.send(400, "text/plain", "Bad Request: Invalid minutes parameter");
    }
  } else {
    server.send(400, "text/plain", "Bad Request: Missing minutes parameter");
  }
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
  server.on("/", HTTP_GET, handleHome);
  server.on("/bulb", HTTP_GET, handleRoot);
  server.on("/bulb/", HTTP_GET, handleRoot);
  server.on("/bulb/on", HTTP_GET, handleLEDOn);
  server.on("/bulb/off", HTTP_GET, handleLEDOff);
  server.on("/timer", HTTP_GET, handleSetTimer);
  server.on("/set_timer", HTTP_GET, handleSetTimerRequest);

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  // Check if the timer has expired
  if (isTimerRunning && timerDuration > 0 && millis() - timerStartTime >= timerDuration) {
    digitalWrite(ledPin, LOW);
    digitalWrite(ledPin0, LOW);
    saveLEDState(false);
    isTimerRunning = false; // Stop the timer
    Serial.println("Timer expired, turning off LED");
  }
}
