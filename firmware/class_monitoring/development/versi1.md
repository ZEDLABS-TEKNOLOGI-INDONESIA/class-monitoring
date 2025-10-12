# 📋 **SKENARIO LENGKAP SISTEM MONITORING KEHADIRAN**

---

## 🌅 **PAGI HARI - JAM 06:55 (Sebelum Operasi Dimulai)**

### **Step 1: Gateway Bangun & Persiapan**

**06:55:00 - Gateway dinyalakan**

-   Gateway boot up
-   Konek ke WiFi sekolah
-   Ambil waktu dari NTP Server internet: "Sekarang jam 06:55:00"
-   Gateway punya jam yang **super akurat** sekarang

**06:55:05 - Gateway mulai broadcast BEACON**

-   Gateway teriak (broadcast LoRa) setiap **30 detik**:
    ```
    "Halo semua node! Sekarang jam 06:55:05
     Cycle belum mulai, standby dulu
     Operasi dimulai jam 07:00:00"
    ```

---

### **Step 2: Node-Node Bangun Satu Per Satu**

**06:56:00 - Node 1 (Kelas 7A) dinyalakan**

-   Node 1 boot up
-   Belum tahu jam berapa
-   Langsung masuk **MODE LISTENING** (dengerin terus)
-   Menunggu beacon dari gateway...

**06:56:30 - Node 1 terima BEACON pertama**

```
Gateway broadcast: "Jam 06:56:30, cycle belum mulai"
```

**Node 1 proses:**

-   "OK, sekarang jam 06:56:30"
-   "Saya Node ID 1, berarti slot saya di detik ke-0"
-   "Operasi mulai jam 07:00:00"
-   "Berarti saya harus mulai kerja di 07:00:00 tepat"
-   Set alarm internal: **07:00:00**
-   Sekarang **TIDUR** sampai jam 07:00:00 (hemat baterai)

---

**06:57:00 - Node 2 (Kelas 7B) dinyalakan**

**06:57:30 - Node 2 terima BEACON**

```
Gateway: "Jam 06:57:30, cycle belum mulai"
```

**Node 2 proses:**

-   "OK, jam 06:57:30"
-   "Saya Node ID 2, slot saya di detik ke-15"
-   "Berarti saya mulai di 07:00:15"
-   Set alarm: **07:00:15**
-   **TIDUR** sampai 07:00:15

---

**06:58:00 - Node 3, 4, 5... sampai Node 30 dinyalakan**

Semua node:

1. Dengerin beacon
2. Dapat waktu yang akurat
3. Hitung slot masing-masing:
    - Node 3 → detik ke-30 → start 07:00:30
    - Node 4 → detik ke-45 → start 07:00:45
    - ...
    - Node 30 → detik ke-435 → start 07:07:15
4. Tidur sampai waktu mereka

---

## 🏫 **OPERASI DIMULAI - JAM 07:00:00**

### **CYCLE 1 (Putaran Pertama)**

---

#### **07:00:00 - Node 1 BANGUN (Giliran Pertama)**

**07:00:00-07:00:10 - SCAN BLE (10 detik)**

-   Node 1 scan bluetooth tag di kelas 7A
-   Deteksi 35 siswa hadir (35 MAC address)
-   Simpan data di memory

**07:00:10-07:00:15 - KIRIM DATA (5 detik)**

-   Node 1 kirim data via LoRa ke gateway:
    ```
    PAKET:
    - Node ID: 1
    - Timestamp: 07:00:10
    - Jumlah siswa: 35
    - MAC List: [AA:BB:CC:11:22:33, DD:EE:FF:44:55:66, ...]
    ```

**07:00:11 - Gateway terima data Node 1**

-   Gateway: "OK, terima dari Node 1"
-   Kirim ACK (konfirmasi) ke Node 1:
    ```
    ACK:
    - "Data diterima!"
    - "Sekarang jam 07:00:11 (update jam kamu)"
    - "Drift kamu: -50ms (jam kamu telat 50ms)"
    ```

**07:00:11 - Node 1 terima ACK**

-   "OK, data terkirim! ✅"
-   Update jam: koreksi drift -50ms
-   Hitung giliran berikutnya: 07:07:30 (cycle ke-2)
-   Set alarm: **07:07:30**
-   Kembali **TIDUR** (hemat baterai)

**07:00:12 - Gateway kirim data ke Server**

-   Gateway ubah data jadi JSON
-   Kirim ke API server via internet (HTTP POST)

**07:00:13 - Server terima & proses**

-   Validasi 35 MAC address
-   Cek: Apakah siswa-siswa ini terdaftar? ✅
-   Cek: Apakah mereka ada jadwal di kelas 7A sekarang? ✅
-   Cek: Apakah sudah ada data sebelumnya (cegah duplikat)? ❌ Belum
-   **SIMPAN** ke database ✅

---

#### **07:00:15 - Node 2 BANGUN (Giliran Kedua)**

**07:00:15-07:00:25 - Node 2 scan BLE**

-   Deteksi 38 siswa di kelas 7B

**07:00:25-07:00:30 - Node 2 kirim data**

-   Kirim ke gateway

**07:00:26 - Gateway terima & kirim ACK**

-   Node 2 dapat update waktu
-   Node 2 tidur lagi sampai 07:07:45 (cycle ke-2)

**07:00:27 - Gateway kirim ke server**

-   Server validasi & simpan data kelas 7B ✅

---

#### **07:00:30 - Node 3 mulai...**

#### **07:00:45 - Node 4 mulai...**

#### **...dan seterusnya...**

---

