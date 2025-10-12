## 📋 **Fitur-Fitur Node:**

### ✅ **Core Functions:**

1. **Auto Sync** - Dengerin beacon dari gateway untuk sync waktu
2. **BLE Scanning** - Scan bluetooth tags selama 10 detik
3. **TDMA Slot** - Kirim data di slot yang tepat (tidak bentrok)
4. **ACK Handling** - Terima ACK dan update waktu
5. **Deep Sleep** - Tidur di luar slot untuk hemat baterai (95%!)
6. **Auto Recovery** - Re-sync otomatis jika drift terlalu besar

### 🛡️ **Safety Features:**

-   State machine yang robust
-   Retry mechanism (max 3x)
-   Drift detection & correction
-   Beacon timeout handling
-   CRC validation

### 📊 **Monitoring:**

-   Real-time status display
-   Statistics tracking
-   Visual feedback (LED + Buzzer)

---

## ⚙️ **Cara Deploy ke 30 Node:**

### **1. Setting NODE_ID untuk Setiap Kelas:**

**SANGAT PENTING:** Ganti `NODE_ID` di baris 19 untuk setiap node:

```cpp
// Node untuk Kelas 7A
#define NODE_ID 1

// Node untuk Kelas 7B
#define NODE_ID 2

// Node untuk Kelas 7C
#define NODE_ID 3

// ... dan seterusnya sampai

// Node untuk Kelas 9I
#define NODE_ID 30
```

**Mapping Kelas:**

```
Node 1-10  : Kelas 7A - 7J
Node 11-20 : Kelas 8A - 8J
Node 21-30 : Kelas 9A - 9I
```

### **2. Install Library yang Dibutuhkan:**

Di Arduino IDE:

-   `LoRa` by Sandeep Mistry
-   Built-in ESP32 BLE libraries (sudah ada di ESP32 core)

### **3. Upload Kode:**

**Untuk setiap node:**

1. Ganti `NODE_ID` sesuai kelas
2. Pilih Board: **ESP32C3 Dev Module**
3. Upload kode
4. Label fisik node dengan marker: "Node X - Kelas Y"

---

## 🔄 **Cara Kerja Node (Step by Step):**

### **Phase 1: First Boot (Listening)**

```
Boot → Initialize LoRa & BLE → STATE_LISTENING
↓
Tunggu beacon dari gateway...
↓
Terima BEACON → Sync waktu → STATE_SYNCED
```

### **Phase 2: Calculate Slot**

```
STATE_SYNCED → Hitung: "Kapan slot saya?"
↓
Node 1: Slot di detik ke-0
Node 2: Slot di detik ke-15
Node 3: Slot di detik ke-30
...
Node 30: Slot di detik ke-435
↓
Set alarm → STATE_IDLE
```

### **Phase 3: Wait for Slot**

```
STATE_IDLE → Tunggu...
↓
Jika waktu tunggu > 60 detik → DEEP SLEEP (hemat baterai)
↓
Bangun 10 detik sebelum slot
↓
Waktu slot tiba! → STATE_SCANNING
```

### **Phase 4: Scan & Transmit**

```
STATE_SCANNING → Scan BLE 10 detik → Detect MAC addresses
↓
STATE_TRANSMITTING → Kirim data via LoRa
↓
STATE_WAITING_ACK → Tunggu konfirmasi (max 2 detik)
↓
ACK received?
  ✅ YES → Update waktu → Calculate next slot → STATE_IDLE
  ❌ NO  → Retry (max 3x) → STATE_IDLE
```

### **Phase 5: Repeat**

```
Kembali ke STATE_IDLE → Tunggu cycle berikutnya (7.5 menit)
Loop terus sampai jam 17:00
```

---

## 📊 **Output Serial Monitor Node:**

