/*
 * NODE LORA - SISTEM MONITORING KEHADIRAN
 * Hardware: ESP32-C3 Super Mini + LoRa Ra-02
 *
 * Fungsi:
 * 1. Terima beacon dari gateway untuk sync waktu
 * 2. Scan Bluetooth tags setiap 10 detik di slot-nya
 * 3. Kirim data ke gateway via LoRa (3 paket max)
 * 4. Terima ACK dan update waktu
 * 5. Deep sleep di luar slot untuk hemat baterai
 *
 * PENTING: Setiap node harus diset NODE_ID yang berbeda (1-30)
 */

#include <SPI.h>
#include <LoRa.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Preferences.h>
#include <sys/time.h>

// ============================================
// NODE CONFIGURATION - GANTI INI UNTUK SETIAP NODE!
// ============================================
#define NODE_ID 1  // GANTI: 1-30 (Node 1 = Kelas 7A, Node 2 = Kelas 7B, dst)

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
// BLE CONFIGURATION
// ============================================
#define BLE_SCAN_TIME 10          // 10 detik scan
#define MAX_BLE_DEVICES 40        // Max 40 siswa per kelas

// ============================================
// TIMING CONFIGURATION
// ============================================
#define CYCLE_DURATION 450        // 7.5 menit = 450 detik
#define SLOT_DURATION 15          // 15 detik per node
#define SCAN_DURATION 10          // 10 detik scan
#define TX_DURATION 5             // 5 detik transmit
#define BEACON_TIMEOUT 90000      // 90 detik timeout beacon
#define ACK_TIMEOUT 2000          // 2 detik timeout ACK
#define MAX_RETRY 3               // Max retry jika gagal
#define DRIFT_THRESHOLD 2000      // 2 detik max drift

// ============================================
// PACKET TYPES
// ============================================
#define PKT_TYPE_BEACON 0x01
#define PKT_TYPE_DATA 0x02
#define PKT_TYPE_ACK 0x03

// ============================================
// STATE MACHINE
// ============================================
enum NodeState {
  STATE_INIT,
  STATE_LISTENING,      // Tunggu beacon pertama
  STATE_SYNCED,         // Sudah sync waktu
  STATE_IDLE,           // Tunggu slot
  STATE_SCANNING,       // Scan BLE
  STATE_TRANSMITTING,   // Kirim data
  STATE_WAITING_ACK,    // Tunggu ACK
  STATE_SLEEP,          // Deep sleep
  STATE_ERROR           // Error state
};

// ============================================
// STRUCTURES
// ============================================
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
  uint8_t payload[180];  // MAC addresses (max 30 x 6 bytes)
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

struct MACAddress {
  uint8_t addr[6];
};

// ============================================
// GLOBAL VARIABLES
// ============================================
NodeState currentState = STATE_INIT;
Preferences preferences;

// Timing
uint32_t mySlotTime = 0;           // Detik ke berapa dalam cycle
uint32_t nextSlotMillis = 0;       // Kapan slot berikutnya (millis)
uint32_t lastBeaconTime = 0;       // Terakhir terima beacon
uint32_t lastSyncTime = 0;         // Terakhir sync waktu
int32_t driftMs = 0;               // Clock drift dalam ms
bool timeSynced = false;

// BLE Scan Results
MACAddress detectedMACs[MAX_BLE_DEVICES];
uint8_t detectedCount = 0;

// Statistics
uint32_t cyclesCompleted = 0;
uint32_t packetsSuccess = 0;
uint32_t packetsFailed = 0;
uint32_t ackReceived = 0;
uint32_t ackMissed = 0;

// BLE Scan
BLEScan* pBLEScan;

// ============================================
// BLE SCAN CALLBACK
// ============================================
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (detectedCount >= MAX_BLE_DEVICES) return;

    // Get MAC address as String (Arduino String, not std::string)
    String macStr = advertisedDevice.getAddress().toString().c_str();

    // Check if already exists
    bool exists = false;
    for (int i = 0; i < detectedCount; i++) {
      char existingMAC[18];
      sprintf(existingMAC, "%02x:%02x:%02x:%02x:%02x:%02x",
              detectedMACs[i].addr[0], detectedMACs[i].addr[1],
              detectedMACs[i].addr[2], detectedMACs[i].addr[3],
              detectedMACs[i].addr[4], detectedMACs[i].addr[5]);

      if (strcasecmp(existingMAC, macStr.c_str()) == 0) {
        exists = true;
        break;
      }
    }

    if (!exists) {
      // Parse MAC address
      sscanf(macStr.c_str(), "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
             &detectedMACs[detectedCount].addr[0],
             &detectedMACs[detectedCount].addr[1],
             &detectedMACs[detectedCount].addr[2],
             &detectedMACs[detectedCount].addr[3],
             &detectedMACs[detectedCount].addr[4],
             &detectedMACs[detectedCount].addr[5]);

      detectedCount++;

      Serial.printf("  📱 Device %d: %s (RSSI: %d)\n",
                    detectedCount, macStr.c_str(), advertisedDevice.getRSSI());
    }
  }
};

