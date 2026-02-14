#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <time.h>
#include "Secrets.h"
#include "IDisplay.h"
#include "display/Hub75Display.h"

// Animation includes
#include "animations/EggChickAnimation.h"
#include "animations/ValentineHeartbeat.h"
#include "animations/ValentineTwoHearts.h"
#include "animations/ValentineLoveLetter.h"
#include "animations/ValentineTextOnly.h"
#include "animations/ValentineTextChick.h"
#include "animations/Chick1Animation.h"
#include "animations/Mosq1Animation.h"

// Active animation tracking
enum class ActiveAnimation {
    NONE,
    EGG_CHICK,
    VALENTINE_HEARTBEAT,
    VALENTINE_TWO_HEARTS,
    VALENTINE_LOVE_LETTER,
    VALENTINE_TEXT_ONLY,
    VALENTINE_TEXT_CHICK,
    CHICK1,
    MOSQ1
};
ActiveAnimation currentAnimation = ActiveAnimation::NONE;

#define PANEL_WIDTH  64
#define PANEL_HEIGHT 64

#define R1_PIN  4
#define G1_PIN  5
#define B1_PIN  6
#define R2_PIN  7
#define G2_PIN  15
#define B2_PIN  16

#define A_PIN   18
#define B_PIN   8
#define C_PIN   3
#define D_PIN   10
#define E_PIN   17

#define LAT_PIN 12
#define OE_PIN  13
#define CLK_PIN 14

IDisplay *display = nullptr;
MatrixPanel_I2S_DMA *matrixDisplay = nullptr;
Hub75Display *hub75Display = nullptr;

#ifndef FW_VERSION
#define FW_VERSION "dev"
#endif
const uint32_t OTA_CHECK_INTERVAL_MS = 6UL * 60UL * 60UL * 1000UL;

WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

String currentText = "Hello World";
String lastMessageTime = "";
uint32_t lastOtaCheckMs = 0;

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

bool otaConfigIsValid() {
  return strlen(GITHUB_OWNER) > 0 && strlen(GITHUB_REPO) > 0;
}

String buildGithubReleaseAssetUrl(const char *assetName) {
  String url = "https://github.com/";
  url += GITHUB_OWNER;
  url += "/";
  url += GITHUB_REPO;
  url += "/releases/latest/download/";
  url += assetName;
  return url;
}

int readVersionPart(const String &version, int index) {
  int partStart = 0;
  int currentPart = 0;

  for (int i = 0; i <= version.length(); i++) {
    if (i == version.length() || version[i] == '.') {
      if (currentPart == index) {
        String part = version.substring(partStart, i);
        part.trim();
        return part.toInt();
      }
      currentPart++;
      partStart = i + 1;
    }
  }

  return 0;
}

bool isRemoteVersionNewer(const String &remoteVersion, const String &localVersion) {
  for (int i = 0; i < 3; i++) {
    int remotePart = readVersionPart(remoteVersion, i);
    int localPart = readVersionPart(localVersion, i);
    if (remotePart > localPart) {
      return true;
    }
    if (remotePart < localPart) {
      return false;
    }
  }

  return false;
}

String fetchRemoteVersion() {
  WiFiClientSecure httpsClient;
  httpsClient.setInsecure();

  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setConnectTimeout(8000);

  String versionUrl = buildGithubReleaseAssetUrl("version.txt");
  if (!http.begin(httpsClient, versionUrl)) {
    return "";
  }

  http.setUserAgent("esp32-rgb-ota");
  int statusCode = http.GET();
  if (statusCode != HTTP_CODE_OK) {
    http.end();
    return "";
  }

  String remoteVersion = http.getString();
  http.end();
  remoteVersion.trim();
  return remoteVersion;
}

