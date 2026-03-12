#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiUdp.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// WiFi / receiver
const char* ssid = "*******";
const char* password = "*********";
const char* tcpHost = "**.**.***.**";  // Motor1 TCP
const uint16_t tcpPort = 80;

const char* udpHost = "**.**.***.**";  // Boat UDP
const int udpPort = ****;

WiFiClient tcpClient;
WiFiUDP udp;

// Joysticks
#define JOY1_X 33
#define JOY1_Y 25
#define JOY1_SW 26

#define JOY2_X 35
#define JOY2_Y 34
#define JOY2_SW 27

const int LOW_TH = 1000;
const int HIGH_TH = 3500;

String lastCommandWinch = "STOP";
String lastCommandBoat = "STOP";
unsigned long limitMsgTime = 0;

// --- Winch Depth Estimation ---
float winchDepth = 0.0;         // meters
const float ROPE_SPEED = 0.01;  // meters per second
const float MAX_DEPTH = 1.0;    // meters (full rope length)
unsigned long lastDepthUpdate = 0;

void setup() {
  Serial.begin(115200);
  pinMode(JOY1_SW, INPUT_PULLUP);
  pinMode(JOY2_SW, INPUT_PULLUP);

  Wire.begin(21,22);
  if(!display.begin(SSD1306_SWITCHCAPVCC,0x3C)){for(;;);}
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // Connect Wi-Fi
  WiFi.begin(ssid,password);
  Serial.print("Connecting Wi-Fi");
  while(WiFi.status()!=WL_CONNECTED){delay(500);Serial.print(".");}
  Serial.println("\nWi-Fi Connected");

  display.clearDisplay();
  display.setCursor(0,0);
  display.setTextSize(1);
  display.println("WiFi Connected");
  display.display();
  delay(800);
}

void loop() {
  // --- Winch Joystick ---
  int x1 = analogRead(JOY1_X);
  bool btnWinch = digitalRead(JOY1_SW) == LOW;
  String cmdWinch = "STOP";
  if (x1 < LOW_TH) cmdWinch = "UP";
  else if (x1 > HIGH_TH) cmdWinch = "DOWN";

  if(cmdWinch != lastCommandWinch){
    lastCommandWinch = cmdWinch;
    sendWinch(cmdWinch);
  }

  // --- Boat Joystick ---
  int x2 = analogRead(JOY2_X);
  int y2 = analogRead(JOY2_Y);
  bool btnBoat = digitalRead(JOY2_SW) == LOW;
  String cmdBoat = "STOP";
  if(y2 < 1500) cmdBoat = "FORWARD";
  else if(y2 > 3000) cmdBoat = "BACKWARD";
  else if(x2 < 1500) cmdBoat = "LEFT";
  else if(x2 > 3000) cmdBoat = "RIGHT";

  if(cmdBoat != lastCommandBoat){
    lastCommandBoat = cmdBoat;
    sendBoat(cmdBoat);
  }

  // --- Read Winch feedback (TCP) ---
  if (tcpClient.connected() && tcpClient.available()) {
    String feedback = tcpClient.readStringUntil('\n');
    feedback.trim();
    if (feedback.length() > 0) {
      Serial.println("Feedback: " + feedback);
      if (feedback == "LIMIT") {
        limitMsgTime = millis();  // trigger OLED
        winchDepth = 0.0;         // reset depth at limit
      }
    }
  }

  // --- Depth Update every 100 ms ---
  if (millis() - lastDepthUpdate >= 100) {
    lastDepthUpdate = millis();

    if (lastCommandWinch == "UP") {
      winchDepth -= ROPE_SPEED * 0.1; // 0.1s step
      if (winchDepth < 0) winchDepth = 0;
    }
    else if (lastCommandWinch == "DOWN") {
      winchDepth += ROPE_SPEED * 0.1; // 0.1s step
      if (winchDepth > MAX_DEPTH) winchDepth = MAX_DEPTH;
    }
  }

  showOLED(cmdWinch, btnWinch, cmdBoat, btnBoat);
  delay(20);
}

// --- Send commands ---
void sendWinch(String cmd){
  if(tcpClient.connect(tcpHost,tcpPort)){
    tcpClient.print(cmd + "\n");
  } else Serial.println("Winch send failed");
}

void sendBoat(String cmd){
  udp.beginPacket(udpHost,udpPort);
  udp.print(cmd);
  udp.endPacket();
}

// --- OLED Display ---
void showOLED(String winchCmd, bool btnWinch, String boatCmd, bool btnBoat){
  display.clearDisplay();
  display.setTextSize(1); // smaller letters

  // --- Winch ---
  display.setCursor(0,0);
  display.print("Winch: ");
  if(millis() - limitMsgTime < 2000) display.println("LIMIT!");
  else display.println(winchCmd);

  display.setCursor(0,12);
  display.print("Btn: "); 
  display.println(btnWinch ? "Pressed" : "Released");

  // --- Boat ---
  display.setCursor(0,28);
  display.print("Boat: ");
  display.println(boatCmd);

  display.setCursor(0,40);
  display.print("Btn: "); 
  display.println(btnBoat ? "Pressed" : "Released");

  // --- Depth ---
  display.setCursor(0,54);
  display.print("Depth: ");
  display.print(winchDepth,3); // 3 decimals = mm precision
  display.println(" m");

  display.display();
}