// ============================================
// SETUP
// ============================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\n========================================");
  Serial.printf("NODE %d - MONITORING KEHADIRAN\n", NODE_ID);
  Serial.println("========================================\n");

  // Validate Node ID
  if (NODE_ID < 1 || NODE_ID > 30) {
    Serial.println("❌ ERROR: NODE_ID harus 1-30!");
    while(1) {
      blinkLED(5, 100);
      delay(1000);
    }
  }

  // Setup GPIO
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(BUZZER, LOW);

  // Beep startup
  beep(2, 100);

  // Initialize Preferences (untuk save state)
  preferences.begin("attendance", false);

  // Calculate my slot time
  mySlotTime = (NODE_ID - 1) * SLOT_DURATION;  // Node 1 = 0s, Node 2 = 15s, dst
  Serial.printf("📍 My Slot: %u seconds in cycle\n", mySlotTime);

  // Initialize LoRa
  initLoRa();

  // Initialize BLE
  initBLE();

  // Start in LISTENING state
  currentState = STATE_LISTENING;
  Serial.println("\n🎧 Listening for beacon...\n");

  blinkLED(3, 50);
}

// ============================================
// MAIN LOOP
// ============================================
void loop() {
  unsigned long currentMillis = millis();

  switch (currentState) {

    case STATE_LISTENING:
      // Tunggu beacon pertama untuk sync waktu
      listenForBeacon();

      // Timeout check
      if (currentMillis - lastBeaconTime > BEACON_TIMEOUT && lastBeaconTime > 0) {
        Serial.println("⚠️  Beacon timeout! Re-listening...");
        lastBeaconTime = 0;
      }
      break;

    case STATE_SYNCED:
      // Baru sync, hitung kapan slot berikutnya
      calculateNextSlot();
      currentState = STATE_IDLE;
      break;

    case STATE_IDLE:
      // Tunggu sampai waktu slot
      if (currentMillis >= nextSlotMillis) {
        Serial.println("\n🎯 MY SLOT TIME!");
        currentState = STATE_SCANNING;
      }

      // Listen beacon untuk update (passive)
      listenForBeacon();

      // Check drift - kalau terlalu besar, re-sync
      if (abs(driftMs) > DRIFT_THRESHOLD) {
        Serial.printf("⚠️  Drift too large: %d ms. Re-syncing...\n", driftMs);
        currentState = STATE_LISTENING;
        timeSynced = false;
      }
      break;

    case STATE_SCANNING:
      // Scan BLE untuk 10 detik
      scanBLETags();
      currentState = STATE_TRANSMITTING;
      break;

    case STATE_TRANSMITTING:
      // Kirim data ke gateway
      transmitData();
      currentState = STATE_WAITING_ACK;
      break;

    case STATE_WAITING_ACK:
      // Tunggu ACK dari gateway
      if (waitForAck()) {
        Serial.println("✅ ACK received!");
        ackReceived++;
        packetsSuccess++;
        beep(1, 30);

        // Calculate next slot
        cyclesCompleted++;
        calculateNextSlot();
        currentState = STATE_IDLE;
      } else {
        Serial.println("❌ ACK timeout!");
        ackMissed++;
        packetsFailed++;

        // Retry or skip
        static int retryCount = 0;
        if (retryCount < MAX_RETRY) {
          Serial.printf("🔄 Retry %d/%d\n", retryCount + 1, MAX_RETRY);
          retryCount++;
          currentState = STATE_TRANSMITTING;
        } else {
          Serial.println("⚠️  Max retry reached. Skipping to next cycle.");
          retryCount = 0;
          calculateNextSlot();
          currentState = STATE_IDLE;
        }
      }
      break;

    case STATE_ERROR:
      Serial.println("❌ ERROR STATE! Rebooting...");
      delay(5000);
      ESP.restart();
      break;
  }

  // Print status setiap 30 detik
  static unsigned long lastStatusPrint = 0;
  if (currentMillis - lastStatusPrint >= 30000) {
    printStatus();
    lastStatusPrint = currentMillis;
  }

  delay(10);  // Small delay untuk stabilitas
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
  Serial.printf("   SF: %d | BW: %.0f kHz\n",
                LORA_SPREADING_FACTOR, LORA_BANDWIDTH / 1E3);
}