void checkAndApplyOtaUpdate(bool forceCheck = false) {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (!otaConfigIsValid()) {
    if (forceCheck) {
      Serial.println("OTA not configured: set GITHUB_OWNER and GITHUB_REPO constants");
    }
    return;
  }

  uint32_t now = millis();
  if (!forceCheck && (now - lastOtaCheckMs) < OTA_CHECK_INTERVAL_MS) {
    return;
  }
  lastOtaCheckMs = now;

  String localVersion = FW_VERSION;
  localVersion.trim();
  String remoteVersion = fetchRemoteVersion();

  if (remoteVersion.length() == 0) {
    if (forceCheck) {
      Serial.println("OTA check failed: could not fetch version.txt from GitHub");
    }
    return;
  }

  Serial.printf("OTA check local=%s remote=%s\n", localVersion.c_str(), remoteVersion.c_str());

  if (!forceCheck && !isRemoteVersionNewer(remoteVersion, localVersion)) {
    return;
  }

  if (forceCheck && !isRemoteVersionNewer(remoteVersion, localVersion)) {
    Serial.println("OTA: already at latest firmware");
    return;
  }

  WiFiClientSecure updateClient;
  updateClient.setInsecure();

  String firmwareUrl = buildGithubReleaseAssetUrl("firmware.bin");
  Serial.printf("OTA: downloading %s\n", firmwareUrl.c_str());

  t_httpUpdate_return result = httpUpdate.update(updateClient, firmwareUrl);
  switch (result) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("OTA failed (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("OTA: no update available");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("OTA: update successful, rebooting...");
      break;
  }
}

// Tiny 4x5 pixel font for digits and colon
const uint8_t tinyDigits[11][5] = {
  {0b1111, 0b1001, 0b1001, 0b1001, 0b1111}, // 0
  {0b0010, 0b0110, 0b0010, 0b0010, 0b0111}, // 1
  {0b1111, 0b0001, 0b1111, 0b1000, 0b1111}, // 2
  {0b1111, 0b0001, 0b1111, 0b0001, 0b1111}, // 3
  {0b1001, 0b1001, 0b1111, 0b0001, 0b0001}, // 4
  {0b1111, 0b1000, 0b1111, 0b0001, 0b1111}, // 5
  {0b1111, 0b1000, 0b1111, 0b1001, 0b1111}, // 6
  {0b1111, 0b0001, 0b0001, 0b0001, 0b0001}, // 7
  {0b1111, 0b1001, 0b1111, 0b1001, 0b1111}, // 8
  {0b1111, 0b1001, 0b1111, 0b0001, 0b1111}, // 9
  {0b0000, 0b0100, 0b0000, 0b0100, 0b0000}  // : (colon)
};

void drawTinyDigit(char c, int16_t x, int16_t y, uint16_t color) {
  int digit = -1;
  if (c >= '0' && c <= '9') {
    digit = c - '0';
  } else if (c == ':') {
    digit = 10;
  } else {
    return;
  }
  
  for (int row = 0; row < 5; row++) {
    uint8_t rowData = tinyDigits[digit][row];
    for (int col = 0; col < 4; col++) {
      if (rowData & (1 << (3 - col))) {
        display->drawPixel(x + col, y + row, color);
      }
    }
  }
}

/**
 * Draw timestamp in bottom right corner with black background for visibility
 */
void drawTimestamp() {
  if (lastMessageTime.length() == 0) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char timeStr[6];
      strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
      lastMessageTime = String(timeStr);
    }
  }
  
  if (lastMessageTime.length() > 0) {
    int charWidth = 4;
    int spacing = 1;
    int totalWidth = lastMessageTime.length() * (charWidth + spacing) - spacing;
    int timeX = PANEL_WIDTH - totalWidth - 3;
    int timeY = PANEL_HEIGHT - 7;
    
    // Draw black background box with 1px padding
    display->fillRect(timeX - 2, timeY - 1, totalWidth + 4, 7, display->color565(0, 0, 0));
    
    // Draw time in bright white
    uint16_t timeColor = display->color565(255, 255, 255);
    for (size_t i = 0; i < lastMessageTime.length(); i++) {
      drawTinyDigit(lastMessageTime[i], timeX + i * (charWidth + spacing), timeY, timeColor);
    }
  }
}

void drawText(const String &text, uint8_t r, uint8_t g, uint8_t b) {
  display->fillScreen(display->color565(0, 0, 0));
  display->setTextColor(display->color565(r, g, b));
  
  const int charWidth = 6;
  const int charHeight = 8;
  const int maxCharsPerLine = PANEL_WIDTH / charWidth;
  
  String lines[10];
  int lineCount = 0;
  String currentLine = "";
  
  for (unsigned int i = 0; i < text.length() && lineCount < 10; i++) {
    if (currentLine.length() >= (unsigned int)maxCharsPerLine) {
      lines[lineCount++] = currentLine;
      currentLine = "";
    }
    currentLine += text[i];
  }
  if (currentLine.length() > 0 && lineCount < 10) {
    lines[lineCount++] = currentLine;
  }
  
  int totalHeight = lineCount * charHeight;
  int startY = (PANEL_HEIGHT - totalHeight) / 2;
  
  display->setTextSize(1);
  for (int i = 0; i < lineCount; i++) {
    int lineWidth = lines[i].length() * charWidth;
    int x = (PANEL_WIDTH - lineWidth) / 2;
    int y = startY + (i * charHeight);
    display->setCursor(x, y);
    display->print(lines[i].c_str());
  }
  
  drawTimestamp();
}

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  display->fillScreen(0);
  display->setTextColor(display->color565(100, 100, 255));
  display->setCursor(8, 28);
  display->print("WiFi...");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(250);
    attempts++;
  }
}

