#include <SPI.h>
#include <LoRa.h>

#define SCK 6
#define MISO 2
#define MOSI 7
#define SS 10
#define RST 3
#define DIO0 4
#define LED_BLUE 8

// Data hasil scan WiFi/BLE
const char* scannedMACs[] = {
  "fe:te:sd:ge:5g:01",
  "fe:te:sd:ge:5g:02",
  "fe:te:sd:ge:5g:03",
  "fe:te:sd:ge:5g:04",
  "fe:te:sd:ge:5g:05",
  "fe:te:sd:ge:5g:06",
  "fe:te:sd:ge:5g:07",
  "fe:te:sd:ge:5g:08",
  "fe:te:sd:ge:5g:09",
  "fe:te:sd:ge:5g:10",
  "fe:te:sd:ge:5g:11",
  "fe:te:sd:ge:5g:12",
  "fe:te:sd:ge:5g:13",
  "fe:te:sd:ge:5g:14",
  "fe:te:sd:ge:5g:15",
  "fe:te:sd:ge:5g:16",
  "fe:te:sd:ge:5g:17",
  "fe:te:sd:ge:5g:18",
  "fe:te:sd:ge:5g:19",
  "fe:te:sd:ge:5g:20",
  "fe:te:sd:ge:5g:21",
  "fe:te:sd:ge:5g:22",
  "fe:te:sd:ge:5g:23",
  "fe:te:sd:ge:5g:24",
  "fe:te:sd:ge:5g:25",
  "fe:te:sd:ge:5g:26",
  "fe:te:sd:ge:5g:27",
  "fe:te:sd:ge:5g:28",
  "fe:te:sd:ge:5g:29",
  "fe:te:sd:ge:5g:30",
  "fe:te:sd:ge:5g:31",
  "fe:te:sd:ge:5g:32",
  "fe:te:sd:ge:5g:33",
  "fe:te:sd:ge:5g:34",
  "fe:te:sd:ge:5g:35",
  "fe:te:sd:ge:5g:36",
  "fe:te:sd:ge:5g:37",
  "fe:te:sd:ge:5g:38",
  "fe:te:sd:ge:5g:39",
  "fe:te:sd:ge:5g:40",
  "fe:te:sd:ge:5g:41",
  "fe:te:sd:ge:5g:42",
  "fe:te:sd:ge:5g:43",
  "fe:te:sd:ge:5g:44",
  "fe:te:sd:ge:5g:45",
  "fe:te:sd:ge:5g:46",
  "fe:te:sd:ge:5g:47",
  "fe:te:sd:ge:5g:48",
  "fe:te:sd:ge:5g:49",
  "fe:te:sd:ge:5g:50",
};

const int macCount = 50;

void setup() {
  Serial.begin(115200);

  pinMode(LED_BLUE, OUTPUT);
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(433E6)) {
    Serial.println("LoRa failed!");
    while (1);
  }

  LoRa.setTxPower(20);
  LoRa.setSpreadingFactor(7);
  LoRa.setSignalBandwidth(500E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0x12);
  LoRa.enableCrc();

  Serial.println("LoRa Sender Ready!");
  Serial.println("Sending scan data...\n");
}

void loop() {
  // Kirim data scan via LoRa
  sendScanData("1", scannedMACs, macCount, "2025-10-25 08:56:35");

  Serial.println("\n✓ Data sent! Waiting 30 seconds...\n");
  delay(5000); // Kirim setiap 30 detik
}

// ===== METODE 1: COMPACT STRING (REKOMENDASI) =====
void sendScanData(const char* deviceId, const char* macs[], int count, const char* timestamp) {
  // Format: deviceId|mac1,mac2,...|timestamp
  String compactData = String(deviceId) + "|";

  for(int i = 0; i < count; i++) {
    // Hapus ":" dari MAC untuk hemat space
    String mac = String(macs[i]);
    mac.replace(":", "");
    compactData += mac;
    if(i < count - 1) compactData += ",";
  }

  compactData += "|" + String(timestamp);

  Serial.print("Compact data size: ");
  Serial.print(compactData.length());
  Serial.println(" bytes");

  // Pecah jadi beberapa paket
  sendLargeString(compactData);
}

void sendLargeString(String data) {
  int totalLen = data.length();
  int packets = (totalLen + 249) / 250; // 250 byte per paket (sisakan 5 byte header)

  Serial.print("Sending ");
  Serial.print(totalLen);
  Serial.print(" bytes in ");
  Serial.print(packets);
  Serial.println(" packets\n");

  for(int i = 0; i < packets; i++) {
    digitalWrite(LED_BLUE, HIGH);

    int start = i * 250;
    int len = min(250, totalLen - start);
    String chunk = data.substring(start, start + len);

    // Header paket
    LoRa.beginPacket();
    LoRa.write(0xAA);                    // Start marker
    LoRa.write((byte)i);                 // Nomor paket
    LoRa.write((byte)packets);           // Total paket
    LoRa.write((byte)(len >> 8));        // Panjang data (high)
    LoRa.write((byte)(len & 0xFF));      // Panjang data (low)
    LoRa.print(chunk);                   // Data
    LoRa.endPacket();

    Serial.print("Packet ");
    Serial.print(i + 1);
    Serial.print("/");
    Serial.print(packets);
    Serial.print(" sent (");
    Serial.print(len);
    Serial.println(" bytes)");

    digitalWrite(LED_BLUE, LOW);

    // Tunggu ACK
    if(waitForAck(i, 2000)) {
      Serial.println("  ✓ ACK received");
    } else {
      Serial.println("  ✗ No ACK, retrying...");
      i--; // Kirim ulang
      delay(500);
    }

    delay(100); // Jeda antar paket
  }
}

bool waitForAck(byte packetNum, unsigned long timeout) {
  unsigned long start = millis();
  while(millis() - start < timeout) {
    int size = LoRa.parsePacket();
    if(size >= 2) {
      byte marker = LoRa.read();
      byte ackNum = LoRa.read();
      if(marker == 0xBB && ackNum == packetNum) {
        return true;
      }
    }
  }
  return false;
}
