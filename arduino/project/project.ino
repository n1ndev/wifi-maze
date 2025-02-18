#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>

const char* ssid = "ConditionsMesh";
const char* password = "Conditions1189";

// The device will be accessible as http://qtpy.local
const char* hostname = "qtpy";

// Pre-shared secret token
const char* SECRET_TOKEN = "MY_SECRET_TOKEN";

WebServer server(80);

// Handler for the root URL (for testing)
void handleRoot() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "text/plain", "Hello from QT Py S3");
}

// Handler for /command?cmd=...&token=...
void handleCommand() {
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
  Serial.println();

  // Connect to WiFi network
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Start mDNS with the hostname "qtpy"
  if (!MDNS.begin(hostname)) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.print("mDNS responder started: http://");
  Serial.print(hostname);
  Serial.println(".local");

  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/command", handleCommand);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