```
========================================
NODE 1 - MONITORING KEHADIRAN
========================================

📍 My Slot: 0 seconds in cycle

📡 Initializing LoRa...
✅ LoRa Initialized!
   Frequency: 923.00 MHz
   SF: 7 | BW: 125 kHz

📱 Initializing BLE...
✅ BLE Initialized!

🎧 Listening for beacon...

📡 BEACON received | Cycle: 0 | Second: 0 | Time: 1697097600
✅ Time synchronized!

⏰ Next slot in 5 seconds (5000 ms)

🎯 MY SLOT TIME!

🔍 Scanning BLE tags...
   Duration: 10 seconds
  📱 Device 1: aa:bb:cc:11:22:33 (RSSI: -65)
  📱 Device 2: dd:ee:ff:44:55:66 (RSSI: -72)
  📱 Device 3: 11:22:33:aa:bb:cc (RSSI: -68)
  ...
  📱 Device 35: ff:ee:dd:cc:bb:aa (RSSI: -70)

✅ Scan complete! Found 35 devices

📋 Detected MAC Addresses:
   1. AA:BB:CC:11:22:33
   2. DD:EE:FF:44:55:66
   3. 11:22:33:AA:BB:CC
   ...
   35. FF:EE:DD:CC:BB:AA

📤 Transmitting data...
   Node: 1 | Students: 35 | Time: 1697097610

⏳ Waiting for ACK.....
   ACK Time: 1697097611 | Drift: -50 ms

✅ ACK received!

⏰ Next slot in 445 seconds (445000 ms)
😴 Deep sleep for 435 seconds to save battery...

[Node tidur untuk hemat baterai]
[Bangun otomatis 10 detik sebelum slot berikutnya]

🎯 MY SLOT TIME!
🔍 Scanning BLE tags...
...
```

---

## 💡 **Tips Deployment:**

### **1. Labeling Node (PENTING!)**

Buat label untuk setiap node:

```
┌─────────────────────────┐
│   NODE 1 - KELAS 7A     │
│   MAC: 01:23:45:67:89   │
│   Slot: 0 detik         │
└─────────────────────────┘
```

### **2. Testing Sequence:**

**Step 1: Test 1 Node Dulu**

```bash
1. Upload code dengan NODE_ID = 1
2. Nyalakan Gateway
3. Nyalakan Node 1
4. Pastikan Node 1 dapat beacon
5. Pastikan Node 1 dapat ACK
6. Cek data masuk ke API server
```

**Step 2: Test 3 Node Bersamaan**

```bash
1. Upload NODE_ID 1, 2, 3 ke 3 device berbeda
2. Nyalakan semua bersamaan
3. Pastikan tidak ada collision
4. Cek timing: Node 1 (0s), Node 2 (15s), Node 3 (30s)
```

**Step 3: Full Deployment**

```bash
1. Upload ke semua 30 node dengan NODE_ID berbeda
2. Deploy bertahap: 10 node per hari
3. Monitor dashboard untuk tracking
```

### **3. Troubleshooting per Node:**

**Problem: Node tidak sync waktu**

```
Symptom: Stuck di "🎧 Listening for beacon..."
Solution:
- Cek jarak ke gateway (RSSI harus > -100dBm)
- Pastikan gateway sudah broadcast beacon
- Cek frekuensi LoRa sama (923 MHz)
```

**Problem: Node tidak detect BLE tags**

```
Symptom: "Found 0 devices"
Solution:
- Pastikan BLE tags nyala & advertising
- Cek jarak (BLE range ~10 meter)
- Test dengan HP: scan BLE dari smartphone
```

**Problem: Node tidak dapat ACK**

```
Symptom: "ACK timeout!" terus-menerus
Solution:
- Cek timing slot (mungkin bentrok dengan node lain)
- Cek RSSI (sinyal terlalu lemah?)
- Pastikan gateway masih online
```

**Problem: Battery cepat habis**

```
Symptom: Battery habis < 1 hari
Solution:
- Pastikan deep sleep berfungsi (lihat log "😴 Deep sleep...")
- Cek apakah node stuck di scanning terus
- Reduce TX power jika RSSI bagus
```

---

## 🔋 **Estimasi Battery Life:**

### **Konsumsi Daya:**

```
Active (Scanning + TX): 200mA x 15 detik = 0.83 mAh
Deep Sleep:             10mA x 435 detik = 1.21 mAh
─────────────────────────────────────────────────
Per Cycle (7.5 menit):  2.04 mAh

Per Hari (80 cycles):   163 mAh
```

**Dengan battery 2000mAh:**

-   **Theoretical:** 2000 / 163 = **12.2 hari**
-   **Real world (70% efficiency):** **~8-9 hari**

**Tips memperpanjang battery:**

