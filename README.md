# 📚 Monitoring Class - Sistem Monitoring Kehadiran Berbasis IoT

<div align="center">

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![IoT](https://img.shields.io/badge/IoT-Enabled-orange.svg)
![Laravel](https://img.shields.io/badge/Laravel-12.x-red.svg)

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
- **Deteksi Kehadiran Otomatis**: Menggunakan Bluetooth Tag untuk identifikasi siswa dan guru
- **Komunikasi Jarak Jauh**: Implementasi LoRa untuk transmisi data jarak jauh
- **Real-time Monitoring**: Dashboard web untuk memantau kehadiran secara langsung
- **Notifikasi WhatsApp**: Pemberitahuan otomatis ke orang tua dan Wakil Kepala Kurikulum
- **Laporan Komprehensif**: Sistem pelaporan kehadiran dengan berbagai filter dan export

### 📊 Fitur Dashboard
- Dashboard statistik kehadiran
- Manajemen data siswa dan guru
- Manajemen kelas dan jadwal
- Rekap kehadiran harian, mingguan, dan bulanan
- Export laporan ke PDF dan Excel
- Grafik dan visualisasi data kehadiran

### 🔔 Sistem Notifikasi
- Notifikasi real-time saat siswa/guru hadir
- Notifikasi ketidakhadiran
- Laporan kehadiran harian otomatis
- Peringatan untuk tingkat kehadiran rendah

## 🏗 Arsitektur Sistem

```
┌─────────────────┐
│  Bluetooth Tag  │
│  (Siswa/Guru)   │
└────────┬────────┘
         │ Bluetooth
         ▼
┌─────────────────┐
│     ESP32       │
│  (Reader BLE)   │
└────────┬────────┘
         │ LoRa
         ▼
┌─────────────────┐
│  LoRa Gateway   │
│    + ESP32      │
└────────┬────────┘
         │ HTTP/API
         ▼
┌─────────────────┐
│  Laravel API    │
│  (Backend Web)  │
└────────┬────────┘
         │
         ├──────────────┐
         ▼              ▼
┌─────────────┐  ┌──────────────┐
│  Database   │  │  WhatsApp    │
│   MySQL     │  │  Gateway API │
└─────────────┘  └──────────────┘
```

## 🛠 Teknologi yang Digunakan

### Hardware
- **ESP32**: Microcontroller untuk BLE scanning dan LoRa communication
- **LoRa Module**: SX1276/RFM95 untuk komunikasi jarak jauh
- **Bluetooth Tag**: iBeacon/BLE tag untuk identifikasi

### Software
- **Backend**: Laravel 12.x (PHP 8.2+)
- **Frontend**: Blade Template, Bootstrap 5, jQuery
- **Database**: MySQL 8.0+
- **IoT**: Arduino IDE, ESP32 Libraries
- **API Integration**: WhatsApp Business API / Fonnte

### Libraries & Dependencies
```
ESP32:
- LoRa by Sandeep Mistry
- BLE Arduino Library
- WiFi & HTTPClient

Laravel:
- Laravel Sanctum (API Authentication)
- Laravel Excel (Export laporan)
- Maatwebsite Excel
- Guzzle HTTP Client
```

## 📋 Persyaratan Sistem

### Hardware Requirements
- ESP32 Development Board (minimum 2 unit)
- LoRa Module SX1276 (2 unit)
- Bluetooth Low Energy Tags/Beacon
- Power supply untuk perangkat IoT

### Software Requirements
- PHP >= 8.2
- Composer
- MySQL >= 8.0
- Node.js >= 16.x (untuk asset compilation)
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

# Install NPM dependencies
npm install
npm run build

# Start server
php artisan serve
```

### 3. Setup IoT Devices

#### ESP32 Reader (BLE Scanner + LoRa Transmitter)
```cpp
// Upload sketch dari folder /arduino/esp32_reader
// Konfigurasi di config.h:
#define LORA_FREQUENCY 915E6
#define BLE_SCAN_TIME 5
#define GATEWAY_ID 1
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
WHATSAPP_API_URL=https://wapi.zedlabs.id/send-message
WHATSAPP_API_TOKEN=your_api_token
```

## 📱 Penggunaan

### 1. Registrasi Bluetooth Tag
- Login ke dashboard admin
- Masuk ke menu "Manajemen Tag"
- Tambahkan MAC Address Bluetooth Tag
- Assign tag ke siswa/guru

### 2. Setup Ruang Kelas
- Buat data kelas di menu "Manajemen Kelas"
- Tambahkan jadwal pelajaran
- Pasang ESP32 Reader di setiap ruang kelas

### 3. Monitoring Kehadiran
- Akses dashboard untuk melihat kehadiran real-time
- Sistem otomatis mendeteksi saat tag memasuki area kelas
- Notifikasi dikirim otomatis ke WhatsApp

### 4. Generate Laporan
- Pilih periode dan filter yang diinginkan
- Export ke format PDF atau Excel
- Kirim laporan via email/WhatsApp

## 📡 Dokumentasi API

### Endpoint Kehadiran

#### POST /api/attendance
Mengirim data kehadiran dari IoT device

**Request Body:**
```json
{
  "tag_id": "AA:BB:CC:DD:EE:FF",
  "reader_id": "ESP32_001",
  "timestamp": "2025-10-09 08:15:00",
  "signal_strength": -65,
  "class_id": 1
}
```

**Response:**
```json
{
  "success": true,
  "message": "Attendance recorded successfully",
  "data": {
    "id": 123,
    "student_name": "Ahmad Fadli",
    "status": "present",
    "time": "08:15:00"
  }
}
```

#### GET /api/attendance/today
Mendapatkan data kehadiran hari ini

**Headers:**
```
Authorization: Bearer {token}
```

**Response:**
```json
{
  "success": true,
  "data": [
    {
      "id": 1,
      "student_name": "Ahmad Fadli",
      "class": "X-A",
      "status": "present",
      "time": "08:15:00"
    }
  ]
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
students (id, user_id, nis, class_id, tag_mac)
teachers (id, user_id, nip, tag_mac)
classes (id, name, grade, major)
attendances (id, student_id, date, time, status, reader_id)
schedules (id, class_id, teacher_id, subject, day, start_time, end_time)
notifications (id, user_id, type, message, sent_at, status)
```

## 🤝 Kontribusi

Kontribusi sangat diterima! Silakan ikuti langkah berikut:

1. Fork repository ini
2. Buat branch fitur (`git checkout -b feature/AmazingFeature`)
3. Commit perubahan (`git commit -m 'Add some AmazingFeature'`)
4. Push ke branch (`git push origin feature/AmazingFeature`)
5. Buat Pull Request

## 📝 Roadmap

- [ ] Implementasi Face Recognition sebagai backup
- [ ] Mobile App untuk Android & iOS
- [ ] Integration dengan sistem akademik
- [ ] Multi-tenancy support
- [ ] Advanced analytics & AI prediction
- [ ] RFID card reader support
- [ ] Biometric integration

## 🐛 Known Issues

- LoRa range terbatas dalam bangunan dengan banyak penghalang
- Bluetooth interference pada area dengan banyak device
- Delay notifikasi WhatsApp tergantung provider

## 📄 Lisensi

Proyek ini dilisensikan di bawah [MIT License](LICENSE)

## 👥 Tim Pengembang

- **Project Lead**: [Yahya Zulfikri]
- **Backend Developer**: [Yahya Zulfikri]
- **IoT Engineer**: [Yahya Zulfikri]
- **Frontend Developer**: [Yahya Zulfikri]

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

Made with ❤️ by [ZEDLABS]

</div>