/**
 * Stop all animations
 */
void stopAllAnimations() {
    eggChickAnimation.stop();
    valentineHeartbeat.stop();
    valentineTwoHearts.stop();
    valentineLoveLetter.stop();
    valentineTextOnly.stop();
    valentineTextChick.stop();
    chick1Animation.stop();
    mosq1Animation.stop();
    currentAnimation = ActiveAnimation::NONE;
}

/**
 * Check if any animation is currently running
 */
bool isAnyAnimationRunning() {
    return eggChickAnimation.isRunning() ||
           valentineHeartbeat.isRunning() ||
           valentineTwoHearts.isRunning() ||
           valentineLoveLetter.isRunning() ||
           valentineTextOnly.isRunning() ||
           valentineTextChick.isRunning() ||
           chick1Animation.isRunning() ||
           mosq1Animation.isRunning();
}

/**
 * Parse incoming MQTT/Serial command
 */
void parseCommand(const String &command) {
  // === Valentine Animation Commands ===
  // ANIM:VAL1 - Heartbeat pulse animation
  if (command == "ANIM:VAL1" || command == "val1" || command == "heartbeat" || command == "pulse") {
    stopAllAnimations();
    valentineHeartbeat.start();
    currentAnimation = ActiveAnimation::VALENTINE_HEARTBEAT;
    Serial.println("Starting Valentine Heartbeat animation!");
    return;
  }
  
  // ANIM:VAL2 - Two hearts become one
  if (command == "ANIM:VAL2" || command == "val2" || command == "twohearts" || command == "love") {
    stopAllAnimations();
    valentineTwoHearts.start();
    currentAnimation = ActiveAnimation::VALENTINE_TWO_HEARTS;
    Serial.println("Starting Valentine Two Hearts animation!");
    return;
  }
  
  // ANIM:VAL3 - Love letter animation
  if (command == "ANIM:VAL3" || command == "val3" || command == "letter" || command == "envelope") {
    stopAllAnimations();
    valentineLoveLetter.start();
    currentAnimation = ActiveAnimation::VALENTINE_LOVE_LETTER;
    Serial.println("Starting Valentine Love Letter animation!");
    return;
  }

  // ANIM:VAL4 - Text-only I <3 YOU on black
  if (command == "ANIM:VAL4" || command == "val4" || command == "textlove" || command == "ily") {
    stopAllAnimations();
    valentineTextOnly.start();
    currentAnimation = ActiveAnimation::VALENTINE_TEXT_ONLY;
    Serial.println("Starting Valentine Text-Only animation!");
    return;
  }

  // ANIM:VAL5 - Text-only base with walking chick
  if (command == "ANIM:VAL5" || command == "val5" || command == "chicklove" || command == "ilychick") {
    stopAllAnimations();
    valentineTextChick.start();
    currentAnimation = ActiveAnimation::VALENTINE_TEXT_CHICK;
    Serial.println("Starting Valentine Text + Chick animation!");
    return;
  }

  // CHICK1 - Front-facing chick with broken shell top/bottom
  if (command == "ANIM:CHICK1" || command == "chick1" || command == "cutechick") {
    stopAllAnimations();
    chick1Animation.start();
    currentAnimation = ActiveAnimation::CHICK1;
    Serial.println("Starting CHICK1 animation!");
    return;
  }

  // MOSQ1 - White/blue mosque with crescent and blinking stars
  if (command == "ANIM:MOSQ1" || command == "mosq1" || command == "mosque") {
    stopAllAnimations();
    mosq1Animation.start();
    currentAnimation = ActiveAnimation::MOSQ1;
    Serial.println("Starting MOSQ1 animation!");
    return;
  }
  
  // Egg/Chick animation commands
  if (command == "egg" || command == "hatch" || command == "animation/egg_chick" || command == "chick") {
    stopAllAnimations();
    eggChickAnimation.start();
    currentAnimation = ActiveAnimation::EGG_CHICK;
    Serial.println("Starting egg hatching animation!");
    return;
  }
  
  // Stop command
  if (command == "stop" || command == "animation/stop" || command == "ANIM:STOP") {
    stopAllAnimations();
    drawText(currentText, 255, 255, 255);  // Restore text display
    Serial.println("Animation stopped");
    return;
  }

  if (command == "OTA:CHECK" || command == "ota" || command == "update") {
    checkAndApplyOtaUpdate(true);
    return;
  }
  
  // Stop any running animation when new text arrives
  if (isAnyAnimationRunning()) {
    stopAllAnimations();
  }
  
  if (command.startsWith("TEXT:")) {
    String text = command.substring(5);
    text.trim();
    if (text.length() > 0) {
      currentText = text;
      
      // Update timestamp
      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        char timeStr[6];
        strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
        lastMessageTime = String(timeStr);
      }
      
      drawText(currentText, 255, 255, 255);
    }
  } else {
    // Treat any other command as direct text
    currentText = command;
    
    // Update timestamp
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char timeStr[6];
      strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
      lastMessageTime = String(timeStr);
    }
    
    drawText(currentText, 255, 255, 255);
  }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  if (strcmp(topic, MQTT_TOPIC_TEXT) != 0) {
    return;
  }

  String incoming;
  incoming.reserve(length);
  for (unsigned int i = 0; i < length; i++) {
    incoming += static_cast<char>(payload[i]);
  }
  incoming.trim();

  if (incoming.length() > 0) {
    parseCommand(incoming);
  }
}

