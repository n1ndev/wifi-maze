#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>

// Replace with your WiFi credentials
const char* ssid = "network";
const char* password = "password";

// mDNS hostname – device will be accessible as http://qtpy.local
const char* hostname = "qtpy";

// Create a global variable to store the device's MAC address
String deviceMac;

// Create a web server on port 80
WebServer server(80);

// Helper function to add required headers for CORS and Private Network Access
void addCORSHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Private-Network", "true");
  server.sendHeader("Access-Control-Allow-Methods", "GET,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "*");
  
  // New headers required by the PNA proposal:
  server.sendHeader("Private-Network-Access-Name", "qtpy");
  // Use the device's MAC address as the ID header.
  server.sendHeader("Private-Network-Access-ID", deviceMac);
}

// Handler for OPTIONS preflight requests
void handleOptions() {
  addCORSHeaders();
  server.send(200, "text/plain", "");
}

// Handler for the root URL (for testing)
void handleRoot() {
  addCORSHeaders();
  server.send(200, "text/plain", "Hello from QT Py S3");
}

// Handler for /command endpoint: expects "cmd" and "token" parameters.
void handleCommand() {
  addCORSHeaders();
  
  // Check for token parameter
  if (!server.hasArg("token")) {
    server.send(401, "text/plain", "Token required");
    return;
  }
  String token = server.arg("token");
  if (token != "MY_SECRET_TOKEN") {  // Replace with your actual secret token
    server.send(403, "text/plain", "Invalid token");
    return;
  }
  
  // Process the command parameter
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");
    Serial.print("Received command: ");
    Serial.println(cmd);
    server.send(200, "text/plain", "Command received: " + cmd);
  } else {
    server.send(400, "text/plain", "No command received");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected. IP: ");
  Serial.println(WiFi.localIP());
  
  // Store the device's MAC address in the global variable
  deviceMac = WiFi.macAddress();
  Serial.print("Device MAC Address: ");
  Serial.println(deviceMac);
  
  // Start mDNS so the device is reachable as "qtpy.local"
  if (!MDNS.begin(hostname)) {
    Serial.println("Error starting mDNS");
    while (1) { delay(1000); }
  }
  Serial.print("mDNS responder started: http://");
  Serial.print(hostname);
  Serial.println(".local");
  
  // Define HTTP server routes and handle OPTIONS preflight requests
  server.on("/", HTTP_GET, handleRoot);
  server.on("/", HTTP_OPTIONS, handleOptions);
  
  server.on("/command", HTTP_GET, handleCommand);
  server.on("/command", HTTP_OPTIONS, handleOptions);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