void listenForBeacon() {
  int packetSize = LoRa.parsePacket();
  if (packetSize == 0) return;

  // Read packet
  uint8_t buffer[256];
  int idx = 0;
  while (LoRa.available() && idx < 256) {
    buffer[idx++] = LoRa.read();
  }

  // Check if beacon
  if (buffer[0] == PKT_TYPE_BEACON && idx >= sizeof(BeaconPacket)) {
    BeaconPacket* beacon = (BeaconPacket*)buffer;

    Serial.printf("\n📡 BEACON received | Cycle: %d | Second: %d | Time: %u\n",
                  beacon->cycleNumber, beacon->cycleSecond, beacon->timestamp);

    // Update system time
    struct timeval tv;
    tv.tv_sec = beacon->timestamp;
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);

    lastBeaconTime = millis();
    lastSyncTime = millis();

    if (!timeSynced) {
      Serial.println("✅ Time synchronized!");
      timeSynced = true;
      currentState = STATE_SYNCED;
      blinkLED(3, 100);
    }
  }
}

void transmitData() {
  Serial.println("\n📤 Transmitting data...");

  // Prepare packet
  DataPacket packet;
  packet.type = PKT_TYPE_DATA;
  packet.nodeId = NODE_ID;
  packet.packetSeq = 1;  // Simplified - single packet
  packet.timestamp = getUnixTime();
  packet.studentCount = detectedCount;
  packet.payloadLength = detectedCount * 6;

  // Copy MAC addresses to payload
  memcpy(packet.payload, detectedMACs, detectedCount * 6);

  // Calculate CRC (simplified)
  packet.crc = calculateCRC((uint8_t*)&packet, sizeof(packet) - 2);

  // Send packet
  LoRa.beginPacket();
  LoRa.write((uint8_t*)&packet, sizeof(packet));
  LoRa.endPacket();

  Serial.printf("   Node: %d | Students: %d | Time: %u\n",
                packet.nodeId, packet.studentCount, packet.timestamp);

  blinkLED(2, 50);
}

bool waitForAck() {
  unsigned long startTime = millis();

  Serial.print("⏳ Waiting for ACK");

  while (millis() - startTime < ACK_TIMEOUT) {
    Serial.print(".");

    int packetSize = LoRa.parsePacket();
    if (packetSize > 0) {
      uint8_t buffer[256];
      int idx = 0;
      while (LoRa.available() && idx < 256) {
        buffer[idx++] = LoRa.read();
      }

      // Check if ACK for this node
      if (buffer[0] == PKT_TYPE_ACK && idx >= sizeof(AckPacket)) {
        AckPacket* ack = (AckPacket*)buffer;

        if (ack->nodeId == NODE_ID) {
          Serial.println();
          Serial.printf("   ACK Time: %u | Drift: %d ms\n",
                        ack->timestamp, ack->driftMs);

          // Update time from ACK
          struct timeval tv;
          tv.tv_sec = ack->timestamp;
          tv.tv_usec = 0;
          settimeofday(&tv, NULL);

          driftMs = ack->driftMs;
          lastSyncTime = millis();

          return true;
        }
      }
    }

    delay(50);
  }

  Serial.println(" TIMEOUT");
  return false;
}

// ============================================
// BLE FUNCTIONS
// ============================================
void initBLE() {
  Serial.println("📱 Initializing BLE...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);  // Active scan lebih akurat
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);

  Serial.println("✅ BLE Initialized!");
}

