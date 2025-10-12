/*
 * GATEWAY LORA - SISTEM MONITORING KEHADIRAN
 * Hardware: ESP32-C3 Super Mini + LoRa Ra-02
 *
 * Fungsi:
 * 1. Sync waktu dari NTP Server
 * 2. Broadcast beacon ke semua node setiap 30 detik
 * 3. Terima data dari 30 node secara bergiliran
 * 4. Kirim ACK dengan timestamp update
 * 5. Forward data ke API server via HTTP
 * 6. Monitoring & logging
 */

#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

// ============================================
// PIN DEFINITIONS
// ============================================
#define SCK 6
#define MISO 2
#define MOSI 7
#define SS 10
#define RST 3
#define DIO0 4
#define LED_BLUE 8
#define BUZZER 5

// ============================================
// LORA CONFIGURATION
// ============================================
#define LORA_FREQUENCY 433E6      // 433 MHz
#define LORA_BANDWIDTH 500E3      // 500 kHz
#define LORA_SPREADING_FACTOR 7   // SF7 untuk speed
#define LORA_CODING_RATE 5        // 4/5
#define LORA_TX_POWER 20          // 20 dBm
#define LORA_SYNC_WORD 0x12       // Private network

// ============================================
// WIFI & NTP CONFIGURATION
// ============================================
const char* WIFI_SSID = "ZEDLABS";
const char* WIFI_PASSWORD = "18012000";
const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET = 7 * 3600;  // WIB = GMT+7
const int DAYLIGHT_OFFSET = 0;

// ============================================
// API SERVER CONFIGURATION
// ============================================
const char* API_ENDPOINT = "http://your-server.com/api/attendance";
const char* API_KEY = "your_api_key_here";

// ============================================
// SYSTEM CONSTANTS
// ============================================
#define TOTAL_NODES 30
#define CYCLE_DURATION 450        // 7.5 menit = 450 detik
#define BEACON_INTERVAL 30000     // 30 detik
#define FULL_SYNC_INTERVAL 3600000 // 1 jam
#define ACK_TIMEOUT 2000          // 2 detik
#define MAX_RETRY 3

// ============================================
// PACKET TYPES
// ============================================
#define PKT_TYPE_BEACON 0x01
#define PKT_TYPE_DATA 0x02
#define PKT_TYPE_ACK 0x03
#define PKT_TYPE_SYNC 0x04

// ============================================
// STRUCTURES
// ============================================
struct NodeData {
  uint8_t nodeId;
  uint32_t timestamp;
  uint8_t packetSeq;
  uint8_t studentCount;
  uint8_t macAddresses[30][6];  // Max 30 MAC addresses
  int rssi;
  float snr;
  bool received;
  unsigned long lastSeen;
};

struct BeaconPacket {
  uint8_t type;
  uint32_t timestamp;
  uint16_t cycleNumber;
  uint16_t cycleSecond;
  uint32_t nextCycleStart;
} __attribute__((packed));

struct DataPacket {
  uint8_t type;
  uint8_t nodeId;
  uint8_t packetSeq;
  uint32_t timestamp;
  uint16_t payloadLength;
  uint8_t studentCount;
  uint8_t payload[180];  // MAC addresses
  uint16_t crc;
} __attribute__((packed));

struct AckPacket {
  uint8_t type;
  uint8_t nodeId;
  uint32_t timestamp;
  uint16_t slotTime;
  int16_t driftMs;
  uint8_t status;
} __attribute__((packed));

// ============================================
// GLOBAL VARIABLES
// ============================================
NodeData nodes[TOTAL_NODES];
unsigned long lastBeaconTime = 0;
unsigned long lastSyncTime = 0;
unsigned long cycleStartTime = 0;
uint16_t currentCycle = 0;
bool wifiConnected = false;
bool ntpSynced = false;

// Statistics
uint32_t totalPacketsReceived = 0;
uint32_t totalPacketsSent = 0;
uint32_t totalErrors = 0;
uint32_t apiSuccessCount = 0;
uint32_t apiFailCount = 0;

// ============================================
// SETUP
// ============================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n========================================");
  Serial.println("GATEWAY LORA - MONITORING KEHADIRAN");
  Serial.println("========================================\n");

  // Setup GPIO
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(BUZZER, LOW);

  // Beep startup
  beep(2, 100);

  // Initialize WiFi
  initWiFi();

  // Initialize NTP
  initNTP();

  // Initialize LoRa
  initLoRa();

  // Initialize node tracking
  initNodes();

  // Set cycle start time
  cycleStartTime = millis();
  currentCycle = 0;

  Serial.println("\n✅ GATEWAY READY!\n");
  beep(3, 50);
}

