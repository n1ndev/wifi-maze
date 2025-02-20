#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>

// Replace with your WiFi credentials
const char* ssid = "your_network_ssid";
const char* password = "your_network_password";

// mDNS hostname – the device will be accessible as http://qtpy.local
const char* hostname = "qtpy";

// Pre-shared secret token for authentication
const char* SECRET_TOKEN = "MY_SECRET_TOKEN";

// Create a web server on port 80
WebServer server(80);

// Handler for the root URL (for testing)
void handleRoot() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Hello from QT Py S3");
}

// Handler for /command endpoint: expects "cmd" and "token" parameters.
void handleCommand() {
  // Check for token
  if (!server.hasArg("token")) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(401, "text/plain", "Token required");
    return;
  }
  String token = server.arg("token");
  if (token != SECRET_TOKEN) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(403, "text/plain", "Invalid token");
    return;
  }
  
  // Process command if provided
  if (server.hasArg("cmd")) {
    String cmd = server.arg("cmd");
    Serial.print("Received command: ");
    Serial.println(cmd);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "text/plain", "Command received: " + cmd);
  } else {
    server.sendHeader("Access-Control-Allow-Origin", "*");
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
  
  // Start mDNS so the device is reachable as "qtpy.local"
  if (!MDNS.begin(hostname)) {
    Serial.println("Error starting mDNS");
    while (1) { delay(1000); }
  }
  Serial.print("mDNS responder started: http://");
  Serial.print(hostname);
  Serial.println(".local");
  
  // Define HTTP server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/command", HTTP_GET, handleCommand);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