void scanBLETags() {
  Serial.println("\n🔍 Scanning BLE tags...");
  Serial.printf("   Duration: %d seconds\n", BLE_SCAN_TIME);

  // Reset counter
  detectedCount = 0;
  memset(detectedMACs, 0, sizeof(detectedMACs));

  digitalWrite(LED_BLUE, HIGH);

  // Start scan - return pointer
  BLEScanResults* pResults = pBLEScan->start(BLE_SCAN_TIME, false);

  digitalWrite(LED_BLUE, LOW);

  Serial.printf("✅ Scan complete! Found %d devices\n", detectedCount);

  // Print all MACs
  if (detectedCount > 0) {
    Serial.println("\n📋 Detected MAC Addresses:");
    for (int i = 0; i < detectedCount; i++) {
      Serial.printf("   %d. %02X:%02X:%02X:%02X:%02X:%02X\n", i + 1,
                    detectedMACs[i].addr[0], detectedMACs[i].addr[1],
                    detectedMACs[i].addr[2], detectedMACs[i].addr[3],
                    detectedMACs[i].addr[4], detectedMACs[i].addr[5]);
    }
  } else {
    Serial.println("⚠️  No devices detected!");
  }

  pBLEScan->clearResults();
  delete pResults;  // Clean up

  beep(2, 50);
}

// ============================================
// TIMING FUNCTIONS
// ============================================
void calculateNextSlot() {
  uint32_t currentTime = getUnixTime();
  uint32_t currentSecondInDay = currentTime % 86400;  // Detik sejak midnight

  // Hitung detik sejak awal cycle terdekat
  uint32_t secondsSinceCycleStart = currentSecondInDay % CYCLE_DURATION;

  // Hitung detik sampai slot saya berikutnya
  uint32_t secondsToMySlot;
  if (secondsSinceCycleStart < mySlotTime) {
    // Slot saya masih di cycle ini
    secondsToMySlot = mySlotTime - secondsSinceCycleStart;
  } else {
    // Slot saya di cycle berikutnya
    secondsToMySlot = (CYCLE_DURATION - secondsSinceCycleStart) + mySlotTime;
  }

  nextSlotMillis = millis() + (secondsToMySlot * 1000);

  Serial.printf("⏰ Next slot in %u seconds (%lu ms)\n",
                secondsToMySlot, nextSlotMillis);

  // Deep sleep jika waktu tunggu > 60 detik
  if (secondsToMySlot > 60) {
    Serial.printf("😴 Deep sleep for %u seconds to save battery...\n",
                  secondsToMySlot - 10);  // Bangun 10 detik lebih awal
    delay(100);

    // Deep sleep (bangun 10 detik sebelum slot)
    esp_sleep_enable_timer_wakeup((secondsToMySlot - 10) * 1000000ULL);
    esp_deep_sleep_start();
  }
}

uint32_t getUnixTime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec;
}

String getTimeString() {
  time_t now = getUnixTime();
  struct tm* timeinfo = localtime(&now);
  char buffer[64];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
  return String(buffer);
}

// ============================================
// UTILITY FUNCTIONS
// ============================================
uint16_t calculateCRC(uint8_t* data, size_t length) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < length; i++) {
    crc ^= data[i];
    for (int j = 0; j < 8; j++) {
      if (crc & 0x0001) {
        crc = (crc >> 1) ^ 0xA001;
      } else {
        crc = crc >> 1;
      }
    }
  }
  return crc;
}

void printStatus() {
  Serial.println("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.printf("📊 NODE %d STATUS | %s\n", NODE_ID, getTimeString().c_str());
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
  Serial.printf("  State: ");
  switch(currentState) {
    case STATE_LISTENING: Serial.println("🎧 LISTENING"); break;
    case STATE_SYNCED: Serial.println("✅ SYNCED"); break;
    case STATE_IDLE: Serial.println("⏸️  IDLE"); break;
    case STATE_SCANNING: Serial.println("🔍 SCANNING"); break;
    case STATE_TRANSMITTING: Serial.println("📤 TRANSMITTING"); break;
    case STATE_WAITING_ACK: Serial.println("⏳ WAITING_ACK"); break;
    default: Serial.println("❓ UNKNOWN"); break;
  }
  Serial.printf("  Uptime: %lu sec\n", millis() / 1000);
  Serial.printf("  Time Synced: %s\n", timeSynced ? "YES" : "NO");
  Serial.printf("  Drift: %d ms\n", driftMs);
  Serial.printf("  My Slot: %u sec\n", mySlotTime);
  Serial.printf("  Cycles: %u\n", cyclesCompleted);
  Serial.printf("  Success: %u | Failed: %u\n", packetsSuccess, packetsFailed);
  Serial.printf("  ACK RX: %u | Missed: %u\n", ackReceived, ackMissed);
  Serial.printf("  Last Devices: %d\n", detectedCount);
  Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
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
