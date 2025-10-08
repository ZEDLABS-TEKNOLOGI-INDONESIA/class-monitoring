# 📚 Monitoring Class - Sistem Monitoring Kehadiran Berbasis IoT

<div align="center">

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![IoT](https://img.shields.io/badge/IoT-Enabled-orange.svg)
![Laravel](https://img.shields.io/badge/Laravel-10.x-red.svg)

**Sistem monitoring kehadiran otomatis untuk siswa dan guru menggunakan teknologi IoT**

[Fitur](#-fitur) • [Arsitektur](#-arsitektur-sistem) • [Instalasi](#-instalasi) • [Dokumentasi](#-dokumentasi-api) • [Kontribusi](#-kontribusi)

</div>

---

## 📖 Deskripsi

Monitoring Class adalah sistem monitoring keberadaan guru dan siswa di kelas berbasis IoT yang memantau kehadiran secara real-time pada setiap jam pelajaran menggunakan Bluetooth Tag, ESP32, dan LoRa. 

Sistem bekerja dengan cara:
- **Scanning Berkelanjutan**: Alat ESP32 melakukan scanning Bluetooth Tag secara berkala untuk mendeteksi keberadaan guru dan siswa di kelas
- **Transmisi Real-time**: Data hasil scanning dikirimkan ke server melalui LoRa Gateway dan API Laravel
- **Notifikasi Terjadwal**: Notifikasi WhatsApp otomatis dikirim ke orang tua siswa dan Wakil Kepala Kurikulum setiap pergantian jam pelajaran
- **Monitoring Per Jam Pelajaran**: Sistem memantau dan mencatat keberadaan di kelas untuk setiap sesi jam pelajaran sesuai jadwal

## ✨ Fitur

### 🎯 Fitur Utama
- **Scanning Berkelanjutan**: ESP32 melakukan scanning Bluetooth Tag secara berkala (setiap 30 detik)
- **Monitoring Per Jam Pelajaran**: Memantau keberadaan guru dan siswa untuk setiap sesi jam pelajaran
- **Komunikasi Jarak Jauh**: Implementasi LoRa untuk transmisi data jarak jauh dari ruang kelas ke server
- **Real-time Presence Tracking**: Dashboard web untuk memantau keberadaan di kelas secara langsung
- **Notifikasi Terjadwal**: Pengiriman notifikasi WhatsApp otomatis setiap jam pelajaran
- **Laporan Per Sesi**: Sistem pelaporan keberadaan berdasarkan jam pelajaran dengan berbagai filter dan export

### 📊 Fitur Dashboard
- **Filament Admin Panel**: Interface modern dan responsif untuk manajemen sistem
- Dashboard statistik keberadaan real-time per jam pelajaran dengan widgets
- Monitoring keberadaan guru dan siswa saat ini di kelas
- **Filament Resources**: Manajemen data siswa, guru, dan Bluetooth Tag dengan CRUD lengkap
- Manajemen kelas dan jadwal jam pelajaran dengan relasi data
- **Filament Tables**: Rekap keberadaan per jam, harian, mingguan, dan bulanan dengan filter & sorting
- **Export Actions**: Export laporan ke PDF dan Excel langsung dari tabel
- Grafik dan visualisasi data keberadaan per sesi menggunakan Chart Widgets
- **Custom Pages**: Halaman monitoring real-time dengan auto-refresh

### 🔔 Sistem Notifikasi
- **Notifikasi Per Jam Pelajaran**: Otomatis dikirim setiap pergantian jam pelajaran
- **Notifikasi ke Orang Tua**: Informasi keberadaan siswa di kelas per jam pelajaran
- **Notifikasi ke Wakil Kurikulum**: Laporan keberadaan guru dan siswa per jam pelajaran
- **Alert Real-time**: Peringatan jika guru/siswa tidak terdeteksi di kelas saat jam pelajaran
- **Ringkasan Harian**: Laporan keberadaan harian otomatis di akhir hari
- **Customizable Template**: Template notifikasi WhatsApp dapat disesuaikan

## 🏗 Arsitektur Sistem

```
┌─────────────────┐
│  Bluetooth Tag  │
│  (Siswa/Guru)   │
└────────┬────────┘
         │ BLE Broadcasting
         ▼
┌─────────────────┐      ┌──────────────────────┐
│     ESP32       │      │   Scanning Berkala   │
│  (BLE Scanner)  │◄─────│  (Setiap 30 detik)   │
│   di Ruang      │      └──────────────────────┘
│     Kelas       │
└────────┬────────┘
         │ LoRa (915MHz)
         │ Data: Tag MAC + RSSI + Timestamp
         ▼
┌─────────────────┐
│  LoRa Gateway   │
│    + ESP32      │
└────────┬────────┘
         │ HTTP POST API
         │ Real-time Data Stream
         ▼
┌─────────────────┐      ┌──────────────────────┐
│  Laravel API    │◄─────│   Cron Job Notif     │
│  (Backend Web)  │      │  (Setiap Jam Pel.)   │
└────────┬────────┘      └──────────────────────┘
         │
         ├──────────────┬─────────────────┐
         ▼              ▼                 ▼
┌─────────────┐  ┌──────────────┐  ┌──────────────┐
│  Database   │  │  WhatsApp    │  │  Dashboard   │
│   MySQL     │  │  Gateway API │  │  Real-time   │
│             │  │              │  │  Monitoring  │
│ - Presence  │  │ - Orang Tua  │  └──────────────┘
│   Logs      │  │ - Waka Kur.  │
│ - Schedules │  │              │
└─────────────┘  └──────────────┘
```

## 🛠 Teknologi yang Digunakan

### Hardware
- **ESP32**: Microcontroller untuk BLE scanning dan LoRa communication
- **LoRa Module**: SX1276/RFM95 untuk komunikasi jarak jauh
- **Bluetooth Tag**: iBeacon/BLE tag untuk identifikasi

### Software
- **Backend**: Laravel 12.x (PHP 8.3+)
- **Admin Panel**: Filament 4.x
- **Database**: MySQL 8.0+
- **IoT**: Arduino IDE, ESP32 Libraries
- **API Integration**: WhatsApp Business API / Fonnte

### Libraries & Dependencies
```
ESP32:
- LoRa by Sandeep Mistry
- BLE Arduino Library
- WiFi & HTTPClient

Laravel 12:
- Filament 4.x (Admin Panel)
- Laravel Sanctum (API Authentication)
- Laravel Excel (Export laporan)
- Maatwebsite Excel
- Guzzle HTTP Client
- Livewire 3.x (Filament dependency)
```

## 📋 Persyaratan Sistem

### Hardware Requirements
- ESP32 Development Board (minimum 2 unit)
- LoRa Module SX1276 (2 unit)
- Bluetooth Low Energy Tags/Beacon
- Power supply untuk perangkat IoT

### Software Requirements
- PHP >= 8.3
- Composer
- MySQL >= 8.0
- Node.js >= 20.x (untuk Filament assets)
- Arduino IDE >= 1.8.x

## 🚀 Instalasi

### 1. Clone Repository
```bash
git clone https://github.com/username/monitoring-class.git
cd monitoring-class
```

### 2. Setup Backend (Laravel)

```bash
# Install dependencies
composer install

# Copy environment file
cp .env.example .env

# Generate application key
php artisan key:generate

# Configure database di file .env
DB_CONNECTION=mysql
DB_HOST=127.0.0.1
DB_PORT=3306
DB_DATABASE=monitoring_class
DB_USERNAME=root
DB_PASSWORD=

# Run migrations
php artisan migrate --seed

# Storage link
php artisan storage:link

# Install NPM dependencies (untuk Filament)
npm install
npm run build

# Create admin user untuk Filament
php artisan make:filament-user

# Start server
php artisan serve
```

**Akses Filament Admin Panel:**
- URL: `http://localhost:8000/admin`
- Login dengan kredensial yang dibuat saat `make:filament-user`

### 3. Setup IoT Devices

#### ESP32 Reader (BLE Scanner + LoRa Transmitter)
```cpp
// Upload sketch dari folder /arduino/esp32_reader
// Konfigurasi di config.h:
#define LORA_FREQUENCY 915E6
#define BLE_SCAN_TIME 5          // Durasi scan dalam detik
#define SCAN_INTERVAL 30000      // Interval scan (30 detik)
#define CLASSROOM_ID 1           // ID ruang kelas
#define RSSI_THRESHOLD -70       // Threshold sinyal untuk deteksi presence
```

#### ESP32 Gateway (LoRa Receiver + API Sender)
```cpp
// Upload sketch dari folder /arduino/esp32_gateway
// Konfigurasi WiFi dan API endpoint:
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* apiEndpoint = "http://yourserver.com/api/attendance";
```

### 4. Konfigurasi WhatsApp Gateway

Edit file `.env` dan tambahkan:
```env
WHATSAPP_API_URL=https://api.fonnte.com/send
WHATSAPP_API_TOKEN=your_api_token
```

## 📱 Penggunaan

### 1. Registrasi Bluetooth Tag
- Login ke Filament Admin Panel (`/admin`)
- Masuk ke menu "Manajemen Tag" (Bluetooth Tags Resource)
- Klik "Create" untuk tambahkan MAC Address Bluetooth Tag
- Assign tag ke siswa/guru yang bersangkutan menggunakan relasi field

### 2. Setup Jadwal Pelajaran
- Buat data kelas di menu "Manajemen Kelas" (Classes Resource)
- Tambahkan jadwal jam pelajaran di "Schedules Resource" (mata pelajaran, guru, waktu mulai, waktu selesai)
- Atur interval notifikasi per jam pelajaran
- Konfigurasi template notifikasi WhatsApp di "Notification Templates"

### 3. Instalasi Perangkat di Ruang Kelas
- Pasang ESP32 Reader di setiap ruang kelas
- Posisikan di lokasi strategis untuk coverage optimal
- Test scanning range Bluetooth
- Verifikasi koneksi LoRa ke Gateway

### 4. Monitoring Real-time
- Akses halaman "Real-time Monitoring" di Filament Admin Panel
- Dashboard menampilkan keberadaan real-time per jam pelajaran dengan auto-refresh
- ESP32 melakukan scanning berkelanjutan setiap 30 detik
- Data keberadaan dikirim ke server secara otomatis
- Status ditampilkan dengan badge: "Di Kelas" (success), "Tidak Terdeteksi" (danger)
- Gunakan filter untuk melihat per kelas atau per jadwal tertentu

### 5. Sistem Notifikasi Otomatis
- **Setiap Jam Pelajaran**: Notifikasi otomatis terkirim ke orang tua dan Wakil Kurikulum
- **Isi Notifikasi untuk Orang Tua**: Status keberadaan siswa di kelas, mata pelajaran, nama guru
- **Isi Notifikasi untuk Wakil Kurikulum**: Ringkasan keberadaan guru dan siswa per kelas
- **Alert**: Pemberitahuan khusus jika guru/siswa tidak terdeteksi
- Monitor status pengiriman di "Notifications Resource"

### 6. Generate Laporan
- Buka "Presence Reports" di Filament Admin Panel
- Pilih periode (per jam pelajaran, harian, mingguan, bulanan) menggunakan date filter
- Gunakan Filament Table Filters untuk filter berdasarkan kelas, mata pelajaran, atau individu
- Klik tombol "Export" di header tabel untuk export ke format PDF atau Excel
- Kirim laporan via email/WhatsApp menggunakan bulk action

## 📡 Dokumentasi API

### Endpoint Keberadaan (Presence)

#### POST /api/presence/scan
Mengirim data hasil scanning dari IoT device (dikirim setiap 30 detik)

**Request Body:**
```json
{
  "classroom_id": 1,
  "reader_id": "ESP32_CLASS_A",
  "timestamp": "2025-10-09 08:15:30",
  "detected_tags": [
    {
      "tag_mac": "AA:BB:CC:DD:EE:FF",
      "rssi": -65,
      "distance": 2.5
    },
    {
      "tag_mac": "11:22:33:44:55:66",
      "rssi": -58,
      "distance": 1.8
    }
  ]
}
```

**Response:**
```json
{
  "success": true,
  "message": "Scan data processed successfully",
  "data": {
    "classroom": "X-A",
    "schedule": {
      "subject": "Matematika",
      "teacher": "Pak Budi",
      "time": "08:00 - 09:30"
    },
    "present": {
      "students": 28,
      "teacher": true
    },
    "absent": {
      "students": 2
    }
  }
}
```

#### GET /api/presence/current/{classroom_id}
Mendapatkan data keberadaan real-time di kelas pada jam pelajaran saat ini

**Headers:**
```
Authorization: Bearer {token}
```

**Response:**
```json
{
  "success": true,
  "data": {
    "classroom": "X-A",
    "current_schedule": {
      "subject": "Matematika",
      "teacher": "Pak Budi Santoso",
      "time": "08:00 - 09:30",
      "session": 1
    },
    "teacher_presence": {
      "name": "Pak Budi Santoso",
      "status": "present",
      "last_detected": "08:15:30",
      "duration": "15 minutes"
    },
    "students_presence": [
      {
        "name": "Ahmad Fadli",
        "nis": "2024001",
        "status": "present",
        "last_detected": "08:15:30"
      },
      {
        "name": "Siti Nurhaliza",
        "nis": "2024002",
        "status": "not_detected",
        "last_detected": null
      }
    ],
    "summary": {
      "total_students": 30,
      "present": 28,
      "not_detected": 2
    }
  }
}
```

#### POST /api/notifications/send-schedule
Trigger manual pengiriman notifikasi per jam pelajaran (biasanya otomatis via cron)

**Request Body:**
```json
{
  "schedule_id": 15,
  "notification_type": "lesson_start"
}
```

**Response:**
```json
{
  "success": true,
  "message": "Notifications sent successfully",
  "sent_count": {
    "parents": 28,
    "curriculum_head": 1
  }
}
```

## 🔐 Keamanan

- API menggunakan Laravel Sanctum untuk autentikasi
- HTTPS wajib untuk production environment
- Rate limiting pada API endpoints
- Validasi data pada setiap request
- Enkripsi komunikasi LoRa (optional)

## 🧪 Testing

```bash
# Run unit tests
php artisan test

# Run specific test
php artisan test --filter AttendanceTest

# Check code coverage
php artisan test --coverage
```

## 📊 Database Schema

### Tabel Utama

```sql
users (id, name, email, role, phone)
students (id, user_id, nis, class_id, tag_mac, parent_phone)
teachers (id, user_id, nip, tag_mac, subjects)
classes (id, name, grade, major, room_number, reader_id)
schedules (id, class_id, teacher_id, subject, day, start_time, end_time, session_number)
presence_logs (id, tag_mac, classroom_id, schedule_id, timestamp, rssi, status)
presence_summary (id, schedule_id, student_id, teacher_id, status, first_detected, last_detected, duration)
scan_logs (id, reader_id, classroom_id, timestamp, detected_count, raw_data)
notifications (id, recipient_phone, recipient_role, schedule_id, message, sent_at, status, type)
notification_templates (id, type, role, template_text, variables)
```

## 🤝 Kontribusi

Kontribusi sangat diterima! Silakan ikuti langkah berikut:

1. Fork repository ini
2. Buat branch fitur (`git checkout -b feature/AmazingFeature`)
3. Commit perubahan (`git commit -m 'Add some AmazingFeature'`)
4. Push ke branch (`git push origin feature/AmazingFeature`)
5. Buat Pull Request

## 📝 Roadmap

- [ ] **Filament Widgets Enhancement**: Custom widgets untuk analytics keberadaan
- [ ] **Multi-tenancy Support**: Filament multi-tenancy untuk multiple sekolah
- [ ] Implementasi Face Recognition sebagai backup
- [ ] Mobile App untuk Android & iOS
- [ ] Integration dengan sistem akademik existing
- [ ] Advanced analytics & AI prediction untuk pola keberadaan
- [ ] RFID card reader support
- [ ] Biometric integration
- [ ] **Filament Actions**: Custom actions untuk bulk operations
- [ ] **Real-time Notifications**: Pusher/Echo integration untuk live updates

## 🐛 Known Issues

- LoRa range terbatas dalam bangunan dengan banyak penghalang (optimal: 200-500m indoor)
- Bluetooth interference pada area dengan banyak device (gunakan RSSI threshold)
- Delay notifikasi WhatsApp tergantung provider API (rata-rata 2-5 detik)
- Tag yang dibawa keluar kelas sesaat masih terdeteksi hingga scan berikutnya (maks 30 detik delay)
- ESP32 perlu reboot periodik untuk stabilitas (recommended: setiap 24 jam)

## 📄 Lisensi

Proyek ini dilisensikan di bawah [MIT License](LICENSE)

## 👥 Tim Pengembang

- **Project Lead**: Yahya Zulfikri
- **Backend Developer**: Yahya Zulfikri
- **IoT Engineer**: Yahya Zulfikri
- **Frontend Developer**: Yahya Zulfikri

## 📞 Kontak & Support

- Email: support@monitoring-class.zedlabs.id
- Website: https://monitoring-class.zedlabs.id
- Documentation: https://monitoring-class.zedlabs.id/docs
- Issues: https://github.com/zulfikriyahya/monitoring-class/issues

## 🙏 Acknowledgments

- Terima kasih kepada komunitas Arduino dan ESP32
- Laravel Indonesia Community
- Contributors dan testers

---

<div align="center">

**⭐ Jika project ini membantu, berikan star di GitHub! ⭐**

Made with ❤️ by ZEDLABS

</div>