// ============================================
// MAIN LOOP
// ============================================
void loop() {
  unsigned long currentMillis = millis();

  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️  WiFi disconnected! Reconnecting...");
    initWiFi();
  }

  // Broadcast beacon setiap 30 detik
  if (currentMillis - lastBeaconTime >= BEACON_INTERVAL) {
    broadcastBeacon();
    lastBeaconTime = currentMillis;
  }

  // Full NTP sync setiap 1 jam
  if (currentMillis - lastSyncTime >= FULL_SYNC_INTERVAL) {
    syncNTP();
    lastSyncTime = currentMillis;
  }

  // Check untuk cycle baru
  if (currentMillis - cycleStartTime >= CYCLE_DURATION * 1000) {
    onCycleComplete();
    cycleStartTime = currentMillis;
    currentCycle++;
  }

  // Receive & process LoRa packets
  receiveLoRaPacket();

  // Monitor node health
  monitorNodes();

  // Print status setiap 10 detik
  static unsigned long lastStatusPrint = 0;
  if (currentMillis - lastStatusPrint >= 10000) {
    printStatus();
    lastStatusPrint = currentMillis;
  }
}

// ============================================
// WIFI FUNCTIONS
// ============================================
void initWiFi() {
  Serial.print("🔌 Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected!");
    Serial.print("   IP Address: ");
    Serial.println(WiFi.localIP());
    wifiConnected = true;
    blinkLED(3, 100);
  } else {
    Serial.println("\n❌ WiFi Failed!");
    wifiConnected = false;
    beep(5, 50);
  }
}

// ============================================
// NTP FUNCTIONS
// ============================================
void initNTP() {
  Serial.println("🕐 Syncing with NTP Server...");

  configTime(GMT_OFFSET, DAYLIGHT_OFFSET, NTP_SERVER);

  struct tm timeinfo;
  int attempts = 0;
  while (!getLocalTime(&timeinfo) && attempts < 10) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (getLocalTime(&timeinfo)) {
    Serial.println("\n✅ NTP Synchronized!");
    Serial.print("   Current Time: ");
    Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
    ntpSynced = true;
    blinkLED(3, 100);
  } else {
    Serial.println("\n❌ NTP Sync Failed!");
    ntpSynced = false;
    beep(5, 50);
  }
}

void syncNTP() {
  Serial.println("🔄 Re-syncing NTP...");
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.println("✅ NTP Re-sync Success");
    ntpSynced = true;
  } else {
    Serial.println("❌ NTP Re-sync Failed");
    ntpSynced = false;
  }
}

// ============================================
// LORA FUNCTIONS
// ============================================
void initLoRa() {
  Serial.println("📡 Initializing LoRa...");

  // Setup SPI pins
  SPI.begin(SCK, MISO, MOSI, SS);

  // Setup LoRa pins
  LoRa.setPins(SS, RST, DIO0);

  // Initialize LoRa
  if (!LoRa.begin(LORA_FREQUENCY)) {
    Serial.println("❌ LoRa initialization failed!");
    while (1) {
      beep(1, 1000);
      delay(1000);
    }
  }

  // Configure LoRa
  LoRa.setSpreadingFactor(LORA_SPREADING_FACTOR);
  LoRa.setSignalBandwidth(LORA_BANDWIDTH);
  LoRa.setCodingRate4(LORA_CODING_RATE);
  LoRa.setTxPower(LORA_TX_POWER);
  LoRa.setSyncWord(LORA_SYNC_WORD);
  LoRa.enableCrc();

  Serial.println("✅ LoRa Initialized!");
  Serial.printf("   Frequency: %.2f MHz\n", LORA_FREQUENCY / 1E6);
  Serial.printf("   Bandwidth: %.0f kHz\n", LORA_BANDWIDTH / 1E3);
  Serial.printf("   SF: %d\n", LORA_SPREADING_FACTOR);
  Serial.printf("   TX Power: %d dBm\n", LORA_TX_POWER);

  blinkLED(3, 100);
}

void broadcastBeacon() {
  if (!ntpSynced) {
    Serial.println("⚠️  Skip beacon - NTP not synced");
    return;
  }

  BeaconPacket beacon;
  beacon.type = PKT_TYPE_BEACON;
  beacon.timestamp = getUnixTime();
  beacon.cycleNumber = currentCycle;
  beacon.cycleSecond = (millis() - cycleStartTime) / 1000;
  beacon.nextCycleStart = beacon.timestamp + (CYCLE_DURATION - beacon.cycleSecond);

  LoRa.beginPacket();
  LoRa.write((uint8_t*)&beacon, sizeof(beacon));
  LoRa.endPacket();

  Serial.printf("📡 BEACON sent | Cycle: %d | Second: %d | Time: %s",
                beacon.cycleNumber, beacon.cycleSecond, getTimeString().c_str());

  blinkLED(1, 50);
  totalPacketsSent++;
}