1. Gunakan battery 3000mAh+ (Li-Po 18650)
2. Reduce TX power jika RSSI bagus: `LORA_TX_POWER 14` (dari 17)
3. Gunakan SF7 (sudah optimal)
4. Disable Serial debug di production (hemat 20mA)

---

## 📝 **Checklist Deployment per Node:**

```
□ Upload code dengan NODE_ID yang benar
□ Test LoRa initialization (cek Serial Monitor)
□ Test BLE scan dengan HP/tag dummy
□ Label fisik node dengan NODE_ID & Kelas
□ Mount di dinding kelas (ketinggian 2-3 meter)
□ Test receive beacon dari gateway
□ Test kirim data & dapat ACK
□ Cek data masuk ke server
□ Test battery life minimal 24 jam
□ Dokumentasi MAC address untuk troubleshooting
```

---

## 🔧 **Modifikasi untuk Optimasi:**

### **1. Disable Serial Debug di Production:**

Tambahkan di awal file:

```cpp
// #define DEBUG_MODE  // Comment ini untuk production

#ifdef DEBUG_MODE
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(...)
#endif
```

Hemat **~20mA** saat idle!

### **2. Dynamic TX Power:**

Tambahkan fungsi:

```cpp
void adjustTxPower(int rssi) {
  if (rssi > -70) {
    LoRa.setTxPower(14);  // Dekat, power rendah
  } else if (rssi > -85) {
    LoRa.setTxPower(17);  // Sedang
  } else {
    LoRa.setTxPower(20);  // Jauh, power tinggi
  }
}
```

### **3. BLE Filter by Vendor:**

Jika BLE tags punya vendor prefix tertentu (misal: `AA:BB:CC:*`):

```cpp
bool isValidTag(std::string mac) {
  // Filter hanya MAC yang dimulai AA:BB:CC
  return (mac.substr(0, 8) == "aa:bb:cc");
}
```

Hemat waktu scan!

---

## 🚀 **Script Automation untuk Upload Massal:**

Buat script Python untuk auto-upload ke 30 node:

```python
# auto_upload.py
import subprocess
import sys

NODE_MAPPING = {
    1: "Kelas 7A", 2: "Kelas 7B", 3: "Kelas 7C",
    # ... tambahkan semua
    30: "Kelas 9I"
}

def upload_node(node_id, port):
    print(f"\n{'='*50}")
    print(f"Uploading NODE {node_id} - {NODE_MAPPING[node_id]}")
    print(f"{'='*50}\n")

    # Edit file, ganti NODE_ID
    with open('node_lora.ino', 'r') as f:
        code = f.read()

    code = code.replace(
        '#define NODE_ID 1',
        f'#define NODE_ID {node_id}'
    )

    with open('node_lora.ino', 'w') as f:
        f.write(code)

    # Upload via arduino-cli
    cmd = f'arduino-cli upload -p {port} --fqbn esp32:esp32:esp32c3'
    subprocess.run(cmd, shell=True)

    print(f"\n✅ NODE {node_id} uploaded!")
    input("Press Enter untuk node berikutnya...")

if __name__ == "__main__":
    for node_id in range(1, 31):
        port = input(f"\nConnect NODE {node_id}, then enter COM port: ")
        upload_node(node_id, port)

    print("\n🎉 All 30 nodes uploaded!")
```

---

## 📊 **Monitoring Dashboard (Bonus):**

Buat simple web dashboard untuk monitoring:

```javascript
// Real-time status 30 nodes
setInterval(() => {
    fetch("/api/nodes/status")
        .then((res) => res.json())
        .then((data) => {
            data.nodes.forEach((node) => {
                updateNodeCard(node.id, node.status, node.lastSeen);
            });
        });
}, 5000); // Update setiap 5 detik
```

---

## ✅ **Summary:**

Anda sekarang punya:

1. ✅ **Gateway Code** - Terima data dari 30 node, forward ke API
2. ✅ **Node Code** - Scan BLE, kirim data di slot yang tepat
3. ✅ **Deep Sleep** - Hemat baterai sampai 8-9 hari
4. ✅ **Auto Recovery** - Re-sync otomatis jika drift
5. ✅ **Deployment Guide** - Step-by-step setup 30 node

**Next Steps:**

1. Test gateway + 1 node dulu
2. Scale ke 3 node, cek timing
3. Deploy ke 30 node bertahap
4. Setup monitoring dashboard
5. Training maintenance team
