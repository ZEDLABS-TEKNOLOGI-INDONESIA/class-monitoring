// Node (Pengirim):
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

int counter = 0;

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
  
  // FASTEST - untuk kirim data banyak dalam jarak dekat
  LoRa.setTxPower(20);           // 20 dBm
  LoRa.setSpreadingFactor(7);    // SF7 (Tercepat)
  LoRa.setSignalBandwidth(500E3); // 500 kHz (paling cepat)
  LoRa.setCodingRate4(5);        // Error correction minimal
  LoRa.setSyncWord(0x12);        // Sync word untuk private network
  // Kecepatan: ~21 kb/s
  // Waktu kirim 200 byte: ~76 ms
  // 500 paket: ~38 detik (tanpa delay)

  Serial.println("LoRa Sender Initialized!");
  
  // Blink 3 kali tanda sukses
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_BLUE, HIGH);
    delay(200);
    digitalWrite(LED_BLUE, LOW);
    delay(200);
  }
}

void loop() {
  Serial.print("Sending packet: ");
  Serial.println(counter);
  
  // Nyalakan LED saat mulai kirim
  digitalWrite(LED_BLUE, HIGH);
  
  // Send LoRa packet
  LoRa.beginPacket();
  LoRa.print("Hello ");
  LoRa.print(counter);
  LoRa.endPacket();
  
  // Matikan LED setelah kirim
  digitalWrite(LED_BLUE, LOW);
  
  counter++;
  delay(2000); // Kirim setiap 2 detik
}