#### **07:07:15 - Node 30 BANGUN (Giliran Terakhir Cycle 1)**

**07:07:15-07:07:25 - Node 30 scan BLE**

-   Deteksi 40 siswa di kelas 9I

**07:07:25-07:07:30 - Node 30 kirim data**

-   Kirim ke gateway
-   Dapat ACK + update waktu
-   Tidur sampai 07:14:45 (cycle ke-2)

**07:07:31 - Gateway kirim ke server**

-   Data kelas 9I tersimpan ✅

---

### **✅ CYCLE 1 SELESAI (7 menit 30 detik)**

**Hasil:**

-   30 kelas sudah lapor semua
-   ~1050 siswa tercatat kehadirannya
-   Data tersimpan di database server

---

## 🔄 **CYCLE 2 DIMULAI - JAM 07:07:30**

**07:07:30 - Node 1 BANGUN LAGI**

-   Mulai scan BLE lagi
-   Kirim data lagi
-   Dapat update waktu dari ACK
-   Tidur lagi sampai cycle 3

**07:07:45 - Node 2 bangun...**
**07:08:00 - Node 3 bangun...**
**...dan seterusnya sampai 07:15:00**

---

## 🔄 **CYCLE 3, 4, 5... TERUS BERLANJUT**

Dalam **10 jam operasi** (07:00-17:00):

-   Total waktu: 600 menit
-   1 cycle: 7.5 menit
-   Total cycle: **~80 cycle per hari**
-   Setiap siswa kehadirannya dicek **80 kali** dalam sehari

---

## 📡 **SISTEM KEAMANAN BERJALAN BERSAMAAN**

### **Setiap 30 detik - Gateway broadcast BEACON**

```
"Jam 07:15:00, Cycle ke-2, detik ke-450"
```

**Fungsi beacon:**

-   Untuk node yang baru nyala (bisa langsung sync)
-   Untuk node yang kehilangan tracking (bisa re-sync)
-   Untuk koreksi drift besar

---

### **Setiap 1 jam - Full Re-Sync**

**08:00:00**

-   Gateway sync ulang ke NTP server
-   Broadcast penuh: "RESET! Ini waktu yang benar!"
-   Semua node update jam mereka
-   Reset akumulasi drift

---

## 🚨 **SKENARIO MASALAH & SOLUSI**

### **Masalah 1: Node 5 Gagal Kirim Data**

**07:01:00 - Node 5 kirim data**

-   Sinyal lemah/interference
-   Gateway **TIDAK** terima data

**07:01:05 - Node 5: "Kok nggak ada ACK ya?"**

-   Tunggu 500ms
-   Masih nggak ada ACK
-   **RETRY #1**: Kirim ulang data

**07:01:06 - Masih gagal**

-   **RETRY #2**: Kirim ulang lagi

**07:01:07 - Akhirnya berhasil**

-   Gateway terima
-   Kirim ACK
-   Node 5: "OK deh, sekarang berhasil ✅"

**Kalau 3x retry gagal semua:**

-   Node 5 simpan data di **memory lokal** (SPIFFS)
-   Coba kirim lagi di cycle berikutnya
-   Gateway catat: "Node 5 missing di cycle 1"
-   Kirim alert ke admin: "⚠️ Node 5 bermasalah"

---

### **Masalah 2: Gateway Mati**

**07:15:00 - Gateway utama mati (listrik padam)**

**07:15:15 - Node 1 kirim data**

-   Nggak ada yang jawab
-   Retry 3x, tetap gagal
-   Node 1 simpan data lokal

**07:15:30 - Node 2 juga gagal kirim**

-   Simpan data lokal juga

**07:16:00 - Gateway BACKUP otomatis aktif**

-   Ambil alih tugas gateway utama
-   Broadcast: "Halo, saya gateway baru kalian!"

**07:16:15 - Node 1 coba kirim lagi**

-   Kali ini berhasil ke gateway backup!
-   Node 1 juga kirim **data yang tersimpan tadi**
-   Semua data terkirim ✅

---

### **Masalah 3: Jam Node Drift (Bergeser)**

**10:00:00 - Node 7 jam-nya drift +2 detik**

-   Seharusnya mulai jam 10:00:00
-   Tapi dia mulai jam 10:00:02
-   **Tabrakan** dengan Node 8!

**10:00:05 - Node 7 terima ACK**

```
ACK: "Drift kamu: +2000ms (kamu terlalu cepat 2 detik!)"
```

**Node 7 deteksi:**

-   "Wah, drift saya >2 detik, bahaya nih"
-   **MASUK MODE RE-SYNC**
-   Dengerin beacon berikutnya
-   Re-calculate slot
-   Kembali normal

---

## 🎯 **KESIMPULAN SKENARIO**

**Sistem bekerja seperti ini:**

1. ⏰ **Pagi:** Gateway sync NTP, broadcast beacon, node-node sync waktu
2. 🔄 **Operasi:** Node kerja bergiliran setiap 15 detik, total 7.5 menit per cycle
3. 📡 **Terus-menerus:** 80 cycle per hari, data terupdate real-time
4. 🛡️ **Safety:** Retry otomatis, backup gateway, local storage, re-sync berkala
5. 💾 **Server:** Validasi data, cegah duplikasi, simpan ke database

**Hasilnya:**

-   ✅ Kehadiran tercatat otomatis setiap ~7.5 menit
-   ✅ Tidak ada collision (bentrokan)
-   ✅ Reliable (99.5% uptime)
-   ✅ Hemat baterai (deep sleep)
-   ✅ Auto-recovery kalau ada masalah