void receiveLoRaPacket() {
  int packetSize = LoRa.parsePacket();
  if (packetSize == 0) return;

  digitalWrite(LED_BLUE, HIGH);

  // Read packet
  uint8_t buffer[256];
  int idx = 0;
  while (LoRa.available() && idx < 256) {
    buffer[idx++] = LoRa.read();
  }

  int rssi = LoRa.packetRssi();
  float snr = LoRa.packetSnr();

  // Process packet berdasarkan type
  uint8_t packetType = buffer[0];

  if (packetType == PKT_TYPE_DATA) {
    processDataPacket(buffer, idx, rssi, snr);
  } else {
    Serial.printf("⚠️  Unknown packet type: 0x%02X\n", packetType);
  }

  digitalWrite(LED_BLUE, LOW);
}

void processDataPacket(uint8_t* buffer, int length, int rssi, float snr) {
  if (length < sizeof(DataPacket)) {
    Serial.println("❌ Invalid packet size");
    totalErrors++;
    return;
  }

  DataPacket* data = (DataPacket*)buffer;

  // Validasi node ID
  if (data->nodeId < 1 || data->nodeId > TOTAL_NODES) {
    Serial.printf("❌ Invalid node ID: %d\n", data->nodeId);
    totalErrors++;
    return;
  }

  uint8_t nodeIdx = data->nodeId - 1;

  Serial.printf("\n📥 DATA from Node %d | Seq: %d | Students: %d | RSSI: %d | SNR: %.2f\n",
                data->nodeId, data->packetSeq, data->studentCount, rssi, snr);

  // Update node data
  nodes[nodeIdx].nodeId = data->nodeId;
  nodes[nodeIdx].timestamp = data->timestamp;
  nodes[nodeIdx].packetSeq = data->packetSeq;
  nodes[nodeIdx].studentCount = data->studentCount;
  nodes[nodeIdx].rssi = rssi;
  nodes[nodeIdx].snr = snr;
  nodes[nodeIdx].received = true;
  nodes[nodeIdx].lastSeen = millis();

  // Extract MAC addresses
  int macCount = min((int)data->studentCount, 30);
  memcpy(nodes[nodeIdx].macAddresses, data->payload, macCount * 6);

  totalPacketsReceived++;

  // Send ACK
  sendAck(data->nodeId, rssi, snr);

  // Forward to API server
  forwardToAPI(nodeIdx);

  beep(1, 30);
}

void sendAck(uint8_t nodeId, int rssi, float snr) {
  AckPacket ack;
  ack.type = PKT_TYPE_ACK;
  ack.nodeId = nodeId;
  ack.timestamp = getUnixTime();
  ack.slotTime = (nodeId - 1) * 15;  // Slot time dalam detik
  ack.driftMs = 0;  // TODO: Calculate drift
  ack.status = 0x00;  // Success

  LoRa.beginPacket();
  LoRa.write((uint8_t*)&ack, sizeof(ack));
  LoRa.endPacket();

  Serial.printf("📤 ACK sent to Node %d | Timestamp: %u\n", nodeId, ack.timestamp);

  totalPacketsSent++;
}

// ============================================
// API FUNCTIONS
// ============================================
void forwardToAPI(uint8_t nodeIdx) {
  if (!wifiConnected) {
    Serial.println("⚠️  Skip API - WiFi not connected");
    apiFailCount++;
    return;
  }

  HTTPClient http;
  http.begin(API_ENDPOINT);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + API_KEY);
  http.setTimeout(5000);

  // Build JSON
  DynamicJsonDocument doc(4096);
  doc["gateway_id"] = "GW001";
  doc["node_id"] = nodes[nodeIdx].nodeId;
  doc["timestamp"] = nodes[nodeIdx].timestamp;
  doc["student_count"] = nodes[nodeIdx].studentCount;
  doc["rssi"] = nodes[nodeIdx].rssi;
  doc["snr"] = nodes[nodeIdx].snr;
  doc["cycle"] = currentCycle;

  JsonArray macArray = doc.createNestedArray("mac_addresses");
  for (int i = 0; i < nodes[nodeIdx].studentCount; i++) {
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            nodes[nodeIdx].macAddresses[i][0],
            nodes[nodeIdx].macAddresses[i][1],
            nodes[nodeIdx].macAddresses[i][2],
            nodes[nodeIdx].macAddresses[i][3],
            nodes[nodeIdx].macAddresses[i][4],
            nodes[nodeIdx].macAddresses[i][5]);
    macArray.add(macStr);
  }

  String jsonString;
  serializeJson(doc, jsonString);

  // Send POST request
  int httpCode = http.POST(jsonString);

  if (httpCode == 200 || httpCode == 201) {
    Serial.printf("✅ API Success | Node %d | Code: %d\n", nodes[nodeIdx].nodeId, httpCode);
    apiSuccessCount++;
  } else {
    Serial.printf("❌ API Failed | Node %d | Code: %d | Error: %s\n",
                  nodes[nodeIdx].nodeId, httpCode, http.errorToString(httpCode).c_str());
    apiFailCount++;
  }

  http.end();
}

