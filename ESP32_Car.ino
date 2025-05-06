#include <WiFi.h>
#include <WebServer.h>
#include <Servo.h>

// Replace with your WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Motor pins
#define M1_IN1 5
#define M1_IN2 18
#define M2_IN1 19
#define M2_IN2 21
#define M3_IN1 22
#define M3_IN2 23
#define M4_IN1 32
#define M4_IN2 33

// Servo pins
#define PAN_PIN 25
#define TILT_PIN 26
Servo panServo, tiltServo;

WebServer server(80);

// -------- Motor Control --------
void stopMotors() {
  digitalWrite(M1_IN1, LOW); digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN1, LOW); digitalWrite(M2_IN2, LOW);
  digitalWrite(M3_IN1, LOW); digitalWrite(M3_IN2, LOW);
  digitalWrite(M4_IN1, LOW); digitalWrite(M4_IN2, LOW);
}

void moveForward() {
  digitalWrite(M1_IN1, HIGH); digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN1, HIGH); digitalWrite(M2_IN2, LOW);
  digitalWrite(M3_IN1, HIGH); digitalWrite(M3_IN2, LOW);
  digitalWrite(M4_IN1, HIGH); digitalWrite(M4_IN2, LOW);
}

void moveBackward() {
  digitalWrite(M1_IN1, LOW); digitalWrite(M1_IN2, HIGH);
  digitalWrite(M2_IN1, LOW); digitalWrite(M2_IN2, HIGH);
  digitalWrite(M3_IN1, LOW); digitalWrite(M3_IN2, HIGH);
  digitalWrite(M4_IN1, LOW); digitalWrite(M4_IN2, HIGH);
}

void turnLeft() {
  digitalWrite(M1_IN1, LOW); digitalWrite(M1_IN2, HIGH);
  digitalWrite(M2_IN1, LOW); digitalWrite(M2_IN2, HIGH);
  digitalWrite(M3_IN1, HIGH); digitalWrite(M3_IN2, LOW);
  digitalWrite(M4_IN1, HIGH); digitalWrite(M4_IN2, LOW);
}

void turnRight() {
  digitalWrite(M1_IN1, HIGH); digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN1, HIGH); digitalWrite(M2_IN2, LOW);
  digitalWrite(M3_IN1, LOW); digitalWrite(M3_IN2, HIGH);
  digitalWrite(M4_IN1, LOW); digitalWrite(M4_IN2, HIGH);
}

// -------- Handlers --------
void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
    <html><head><style>
    button { width: 100px; height: 50px; font-size: 16px; margin: 5px; }
    </style></head><body>
    <h2>ESP32 Car Control</h2>
    <button onclick="fetch('/forward')">Forward</button><br>
    <button onclick="fetch('/left')">Left</button>
    <button onclick="fetch('/stop')">Stop</button>
    <button onclick="fetch('/right')">Right</button><br>
    <button onclick="fetch('/backward')">Backward</button><br><br>
    <label>Pan: </label><input type="range" min="0" max="180" onchange="fetch('/pan?angle='+this.value)">
    <br>
    <label>Tilt: </label><input type="range" min="0" max="180" onchange="fetch('/tilt?angle='+this.value)">
    </body></html>
  )rawliteral");
}

void handleMovement(String cmd) {
  stopMotors();
  if (cmd == "forward") moveForward();
  else if (cmd == "backward") moveBackward();
  else if (cmd == "left") turnLeft();
  else if (cmd == "right") turnRight();
  server.send(200, "text/plain", "OK");
}

void handlePan() {
  if (server.hasArg("angle")) {
    int angle = server.arg("angle").toInt();
    panServo.write(constrain(angle, 0, 180));
  }
  server.send(200, "text/plain", "Pan OK");
}

void handleTilt() {
  if (server.hasArg("angle")) {
    int angle = server.arg("angle").toInt();
    tiltServo.write(constrain(angle, 0, 180));
  }
  server.send(200, "text/plain", "Tilt OK");
}

void setup() {
  Serial.begin(115200);

  // Motor pins
  int motorPins[] = {M1_IN1, M1_IN2, M2_IN1, M2_IN2, M3_IN1, M3_IN2, M4_IN1, M4_IN2};
  for (int i = 0; i < 8; i++) pinMode(motorPins[i], OUTPUT);

  // Servo setup
  panServo.attach(PAN_PIN);
  tiltServo.attach(TILT_PIN);
  panServo.write(90);
  tiltServo.write(90);

  // Connect WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.println(WiFi.localIP());

  // Server routes
  server.on("/", handleRoot);
  server.on("/forward", []() { handleMovement("forward"); });
  server.on("/backward", []() { handleMovement("backward"); });
  server.on("/left", []() { handleMovement("left"); });
  server.on("/right", []() { handleMovement("right"); });
  server.on("/stop", []() { stopMotors(); server.send(200, "text/plain", "Stopped"); });
  server.on("/pan", handlePan);
  server.on("/tilt", handleTilt);

  server.begin();
}

void loop() {
  server.handleClient();
}
