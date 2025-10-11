// Gateway (Penerima):
#include <SPI.h>
#include <LoRa.h>

// Pin definitions untuk LoRa RA-02
#define SCK 6
#define MISO 2
#define MOSI 7
#define SS 10
#define RST 3
#define DIO0 4

// LED biru bawaan ESP32-C3 Super Mini
#define LED_BLUE 8

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  // Setup LED
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
  
  // Setup LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  
  // Initialize LoRa (433MHz untuk Indonesia)
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1) {
      // Blink error pattern
      digitalWrite(LED_BLUE, HIGH);
      delay(100);
      digitalWrite(LED_BLUE, LOW);
      delay(100);
    }
  }
  
  // Set parameter LoRa (HARUS SAMA dengan sender)
  // FASTEST - untuk terima data banyak dalam jarak dekat
  LoRa.setTxPower(20);           // 20 dBm
  LoRa.setSpreadingFactor(7);    // SF7 (Tercepat)
  LoRa.setSignalBandwidth(500E3); // 500 kHz (paling cepat)
  LoRa.setCodingRate4(5);        // Error correction minimal
  LoRa.setSyncWord(0x12);        // Sync word untuk private network
  // Kecepatan: ~21 kb/s
  // Waktu terima 200 byte: ~76 ms
  // 500 paket: ~38 detik (tanpa delay)
  
  Serial.println("LoRa Receiver Initialized!");
  
  // Blink 3 kali tanda sukses
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_BLUE, HIGH);
    delay(200);
    digitalWrite(LED_BLUE, LOW);
    delay(200);
  }
}

void loop() {
  // Check untuk paket masuk
  int packetSize = LoRa.parsePacket();
  
  if (packetSize) {
    // Nyalakan LED saat terima data
    digitalWrite(LED_BLUE, HIGH);
    
    // Baca paket
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }
    
    // Baca RSSI (kekuatan sinyal)
    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();
    
    // Print ke Serial
    Serial.print("Received: ");
    Serial.print(received);
    Serial.print(" | RSSI: ");
    Serial.print(rssi);
    Serial.print(" dBm | SNR: ");
    Serial.print(snr);
    Serial.println(" dB");
    
    // Kedipkan LED (blink cepat 2 kali)
    delay(100);
    digitalWrite(LED_BLUE, LOW);
    delay(100);
    digitalWrite(LED_BLUE, HIGH);
    delay(100);
    digitalWrite(LED_BLUE, LOW);
  }
}