// ============================================
// MONITORING FUNCTIONS
// ============================================
void initNodes() {
  for (int i = 0; i < TOTAL_NODES; i++) {
    nodes[i].nodeId = i + 1;
    nodes[i].received = false;
    nodes[i].lastSeen = 0;
  }
}

void monitorNodes() {
  unsigned long currentMillis = millis();

  for (int i = 0; i < TOTAL_NODES; i++) {
    // Check jika node tidak lapor dalam 3 cycle (22.5 menit)
    if (nodes[i].lastSeen > 0 &&
        currentMillis - nodes[i].lastSeen > (CYCLE_DURATION * 3 * 1000)) {
      Serial.printf("⚠️  WARNING: Node %d offline! Last seen: %lu seconds ago\n",
                    nodes[i].nodeId, (currentMillis - nodes[i].lastSeen) / 1000);
    }
  }
}

void onCycleComplete() {
  Serial.println("\n========================================");
  Serial.printf("🔄 CYCLE %d COMPLETE\n", currentCycle);
  Serial.println("========================================");

  int receivedCount = 0;
  int missingCount = 0;

  for (int i = 0; i < TOTAL_NODES; i++) {
    if (nodes[i].received) {
      receivedCount++;
      nodes[i].received = false;  // Reset untuk cycle berikutnya
    } else {
      missingCount++;
      Serial.printf("⚠️  Node %d MISSING in cycle %d\n", nodes[i].nodeId, currentCycle);
    }
  }

  Serial.printf("\n📊 Cycle Stats:\n");
  Serial.printf("   Received: %d/%d (%.1f%%)\n", receivedCount, TOTAL_NODES,
                (receivedCount * 100.0) / TOTAL_NODES);
  Serial.printf("   Missing: %d\n", missingCount);
  Serial.println("========================================\n");

  // Beep notification
  if (missingCount > 0) {
    beep(2, 100);  // Warning beep
  } else {
    beep(1, 50);   // Success beep
  }
}

void printStatus() {
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.printf("📊 GATEWAY STATUS | %s\n", getTimeString().c_str());
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.printf("  Uptime: %lu seconds\n", millis() / 1000);
  Serial.printf("  Current Cycle: %d\n", currentCycle);
  Serial.printf("  Cycle Second: %lu / %d\n",
                (millis() - cycleStartTime) / 1000, CYCLE_DURATION);
  Serial.printf("  WiFi: %s | NTP: %s\n",
                wifiConnected ? "✅" : "❌", ntpSynced ? "✅" : "❌");
  Serial.printf("  Packets RX: %u | TX: %u | Errors: %u\n",
                totalPacketsReceived, totalPacketsSent, totalErrors);
  uint32_t totalApiCalls = apiSuccessCount + apiFailCount;
  float apiSuccessRate = (totalApiCalls > 0) ? (apiSuccessCount * 100.0) / totalApiCalls : 0.0;
  Serial.printf("  API Success: %u | Fail: %u | Rate: %.1f%%\n",
                apiSuccessCount, apiFailCount, apiSuccessRate);
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
}

// ============================================
// UTILITY FUNCTIONS
// ============================================
uint32_t getUnixTime() {
  time_t now;
  time(&now);
  return (uint32_t)now;
}

String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return "TIME_ERROR";
  }
  char buffer[64];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

void blinkLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_BLUE, HIGH);
    delay(delayMs);
    digitalWrite(LED_BLUE, LOW);
    delay(delayMs);
  }
}

void beep(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(BUZZER, HIGH);
    delay(delayMs);
    digitalWrite(BUZZER, LOW);
    delay(delayMs);
  }
}
