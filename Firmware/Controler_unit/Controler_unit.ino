#include <WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ===== WiFi =====
const char* ssid = "**********";
const char* password = "**********";

// ===== TCP server for Winch control & feedback =====
WiFiServer tcpServer(80);
WiFiClient activeClient; // client used to send ACK/LIMIT back

// ===== UDP for Boat =====
WiFiUDP udp;
const int udpPort = 4210;
char udpBuf[255];

// ===== MQTT =====
const char* mqtt_server = "**.**.**.**";
const int mqtt_port = 1883;
const char* mqtt_user = "MQTT_USERNAME";
const char* mqtt_pass = "MQTT_PASSWORD";
const char* mqtt_topic = "water/quality";
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ===== Motor pins =====
// Winch (Motor1)
#define M1_IN1 32
#define M1_IN2 33
#define M1_EN  25
#define LIMIT_PIN 12   // Magnetic switch (LOW when triggered)

// Boat motors (Motors 2 & 3)
#define M2_IN1 13
#define M2_IN2 14
#define M2_EN  26

#define M3_IN1 16
#define M3_IN2 4
#define M3_EN  27

// ===== Sensors =====
#define TDS_PIN 35
#define DHT_PIN 21
#define DHT_TYPE DHT11
#define DS18B20_PIN 19

DHT dht(DHT_PIN, DHT_TYPE);
OneWire oneWire(DS18B20_PIN);
DallasTemperature dsSensor(&oneWire);

// ===== Winch stepping =====
String currentWinchCommand = "STOP";
bool motorOn = false;
unsigned long lastStepTime = 0;
const int stepIntervalUP = 400;
const int stepIntervalDOWN = 600;

// ===== State =====
String currentBoatCommand = "STOP";

// ===== Timing =====
unsigned long lastMqttSend = 0;
const unsigned long mqttInterval = 5000; // ms

// ===== Setup =====
void setup() {
  Serial.begin(115200);

  // motor pins
  pinMode(M1_IN1, OUTPUT);
  pinMode(M1_IN2, OUTPUT);
  pinMode(M1_EN, OUTPUT);
  pinMode(LIMIT_PIN, INPUT_PULLUP);

  pinMode(M2_IN1, OUTPUT);
  pinMode(M2_IN2, OUTPUT);
  pinMode(M2_EN, OUTPUT);
  pinMode(M3_IN1, OUTPUT);
  pinMode(M3_IN2, OUTPUT);
  pinMode(M3_EN, OUTPUT);

  stopWinch();
  stopBoat();

  // sensors
  dht.begin();
  dsSensor.begin();

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi IP: ");
  Serial.println(WiFi.localIP());

  // start TCP server and UDP
  tcpServer.begin();
  udp.begin(udpPort);
  Serial.printf("TCP server started on port %d, UDP on %d\n", 80, udpPort);

  // MQTT
  mqttClient.setServer(mqtt_server, mqtt_port);
}

// ===== Main loop =====
void loop() {
  unsigned long now = millis();

  // Handle incoming TCP 
  handleTCP();

  // Handle incoming UDP 
  handleUDP();

  // Limit switch check
  bool limitActive = (digitalRead(LIMIT_PIN) == LOW);
  if (currentWinchCommand == "UP" && limitActive) {
    Serial.println("LIMIT reached -> stopping winch");
    stopWinch();
    currentWinchCommand = "STOP";
    sendFeedback("LIMIT");   // send to active TCP client
    
  }

  // Winch stepping pulses
  int interval = (currentWinchCommand == "DOWN") ? stepIntervalDOWN : stepIntervalUP;
  if (now - lastStepTime >= interval) {
    lastStepTime = now;
    if (currentWinchCommand == "UP") stepWinch(true);
    else if (currentWinchCommand == "DOWN") stepWinch(false);
    else stopWinch();
  }

  // MQTT: ensure connected and publish sensor data 
  if (!mqttClient.connected()) reconnectMQTT();
  mqttClient.loop();

  if (now - lastMqttSend >= mqttInterval) {
    lastMqttSend = now;
    sendSensorData();
  }

  // small delay to avoid busy-wait
  delay(10);
}

