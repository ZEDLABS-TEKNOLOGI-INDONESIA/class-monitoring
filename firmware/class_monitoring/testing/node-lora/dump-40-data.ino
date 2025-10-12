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
  "fe:te:sd:ge:5g:ht",
  "fe:te:sd:ge:5g:76",
  "fe:te:sd:ge:5g:jj",
  "fe:te:sd:ge:5g:5j",
  "fe:te:sg:ge:5g:9j",
  "fe:te:sd:ge:5g:a1",
  "fe:te:sd:ge:5g:b2",
  "fe:te:sd:ge:5g:c3",
  "fe:te:sd:ge:5g:d4",
  "fe:te:sd:ge:5g:e5",
  "fe:te:sd:ge:5g:f6",
  "fe:te:sg:ge:5g:k7",
  "fe:te:sg:ge:5g:l8",
  "fe:te:sg:ge:5g:m9",
  "fe:te:sg:ge:5g:n0",
  "fe:te:sd:hf:5g:p1",
  "fe:te:sd:hf:5g:q2",
  "fe:te:sd:hf:5g:r3",
  "fe:te:sd:hf:5g:s4",
  "fe:te:sd:hf:5g:t5",
  "fe:te:sg:hf:5g:u6",
  "fe:te:sg:hf:5g:v7",
  "fe:te:sg:hf:5g:w8",
  "fe:te:sg:hf:5g:x9",
  "fe:te:sg:hf:5g:y0",
  "fe:te:sd:jk:5g:z1",
  "fe:te:sd:jk:5g:aa",
  "fe:te:sd:jk:5g:bb",
  "fe:te:sd:jk:5g:cc",
  "fe:te:sd:jk:5g:dd",
  "fe:te:sg:jk:5g:ee",
  "fe:te:sg:jk:5g:ff",
  "fe:te:sg:jk:5g:gg",
  "fe:te:sg:jk:5g:hh",
  "fe:te:sg:jk:5g:ii",
  "fe:te:sd:lm:5g:kk"
};

const int macCount = 36;

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