void connectMQTT() {
  int attempts = 0;
  while (!mqttClient.connected() && attempts < 5) {
    String clientId = "esp32-rgb-" + String(static_cast<uint32_t>(ESP.getEfuseMac()), HEX);
    if (mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
      mqttClient.subscribe(MQTT_TOPIC_TEXT);
    } else {
      delay(500);
      attempts++;
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  HUB75_I2S_CFG::i2s_pins pins = {
    R1_PIN, G1_PIN, B1_PIN,
    R2_PIN, G2_PIN, B2_PIN,
    A_PIN, B_PIN, C_PIN, D_PIN, E_PIN,
    LAT_PIN, OE_PIN, CLK_PIN
  };

  HUB75_I2S_CFG cfg(PANEL_WIDTH, PANEL_HEIGHT, 1, pins);

  cfg.driver = HUB75_I2S_CFG::SHIFTREG;
  cfg.clkphase = false;  // Try false if ghosting persists
  
  // For 1/32 scan P3 panels - lower speeds reduce ghosting
  // cfg.i2sspeed = HUB75_I2S_CFG::HZ_10M;  // Start with 10MHz
  // Try these if ghosting persists:
  // cfg.i2sspeed = HUB75_I2S_CFG::HZ_8M;   // 8MHz - slower, less ghosting
  cfg.i2sspeed = HUB75_I2S_CFG::HZ_15M;  // higher refresh to reduce visible scan waves
  // cfg.i2sspeed = HUB75_I2S_CFG::HZ_20M;  // 20MHz - original (too fast)
  
  cfg.double_buff = false;
  cfg.setPixelColorDepthBits(3); // fewer bitplanes reduces PWM wave/banding artifacts
  cfg.min_refresh_rate = 120;
  cfg.latch_blanking = 3;  // balance ghosting vs refresh flicker

  matrixDisplay = new MatrixPanel_I2S_DMA(cfg);

  if (!matrixDisplay->begin()) {
    Serial.println("Matrix init failed!");
    while (true);
  }

  matrixDisplay->setBrightness8(110);
  matrixDisplay->clearScreen();

  hub75Display = new Hub75Display(matrixDisplay);
  display = hub75Display;

  display->setTextWrap(false);
  display->setTextSize(1);

  // Show welcome message
  display->fillScreen(0);
  display->setTextColor(display->color565(255, 100, 150));
  display->setCursor(10, 28);
  display->print("RGB Matrix");
  display->setCursor(16, 38);
  display->print("Display");
  delay(2000);

  connectWiFi();
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  struct tm timeinfo;
  for (int i = 0; i < 10; i++) {
    if (getLocalTime(&timeinfo)) {
      char timeStr[6];
      strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
      lastMessageTime = String(timeStr);
      break;
    }
    delay(500);
  }
  
  wifiClient.setInsecure();
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  connectMQTT();
  
  // Initialize animation system
  eggChickAnimation.init(display);
  valentineHeartbeat.init(display);
  valentineTwoHearts.init(display);
  valentineLoveLetter.init(display);
  valentineTextOnly.init(display);
  valentineTextChick.init(display);
  chick1Animation.init(display);
  mosq1Animation.init(display);
  
  // Display initial text
  drawText(currentText, 255, 255, 255);

  Serial.printf("Firmware version: %s\n", FW_VERSION);
  checkAndApplyOtaUpdate(true);
  
  Serial.println("RGB Matrix Display Ready!");
  Serial.println("Commands: TEXT:message or direct text");
  Serial.println("Animations:");
  Serial.println("  egg, hatch, chick - Egg hatching animation");
  Serial.println("  ANIM:VAL1, val1   - Valentine Heartbeat");
  Serial.println("  ANIM:VAL2, val2   - Two Hearts Become One");
  Serial.println("  ANIM:VAL3, val3   - Love Letter");
  Serial.println("  ANIM:VAL4, val4   - I <3 YOU Text Only");
  Serial.println("  ANIM:VAL5, val5   - I <3 YOU + Walking Chick");
  Serial.println("  ANIM:CHICK1       - Cute Chick in Broken Shell");
  Serial.println("  ANIM:MOSQ1        - White/Blue Mosque + Stars");
  Serial.println("  stop              - Stop any animation");
  Serial.println("  OTA:CHECK         - Force OTA check now");
  Serial.println("MQTT Topic: esp32/rgb/text");
}

void loop() {
  static unsigned long lastWifiCheck = 0;
  static unsigned long lastTimeUpdate = 0;
  static bool animationWasRunning = false;
  
  unsigned long now = millis();
  
  // Check WiFi every 30 seconds
  if (now - lastWifiCheck > 30000) {
    lastWifiCheck = now;
    if (WiFi.status() != WL_CONNECTED) {
      connectWiFi();
    }
  }

  checkAndApplyOtaUpdate(false);

  // Keep MQTT connected
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();

  // Handle animation if running (takes priority over text display)
  if (isAnyAnimationRunning()) {
    animationWasRunning = true;
    
    // Update and render the active animation
    switch (currentAnimation) {
      case ActiveAnimation::EGG_CHICK:
        eggChickAnimation.update();
        eggChickAnimation.render();
        break;
      case ActiveAnimation::VALENTINE_HEARTBEAT:
        valentineHeartbeat.update();
        valentineHeartbeat.render();
        break;
      case ActiveAnimation::VALENTINE_TWO_HEARTS:
        valentineTwoHearts.update();
        valentineTwoHearts.render();
        break;
      case ActiveAnimation::VALENTINE_LOVE_LETTER:
        valentineLoveLetter.update();
        valentineLoveLetter.render();
        break;
      case ActiveAnimation::VALENTINE_TEXT_ONLY:
        valentineTextOnly.update();
        valentineTextOnly.render();
        break;
      case ActiveAnimation::VALENTINE_TEXT_CHICK:
        valentineTextChick.update();
        valentineTextChick.render();
        break;
      case ActiveAnimation::CHICK1:
        chick1Animation.update();
        chick1Animation.render();
        break;
      case ActiveAnimation::MOSQ1:
        mosq1Animation.update();
        mosq1Animation.render();
        break;
      default:
        break;
    }
    
    // Still handle Serial input during animation
    if (Serial.available()) {
      String incoming = Serial.readStringUntil('\n');
      incoming.trim();
      if (incoming.length() > 0) {
        parseCommand(incoming);
      }
    }
    
    delay(1);
    return;  // Skip normal display updates while animating
  }
  
  // Restore text display when animation completes
  if (animationWasRunning && !isAnyAnimationRunning()) {
    animationWasRunning = false;
    currentAnimation = ActiveAnimation::NONE;
    drawText(currentText, 255, 255, 255);
    Serial.println("Animation complete, restored text display");
  }

  // Update time every minute (only when not animating)
  if (now - lastTimeUpdate > 60000) {
    lastTimeUpdate = now;
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      char timeStr[6];
      strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
      lastMessageTime = String(timeStr);
      // Redraw to update timestamp
      drawText(currentText, 255, 255, 255);
    }
  }

  // Handle Serial input
  if (Serial.available()) {
    String incoming = Serial.readStringUntil('\n');
    incoming.trim();
    if (incoming.length() > 0) {
      parseCommand(incoming);
    }
  }

  delay(1);
}
