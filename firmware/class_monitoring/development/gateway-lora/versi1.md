## 📋 **Fitur-Fitur yang Sudah Diimplementasikan:**

### ✅ **Core Functions:**

1. **Sinkronisasi NTP** - Auto sync waktu dari internet setiap 1 jam
2. **Broadcast Beacon** - Kirim beacon setiap 30 detik untuk sync node
3. **Receive Data** - Terima data dari 30 node dengan validasi
4. **Send ACK** - Kirim konfirmasi + timestamp update ke node
5. **Forward ke API** - Kirim data ke server via HTTP POST (JSON)
6. **Monitoring** - Track status semua node, deteksi node offline

### 🛡️ **Safety & Reliability:**

-   Validasi packet (size, node ID, CRC)
-   Error handling & retry mechanism
-   Node health monitoring (deteksi offline)
-   Statistics tracking (success/fail rate)
-   Auto WiFi reconnection

### 📊 **Monitoring & Logging:**

-   Real-time status display
-   Cycle completion report
-   Missing node detection
-   API success/fail tracking
-   Visual feedback (LED + Buzzer)

---

## ⚙️ **Cara Menggunakan:**

### **1. Install Library yang Dibutuhkan:**

Di Arduino IDE, install library berikut via Library Manager:

-   `LoRa` by Sandeep Mistry
-   `ArduinoJson` by Benoit Blanchon (v6.x)

### **2. Konfigurasi WiFi & API:**

Edit bagian ini di kode:

```cpp
const char* WIFI_SSID = "YOUR_WIFI_SSID";        // Ganti dengan WiFi sekolah
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD"; // Password WiFi

const char* API_ENDPOINT = "http://your-server.com/api/attendance";
const char* API_KEY = "your_api_key_here";
```

### **3. Upload ke ESP32-C3:**

-   Pilih Board: **ESP32C3 Dev Module**
-   Upload kode
-   Buka Serial Monitor (115200 baud)

---

## 📡 **Format Data yang Dikirim ke API:**

```json
{
  "gateway_id": "GW001",
  "node_id": 1,
  "timestamp": 1697097600,
  "student_count": 35,
  "rssi": -65,
  "snr": 8.5,
  "cycle": 42,
  "mac_addresses": [
    "AA:BB:CC:11:22:33",
    "DD:EE:FF:44:55:66",
    ...
  ]
}
```

---

## 🔍 **Cara Kerja:**

1. **Boot** → Connect WiFi → Sync NTP → Initialize LoRa
2. **Loop:**
    - Broadcast beacon tiap 30 detik
    - Listen untuk data dari node
    - Validasi data yang masuk
    - Kirim ACK ke node
    - Forward data ke API server
    - Monitor health semua node
3. **Cycle Complete** → Print report → Reset flags

---

## 📊 **Output Serial Monitor:**

```
========================================
GATEWAY LORA - MONITORING KEHADIRAN
========================================

🔌 Connecting to WiFi: SekolahWiFi
✅ WiFi Connected!
   IP Address: 192.168.1.100

🕐 Syncing with NTP Server...
✅ NTP Synchronized!
   Current Time: 2024-10-12 06:55:00

📡 Initializing LoRa...
✅ LoRa Initialized!
   Frequency: 923.00 MHz
   Bandwidth: 125 kHz
   SF: 7
   TX Power: 17 dBm

✅ GATEWAY READY!

📡 BEACON sent | Cycle: 0 | Second: 0 | Time: 2024-10-12 07:00:00

📥 DATA from Node 1 | Seq: 1 | Students: 35 | RSSI: -68 | SNR: 9.2
📤 ACK sent to Node 1 | Timestamp: 1697097610
✅ API Success | Node 1 | Code: 200

📥 DATA from Node 2 | Seq: 1 | Students: 38 | RSSI: -72 | SNR: 8.5
...
```

---

## 🚨 **Troubleshooting:**

**WiFi tidak connect:**

-   Cek SSID & password
-   Pastikan ESP32 dalam jangkauan WiFi

**NTP sync gagal:**

-   Cek koneksi internet
-   Coba ganti NTP server: `time.google.com`

**LoRa tidak initialize:**

-   Cek wiring SPI
-   Pastikan pin definitions sesuai hardware

**API error:**

-   Cek endpoint URL
-   Pastikan server bisa diakses dari gateway
-   Validasi format JSON yang diterima server