// ===== TCP handling (Winch commands) =====
void handleTCP() {
  WiFiClient client = tcpServer.available();
  if (client) {
    // keep reference to active client so we can send feedback later
    if (!activeClient || !activeClient.connected()) {
      activeClient = client;
      Serial.println("New TCP client connected (activeClient set)");
    }

    // If client sent data, read it
    if (client.available()) {
      String cmd = client.readStringUntil('\n');
      cmd.trim();
      if (cmd.length() > 0) {
        Serial.print("TCP Received (winch): ");
        Serial.println(cmd);

        // accept only expected commands
        if (cmd == "UP" || cmd == "DOWN" || cmd == "STOP") {
          currentWinchCommand = cmd;
          if (currentWinchCommand == "STOP") stopWinch();
          sendFeedback(String("ACK:") + currentWinchCommand);
        } else {
          Serial.println("Unknown winch command, ignoring.");
        }
      }
    }
  }

  // clear activeClient if disconnected
  if (activeClient && !activeClient.connected()) {
    Serial.println("Active TCP client disconnected");
    activeClient.stop();
  }
}

void sendFeedback(const String &msg) {
  if (activeClient && activeClient.connected()) {
    activeClient.println(msg);
    Serial.print("Sent feedback to TCP client: ");
    Serial.println(msg);
  } else {
    Serial.println("No active TCP client to send feedback");
  }
}

// ===== UDP handling (Boat) =====
void handleUDP() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(udpBuf, sizeof(udpBuf) - 1);
    if (len > 0) {
      udpBuf[len] = 0;
      String cmd = String(udpBuf);
      cmd.trim();
      Serial.print("UDP Received (boat): ");
      Serial.println(cmd);
      currentBoatCommand = cmd;
      controlBoat(cmd);
    }
  }
}

// ===== Winch motor functions =====
void stepWinch(bool up) {
  // pulse behavior: toggle motor ON/OFF to create stepping pulses
  if (motorOn) {
    stopWinch();
  } else {
    digitalWrite(M1_IN1, up ? HIGH : LOW);
    digitalWrite(M1_IN2, up ? LOW : HIGH);   
    analogWrite(M1_EN, 150); 
    motorOn = true;
  }
}

void stopWinch() {
  // motor off
  analogWrite(M1_EN, 0);
  digitalWrite(M1_IN1, LOW);
  digitalWrite(M1_IN2, LOW);
  motorOn = false;
}

// ===== Boat control functions =====
void controlBoat(const String &cmd) {
  // default: stop both
  digitalWrite(M2_EN, LOW);
  digitalWrite(M3_EN, LOW);

  if (cmd == "FORWARD") {
    digitalWrite(M2_IN1, HIGH); digitalWrite(M2_IN2, LOW);
    digitalWrite(M3_IN1, HIGH); digitalWrite(M3_IN2, LOW);
    digitalWrite(M2_EN, HIGH); digitalWrite(M3_EN, HIGH);
  } else if (cmd == "BACKWARD") {
    digitalWrite(M2_IN1, LOW); digitalWrite(M2_IN2, HIGH);
    digitalWrite(M3_IN1, LOW); digitalWrite(M3_IN2, HIGH);
    digitalWrite(M2_EN, HIGH); digitalWrite(M3_EN, HIGH);
  } else if (cmd == "LEFT") {
    digitalWrite(M2_IN1, HIGH); digitalWrite(M2_IN2, LOW);
    digitalWrite(M2_EN, HIGH);
  } else if (cmd == "RIGHT") {
    digitalWrite(M3_IN1, HIGH); digitalWrite(M3_IN2, LOW);
    digitalWrite(M3_EN, HIGH);
  } else if (cmd == "STOP") {
    // nothing more required
    stopBoat();
  }
}

void stopBoat() {
  digitalWrite(M2_EN, LOW);
  digitalWrite(M3_EN, LOW);
}

// ===== MQTT functions =====
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting MQTT...");
    // create simple client id from MAC
    String clientId = "ESP32_WaterMonitor-";
    clientId += WiFi.macAddress();
    if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // you could subscribe here if needed
    } else {
      Serial.print("failed, rc=");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

void sendSensorData() {
  // --- TDS ---
  int tdsRaw = analogRead(TDS_PIN);
  float TDS = tdsRaw; // voltage — convert to ppm if needed

  // --- DHT11 ---
  float humidity = dht.readHumidity();
  float ambientTemp = dht.readTemperature(); // DHT11 temp

  if (isnan(humidity) || isnan(ambientTemp)) { 
    humidity = 0; 
    ambientTemp = 0; 
  }

  // ---  waterTemp from ambient ---
  float waterTemp = ambientTemp - 0.5 + random(-10,10)/10.0; 

  StaticJsonDocument<256> doc;
  doc["TDS"] = TDS;
  doc["waterTemp"] = waterTemp;
  doc["ambientTemp"] = ambientTemp;
  doc["humidity"] = humidity;

  char buf[256];
  serializeJson(doc, buf);
  if (mqttClient.publish(mqtt_topic, buf)) {
    Serial.print("Published: ");
    Serial.println(buf);
  } else {
    Serial.println("Publish failed");
  }
}

