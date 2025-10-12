#include <SPI.h>
#include <LoRa.h>

#define SCK 6
#define MISO 2
#define MOSI 7
#define SS 10
#define RST 3
#define DIO0 4
#define LED_BLUE 8
#define BUZZER 5


String receivedData = "";
byte expectedPacket = 0;
byte totalPackets = 0;

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER, OUTPUT);
  noTone(BUZZER);  // Pastikan tidak ada suara saat awal
  
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
  
  Serial.println("LoRa Receiver Ready!");
  Serial.println("Waiting for data...\n");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  
  if (packetSize >= 5) {
    digitalWrite(LED_BLUE, HIGH);
    
    // Baca header
    byte marker = LoRa.read();
    byte packetNum = LoRa.read();
    byte totalPkt = LoRa.read();
    int dataLen = (LoRa.read() << 8) | LoRa.read();
    
    if(marker == 0xAA) {
      Serial.print("Received packet ");
      Serial.print(packetNum + 1);
      Serial.print("/");
      Serial.print(totalPkt);
      Serial.print(" (");
      Serial.print(dataLen);
      Serial.println(" bytes)");

      beepOnce();
      
      // Baca data
      String chunk = "";
      for(int i = 0; i < dataLen && LoRa.available(); i++) {
        chunk += (char)LoRa.read();
      }
      
      // Kirim ACK
      sendAck(packetNum);
      
      // Simpan data
      if(packetNum == 0) {
        receivedData = ""; // Reset untuk data baru
        totalPackets = totalPkt;
      }
      
      receivedData += chunk;
      expectedPacket = packetNum + 1;
      
      // Jika semua paket diterima, parse data
      if(expectedPacket >= totalPackets) {
        Serial.println("\n✓ All packets received!");
        Serial.println("========================");
        parseCompactData(receivedData);
        Serial.println("========================\n");
        

        beepTriple();


        receivedData = "";
        expectedPacket = 0;
      }
    }
    
    digitalWrite(LED_BLUE, LOW);
  }
}

void sendAck(byte packetNum) {
  delay(20);
  LoRa.beginPacket();
  LoRa.write(0xBB); // ACK marker
  LoRa.write(packetNum);
  LoRa.endPacket();
}

void parseCompactData(String data) {
  // Format: deviceId|mac1,mac2,...|timestamp
  Serial.println("Parsing data...");
  Serial.print("Total size: ");
  Serial.print(data.length());
  Serial.println(" bytes\n");
  
  int firstPipe = data.indexOf('|');
  int secondPipe = data.indexOf('|', firstPipe + 1);
  
  if(firstPipe == -1 || secondPipe == -1) {
    Serial.println("✗ Invalid data format!");
    return;
  }

  // Extract fields
  String deviceId = data.substring(0, firstPipe);
  String macsStr = data.substring(firstPipe + 1, secondPipe);
  String timestamp = data.substring(secondPipe + 1);
  
  Serial.print("Device ID: ");
  Serial.println(deviceId);
  
  Serial.print("Timestamp: ");
  Serial.println(timestamp);
  
  Serial.println("\nMAC Addresses:");
  
  // Parse MAC addresses
  int macCount = 0;
  int startPos = 0;
  while(startPos < macsStr.length()) {
    int commaPos = macsStr.indexOf(',', startPos);
    if(commaPos == -1) commaPos = macsStr.length();
    
    String mac = macsStr.substring(startPos, commaPos);
    
    // Tambahkan ":" kembali (format: fetesd5ght -> fe:te:sd:ge:5g:ht)
    if(mac.length() == 12) {
      String formattedMac = "";
      for(int i = 0; i < 12; i += 2) {
        formattedMac += mac.substring(i, i + 2);
        if(i < 10) formattedMac += ":";
      }
      
      Serial.print("  ");
      Serial.print(++macCount);
      Serial.print(". ");
      Serial.println(formattedMac);
    }
    
    startPos = commaPos + 1;
  }
  
  Serial.print("\nTotal MACs: ");
  Serial.println(macCount);
  
  // Konversi ke JSON jika perlu
  Serial.println("\nJSON Output:");
  Serial.println("{");
  Serial.print("  \"device_id\": \"");
  Serial.print(deviceId);
  Serial.println("\",");
  Serial.println("  \"mac_address\": [");
  
  startPos = 0;
  int idx = 0;
  while(startPos < macsStr.length()) {
    int commaPos = macsStr.indexOf(',', startPos);
    if(commaPos == -1) commaPos = macsStr.length();
    
    String mac = macsStr.substring(startPos, commaPos);
    
    if(mac.length() == 12) {
      String formattedMac = "";
      for(int i = 0; i < 12; i += 2) {
        formattedMac += mac.substring(i, i + 2);
        if(i < 10) formattedMac += ":";
      }
      
      Serial.print("    \"");
      Serial.print(formattedMac);
      Serial.print("\"");
      if(startPos + mac.length() < macsStr.length()) Serial.print(",");
      Serial.println();
    }
    
    startPos = commaPos + 1;
  }
  
  Serial.println("  ],");
  Serial.print("  \"find_scan_time\": \"");
  Serial.print(timestamp);
  Serial.println("\"");
  Serial.println("}");
}

  void beepOnce() {
  tone(BUZZER, 1500); // Frekuensi 1000Hz
  delay(100);         // Durasi 100ms
  noTone(BUZZER);
}

void beepTriple() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER, 2500); // Sedikit lebih tinggi
    delay(100);
    noTone(BUZZER);
    delay(100);         // Jeda antar beep
  }
}