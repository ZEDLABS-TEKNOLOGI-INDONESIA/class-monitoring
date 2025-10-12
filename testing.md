# 📚 API Documentation - Sistem Monitoring Kehadiran

## 🚀 Quick Start

### Installation

```bash
# 1. Clone & Install Dependencies
composer install
npm install

# 2. Environment Setup
cp .env.example .env
php artisan key:generate

# 3. Database Configuration
# Edit .env file:
DB_DATABASE=attendance_system
DB_USERNAME=root
DB_PASSWORD=

# Generate API Key (simpan ini untuk Gateway)
GATEWAY_API_KEY=$(openssl rand -base64 32)
echo "GATEWAY_API_KEY=$GATEWAY_API_KEY" >> .env

# 4. Run Migrations & Seeders
php artisan migrate
php artisan db:seed

# 5. Start Server
php artisan serve
# API available at: http://localhost:8000/api
```

---

## 🔑 Authentication

### Gateway Authentication (Hardware → Server)

Gunakan API Key di header setiap request:

```http
Authorization: Bearer your_super_secret_api_key
```

### Dashboard Authentication (Web/Mobile App)

Gunakan Laravel Sanctum token:

```http
Authorization: Bearer {sanctum_token}
```

---

## 📡 API Endpoints

### 1. Store Attendance (Gateway → Server)

**Endpoint:** `POST /api/gateway/attendance`

**Purpose:** Endpoint utama untuk menerima data kehadiran dari gateway

**Headers:**

```http
Content-Type: application/json
Authorization: Bearer your_api_key_here
```

**Request Body:**

```json
{
    "gateway_id": "GW001",
    "node_id": 1,
    "timestamp": 1697097610,
    "student_count": 35,
    "rssi": -68,
    "snr": 9.2,
    "cycle": 1,
    "mac_addresses": [
        "AA:BB:CC:11:22:33",
        "DD:EE:FF:44:55:66",
        "11:22:33:AA:BB:CC"
    ]
}
```

**Response Success (200):**

```json
{
    "success": true,
    "message": "Attendance data processed",
    "data": {
        "processed": 35,
        "skipped": 0,
        "total": 35,
        "timestamp": "2024-10-12 07:00:10",
        "errors": []
    }
}
```

**Response Error (422):**

```json
{
    "success": false,
    "message": "Validation error",
    "errors": {
        "mac_addresses": ["The mac addresses field is required."]
    }
}
```

**Field Descriptions:**

-   `gateway_id`: ID gateway yang mengirim data (string)
-   `node_id`: ID node (1-30) yang mengirim data (integer)
-   `timestamp`: Unix timestamp saat data di-scan (integer)
-   `student_count`: Jumlah MAC address terdeteksi (integer)
-   `rssi`: Received Signal Strength Indicator (integer, optional)
-   `snr`: Signal-to-Noise Ratio (float, optional)
-   `cycle`: Nomor cycle saat ini (integer, optional)
-   `mac_addresses`: Array MAC address format `XX:XX:XX:XX:XX:XX`

**Business Logic:**

1. Validasi format data
2. Update status node (last_seen, rssi, snr)
3. Untuk setiap MAC address:
    - Cari bluetooth tag yang terdaftar
    - Cari student yang memiliki tag tersebut
    - Validasi jadwal (apakah ada kelas sekarang?)
    - Cek duplikasi (prevent double attendance)
    - Simpan data kehadiran
4. Return summary (berapa berhasil, berapa skip)

---

### 2. Gateway Heartbeat

**Endpoint:** `POST /api/gateway/heartbeat`

**Purpose:** Gateway mengirim status health setiap 1-5 menit

**Request Body:**

```json
{
    "gateway_id": "GW001",
    "uptime": 3600,
    "wifi_connected": true,
    "ntp_synced": true,
    "packets_received": 150,
    "packets_sent": 150,
    "api_success": 145,
    "api_fail": 5
}
```

**Response (200):**

```json
{
    "success": true,
    "message": "Heartbeat received",
    "timestamp": 1697097610
}
```

---

### 3. Get Today's Attendance

**Endpoint:** `GET /api/attendance/today`

**Purpose:** Ambil semua kehadiran hari ini (untuk dashboard)

**Headers:**

```http
Authorization: Bearer {sanctum_token}
```

**Query Parameters:**

-   `page`: Pagination page number (default: 1)
-   `per_page`: Items per page (default: 50)

**Response (200):**

```json
{
    "success": true,
    "data": {
        "current_page": 1,
        "data": [
            {
                "id": 1,
                "student": {
                    "id": 1,
                    "nis": "2024070001",
                    "name": "Siswa 7A-1",
                    "class": {
                        "name": "7A",
                        "grade": "7"
                    }
                },
                "schedule": {
                    "subject": "Matematika",
                    "start_time": "07:00",
                    "end_time": "08:30"
                },
                "date": "2024-10-12",
                "time": "07:05:23",
                "status": "present",
                "rssi": -68
            }
        ],
        "total": 1050
    },
    "statistics": {
        "total_present": 1050,
        "total_students": 1200,
        "attendance_rate": 87.5
    }
}
```

---

### 4. Get Attendance by Date

**Endpoint:** `GET /api/attendance/date/{date}`

**Purpose:** Ambil kehadiran berdasarkan tanggal tertentu

**Example:** `GET /api/attendance/date/2024-10-12`

**Response:** Same as today's attendance

---

### 5. Get Attendance by Student

**Endpoint:** `GET /api/attendance/student/{studentId}`

**Purpose:** Riwayat kehadiran satu siswa

**Query Parameters:**

-   `start_date`: Start date (default: awal bulan ini)
-   `end_date`: End date (default: akhir bulan ini)

**Example:** `GET /api/attendance/student/1?start_date=2024-10-01&end_date=2024-10-31`

**Response (200):**

```json
{
    "success": true,
    "data": {
        "student": {
            "id": 1,
            "nis": "2024070001",
            "name": "Siswa 7A-1",
            "class": {
                "name": "7A"
            }
        },
        "attendances": [
            {
                "date": "2024-10-12",
                "time": "07:05:23",
                "subject": "Matematika",
                "status": "present"
            }
        ],
        "total_days": 20
    }
}
```

---

### 6. Get Attendance by Class

**Endpoint:** `GET /api/attendance/class/{classId}`

**Purpose:** Kehadiran satu kelas pada tanggal tertentu

**Query Parameters:**

-   `date`: Target date (default: today)

**Example:** `GET /api/attendance/class/1?date=2024-10-12`

**Response (200):**

```json
{
    "success": true,
    "data": {
        "attendances": [
            {
                "student": {
                    "name": "Siswa 7A-1"
                },
                "time": "07:05:23",
                "status": "present"
            }
        ],
        "total_present": 35,
        "total_students": 40,
        "attendance_rate": 87.5
    }
}
```

---

### 7. Dashboard Statistics

**Endpoint:** `GET /api/statistics/dashboard`

**Purpose:** Statistik untuk dashboard admin

**Response (200):**

```json
{
    "success": true,
    "data": {
        "today": {
            "total_present": 1050,
            "total_students": 1200,
            "total_classes": 30,
            "nodes_online": 28,
            "attendance_rate": 87.5
        },
        "this_week": {
            "total_present": 5250,
            "average_attendance_rate": 85.2
        },
        "nodes_status": [
            {
                "status": "online",
                "count": 28
            },
            {
                "status": "offline",
                "count": 2
            }
        ],
        "recent_activities": [
            {
                "student": {
                    "name": "Siswa 7A-1"
                },
                "class": {
                    "name": "7A"
                },
                "time": "07:05:23"
            }
        ]
    }
}
```

---

### 8. Get All Nodes

**Endpoint:** `GET /api/nodes`

**Purpose:** List semua node dan statusnya

**Response (200):**

```json
{
    "success": true,
    "data": [
        {
            "id": 1,
            "node_id": 1,
            "gateway_id": "GW001",
            "status": "online",
            "last_seen": "2024-10-12 07:05:00",
            "rssi": -68,
            "snr": 9.2,
            "battery": 85,
            "cycles_completed": 50,
            "health_score": 90
        }
    ]
}
```

---

### 9. Get Single Node

**Endpoint:** `GET /api/nodes/{nodeId}`

**Example:** `GET /api/nodes/1`

**Response (200):**

```json
{
    "success": true,
    "data": {
        "id": 1,
        "node_id": 1,
        "status": "online",
        "last_seen": "2024-10-12 07:05:00",
        "rssi": -68,
        "battery": 85,
        "health_score": 90
    }
}
```

---

### 10. Node Health Check

**Endpoint:** `GET /api/node/{nodeId}/health`

**Purpose:** Cek kesehatan node secara detail

**Example:** `GET /api/node/1/health`

**Response (200):**

```json
{
    "success": true,
    "data": {
        "node_id": 1,
        "status": "online",
        "is_online": true,
        "last_seen": "2024-10-12 07:05:00",
        "last_seen_human": "2 minutes ago",
        "rssi": -68,
        "snr": 9.2,
        "battery": 85,
        "health_score": 90
    }
}
```

**Health Score Breakdown:**

-   Online status: 40 points
-   RSSI quality: 30 points (>-70dBm = 30, >-85dBm = 20, >-100dBm = 10)
-   Battery level: 30 points (>50% = 30, >20% = 20, >10% = 10)
-   **Total: 0-100 points**

---

### 11. Node Statistics

**Endpoint:** `GET /api/statistics/nodes`

**Purpose:** Statistik keseluruhan semua node

**Response (200):**

```json
{
  "success": true,
  "data": {
    "total_nodes": 30,
    "online_nodes": 28,
    "offline_nodes": 2,
    "average_rssi": -72.5,
    "nodes_with_low_battery": 3,
    "nodes_detail": [...]
  }
}
```

---

### 12. Attendance Report

**Endpoint:** `GET /api/attendance/report`

**Purpose:** Generate laporan kehadiran dengan filter

**Query Parameters (Required):**

-   `start_date`: Start date (YYYY-MM-DD)
-   `end_date`: End date (YYYY-MM-DD)

**Query Parameters (Optional):**

-   `class_id`: Filter by class
-   `student_id`: Filter by student

**Example:**

```
GET /api/attendance/report?start_date=2024-10-01&end_date=2024-10-31&class_id=1
```

**Response (200):**

```json
{
  "success": true,
  "data": {
    "attendances": [...],
    "summary": {
      "total_records": 850,
      "unique_students": 40,
      "date_range": {
        "start": "2024-10-01",
        "end": "2024-10-31"
      },
      "daily_stats": {
        "2024-10-01": {
          "count": 35,
          "unique_students": 35
        },
        "2024-10-02": {
          "count": 38,
          "unique_students": 38
        }
      }
    }
  }
}
```

---

## 🧪 Testing with cURL

### Test 1: Store Attendance

```bash
curl -X POST http://localhost:8000/api/gateway/attendance \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer your_api_key_here" \
  -d '{
    "gateway_id": "GW001",
    "node_id": 1,
    "timestamp": 1697097610,
    "snr": 9.2,
    "student_count": 1040,
    "rssi": -68,
    "cycle": 1,
    "mac_addresses": [
        "AA:BB:CC:00:00:63",
        "AA:BB:CC:00:00:64",
        "AA:BB:CC:00:00:65",
        "AA:BB:CC:00:00:66",
        "AA:BB:CC:00:00:67",
        "AA:BB:CC:00:00:68",
        "AA:BB:CC:00:00:69",
        "AA:BB:CC:00:00:6A",
        "AA:BB:CC:00:00:6B",
        "AA:BB:CC:00:00:6C",
        "AA:BB:CC:00:00:6D",
        "AA:BB:CC:00:00:6E",
        "AA:BB:CC:00:00:6F",
        "AA:BB:CC:00:00:70",
        "AA:BB:CC:00:00:71",
        "AA:BB:CC:00:00:72",
        "AA:BB:CC:00:00:73",
        "AA:BB:CC:00:00:74",
        "AA:BB:CC:00:00:75",
        "AA:BB:CC:00:00:76",
        "AA:BB:CC:00:00:77",
        "AA:BB:CC:00:00:78",
        "AA:BB:CC:00:00:79",
        "AA:BB:CC:00:00:7A",
        "AA:BB:CC:00:00:7B",
        "AA:BB:CC:00:00:7C",
        "AA:BB:CC:00:00:7D",
        "AA:BB:CC:00:00:7E",
        "AA:BB:CC:00:00:7F",
        "AA:BB:CC:00:00:80",
        "AA:BB:CC:00:00:81",
        "AA:BB:CC:00:00:82",
        "AA:BB:CC:00:00:83",
        "AA:BB:CC:00:00:84",
        "AA:BB:CC:00:00:85",
        "AA:BB:CC:00:00:86",
        "AA:BB:CC:00:00:87",
        "AA:BB:CC:00:00:88",
        "AA:BB:CC:00:00:89",
        "AA:BB:CC:00:00:8A",
        "AA:BB:CC:00:00:8B",
        "AA:BB:CC:00:00:8C",
        "AA:BB:CC:00:00:8D",
        "AA:BB:CC:00:00:8E",
        "AA:BB:CC:00:00:8F",
        "AA:BB:CC:00:00:90",
        "AA:BB:CC:00:00:91",
        "AA:BB:CC:00:00:92",
        "AA:BB:CC:00:00:93",
        "AA:BB:CC:00:00:94",
        "AA:BB:CC:00:00:95",
        "AA:BB:CC:00:00:96",
        "AA:BB:CC:00:00:97",
        "AA:BB:CC:00:00:98",
        "AA:BB:CC:00:00:99",
        "AA:BB:CC:00:00:9A",
        "AA:BB:CC:00:00:9B",
        "AA:BB:CC:00:00:9C",
        "AA:BB:CC:00:00:9D",
        "AA:BB:CC:00:00:9E",
        "AA:BB:CC:00:00:9F",
        "AA:BB:CC:00:00:A0",
        "AA:BB:CC:00:00:A1",
        "AA:BB:CC:00:00:A2",
        "AA:BB:CC:00:00:A3",
        "AA:BB:CC:00:00:A4",
        "AA:BB:CC:00:00:A5",
        "AA:BB:CC:00:00:A6",
        "AA:BB:CC:00:00:A7",
        "AA:BB:CC:00:00:A8",
        "AA:BB:CC:00:00:A9",
        "AA:BB:CC:00:00:AA",
        "AA:BB:CC:00:00:AB",
        "AA:BB:CC:00:00:AC",
        "AA:BB:CC:00:00:AD",
        "AA:BB:CC:00:00:AE",
        "AA:BB:CC:00:00:AF",
        "AA:BB:CC:00:00:B0",
        "AA:BB:CC:00:00:B1",
        "AA:BB:CC:00:00:B2",
        "AA:BB:CC:00:00:B3",
        "AA:BB:CC:00:00:B4",
        "AA:BB:CC:00:00:B5",
        "AA:BB:CC:00:00:B6",
        "AA:BB:CC:00:00:B7",
        "AA:BB:CC:00:00:B8",
        "AA:BB:CC:00:00:B9",
        "AA:BB:CC:00:00:BA",
        "AA:BB:CC:00:00:BB",
        "AA:BB:CC:00:00:BC",
        "AA:BB:CC:00:00:BD",
        "AA:BB:CC:00:00:BE",
        "AA:BB:CC:00:00:BF",
        "AA:BB:CC:00:00:C0",
        "AA:BB:CC:00:00:C1",
        "AA:BB:CC:00:00:C2",
        "AA:BB:CC:00:00:C3",
        "AA:BB:CC:00:00:C4",
        "AA:BB:CC:00:00:C5",
        "AA:BB:CC:00:00:C6",
        "AA:BB:CC:00:00:C7",
        "AA:BB:CC:00:00:C8",
        "AA:BB:CC:00:00:C9",
        "AA:BB:CC:00:00:CA",
        "AA:BB:CC:00:00:CB",
        "AA:BB:CC:00:00:CC",
        "AA:BB:CC:00:00:CD",
        "AA:BB:CC:00:00:CE",
        "AA:BB:CC:00:00:CF",
        "AA:BB:CC:00:00:D0",
        "AA:BB:CC:00:00:D1",
        "AA:BB:CC:00:00:D2",
        "AA:BB:CC:00:00:D3",
        "AA:BB:CC:00:00:D4",
        "AA:BB:CC:00:00:D5",
        "AA:BB:CC:00:00:D6",
        "AA:BB:CC:00:00:D7",
        "AA:BB:CC:00:00:D8",
        "AA:BB:CC:00:00:D9",
        "AA:BB:CC:00:00:DA",
        "AA:BB:CC:00:00:DB",
        "AA:BB:CC:00:00:DC",
        "AA:BB:CC:00:00:DD",
        "AA:BB:CC:00:00:DE",
        "AA:BB:CC:00:00:DF",
        "AA:BB:CC:00:00:E0",
        "AA:BB:CC:00:00:E1",
        "AA:BB:CC:00:00:E2",
        "AA:BB:CC:00:00:E3",
        "AA:BB:CC:00:00:E4",
        "AA:BB:CC:00:00:E5",
        "AA:BB:CC:00:00:E6",
        "AA:BB:CC:00:00:E7",
        "AA:BB:CC:00:00:E8",
        "AA:BB:CC:00:00:E9",
        "AA:BB:CC:00:00:EA",
        "AA:BB:CC:00:00:EB",
        "AA:BB:CC:00:00:EC",
        "AA:BB:CC:00:00:ED",
        "AA:BB:CC:00:00:EE",
        "AA:BB:CC:00:00:EF",
        "AA:BB:CC:00:00:F0",
        "AA:BB:CC:00:00:F1",
        "AA:BB:CC:00:00:F2",
        "AA:BB:CC:00:00:F3",
        "AA:BB:CC:00:00:F4",
        "AA:BB:CC:00:00:F5",
        "AA:BB:CC:00:00:F6",
        "AA:BB:CC:00:00:F7",
        "AA:BB:CC:00:00:F8",
        "AA:BB:CC:00:00:F9",
        "AA:BB:CC:00:00:FA",
        "AA:BB:CC:00:00:FB",
        "AA:BB:CC:00:00:FC",
        "AA:BB:CC:00:00:FD",
        "AA:BB:CC:00:00:FE",
        "AA:BB:CC:00:00:FF",
        "AA:BB:CC:00:01:00",
        "AA:BB:CC:00:01:01",
        "AA:BB:CC:00:01:02",
        "AA:BB:CC:00:01:03",
        "AA:BB:CC:00:01:04",
        "AA:BB:CC:00:01:05",
        "AA:BB:CC:00:01:06",
        "AA:BB:CC:00:01:07",
        "AA:BB:CC:00:01:08",
        "AA:BB:CC:00:01:09",
        "AA:BB:CC:00:01:0A",
        "AA:BB:CC:00:01:0B",
        "AA:BB:CC:00:01:0C",
        "AA:BB:CC:00:01:0D",
        "AA:BB:CC:00:01:0E",
        "AA:BB:CC:00:01:0F",
        "AA:BB:CC:00:01:10",
        "AA:BB:CC:00:01:11",
        "AA:BB:CC:00:01:12",
        "AA:BB:CC:00:01:13",
        "AA:BB:CC:00:01:14",
        "AA:BB:CC:00:01:15",
        "AA:BB:CC:00:01:16",
        "AA:BB:CC:00:01:17",
        "AA:BB:CC:00:01:18",
        "AA:BB:CC:00:01:19",
        "AA:BB:CC:00:01:1A",
        "AA:BB:CC:00:01:1B",
        "AA:BB:CC:00:01:1C",
        "AA:BB:CC:00:01:1D",
        "AA:BB:CC:00:01:1E",
        "AA:BB:CC:00:01:1F",
        "AA:BB:CC:00:01:20",
        "AA:BB:CC:00:01:21",
        "AA:BB:CC:00:01:22",
        "AA:BB:CC:00:01:23",
        "AA:BB:CC:00:01:24",
        "AA:BB:CC:00:01:25",
        "AA:BB:CC:00:01:26",
        "AA:BB:CC:00:01:27",
        "AA:BB:CC:00:01:28",
        "AA:BB:CC:00:01:29",
        "AA:BB:CC:00:01:2A",
        "AA:BB:CC:00:01:2B",
        "AA:BB:CC:00:01:2C",
        "AA:BB:CC:00:01:2D",
        "AA:BB:CC:00:01:2E",
        "AA:BB:CC:00:01:2F",
        "AA:BB:CC:00:01:30",
        "AA:BB:CC:00:01:31",
        "AA:BB:CC:00:01:32",
        "AA:BB:CC:00:01:33",
        "AA:BB:CC:00:01:34",
        "AA:BB:CC:00:01:35",
        "AA:BB:CC:00:01:36",
        "AA:BB:CC:00:01:37",
        "AA:BB:CC:00:01:38",
        "AA:BB:CC:00:01:39",
        "AA:BB:CC:00:01:3A",
        "AA:BB:CC:00:01:3B",
        "AA:BB:CC:00:01:3C",
        "AA:BB:CC:00:01:3D",
        "AA:BB:CC:00:01:3E",
        "AA:BB:CC:00:01:3F",
        "AA:BB:CC:00:01:40",
        "AA:BB:CC:00:01:41",
        "AA:BB:CC:00:01:42",
        "AA:BB:CC:00:01:43",
        "AA:BB:CC:00:01:44",
        "AA:BB:CC:00:01:45",
        "AA:BB:CC:00:01:46",
        "AA:BB:CC:00:01:47",
        "AA:BB:CC:00:01:48",
        "AA:BB:CC:00:01:49",
        "AA:BB:CC:00:01:4A",
        "AA:BB:CC:00:01:4B",
        "AA:BB:CC:00:01:4C",
        "AA:BB:CC:00:01:4D",
        "AA:BB:CC:00:01:4E",
        "AA:BB:CC:00:01:4F",
        "AA:BB:CC:00:01:50",
        "AA:BB:CC:00:01:51",
        "AA:BB:CC:00:01:52",
        "AA:BB:CC:00:01:53",
        "AA:BB:CC:00:01:54",
        "AA:BB:CC:00:01:55",
        "AA:BB:CC:00:01:56",
        "AA:BB:CC:00:01:57",
        "AA:BB:CC:00:01:58",
        "AA:BB:CC:00:01:59",
        "AA:BB:CC:00:01:5A",
        "AA:BB:CC:00:01:5B",
        "AA:BB:CC:00:01:5C",
        "AA:BB:CC:00:01:5D",
        "AA:BB:CC:00:01:5E",
        "AA:BB:CC:00:01:5F",
        "AA:BB:CC:00:01:60",
        "AA:BB:CC:00:01:61",
        "AA:BB:CC:00:01:62",
        "AA:BB:CC:00:01:63",
        "AA:BB:CC:00:01:64",
        "AA:BB:CC:00:01:65",
        "AA:BB:CC:00:01:66",
        "AA:BB:CC:00:01:67",
        "AA:BB:CC:00:01:68",
        "AA:BB:CC:00:01:69",
        "AA:BB:CC:00:01:6A",
        "AA:BB:CC:00:01:6B",
        "AA:BB:CC:00:01:6C",
        "AA:BB:CC:00:01:6D",
        "AA:BB:CC:00:01:6E",
        "AA:BB:CC:00:01:6F",
        "AA:BB:CC:00:01:70",
        "AA:BB:CC:00:01:71",
        "AA:BB:CC:00:01:72",
        "AA:BB:CC:00:01:73",
        "AA:BB:CC:00:01:74",
        "AA:BB:CC:00:01:75",
        "AA:BB:CC:00:01:76",
        "AA:BB:CC:00:01:77",
        "AA:BB:CC:00:01:78",
        "AA:BB:CC:00:01:79",
        "AA:BB:CC:00:01:7A",
        "AA:BB:CC:00:01:7B",
        "AA:BB:CC:00:01:7C",
        "AA:BB:CC:00:01:7D",
        "AA:BB:CC:00:01:7E",
        "AA:BB:CC:00:01:7F",
        "AA:BB:CC:00:01:80",
        "AA:BB:CC:00:01:81",
        "AA:BB:CC:00:01:82",
        "AA:BB:CC:00:01:83",
        "AA:BB:CC:00:01:84",
        "AA:BB:CC:00:01:85",
        "AA:BB:CC:00:01:86",
        "AA:BB:CC:00:01:87",
        "AA:BB:CC:00:01:88",
        "AA:BB:CC:00:01:89",
        "AA:BB:CC:00:01:8A",
        "AA:BB:CC:00:01:8B",
        "AA:BB:CC:00:01:8C",
        "AA:BB:CC:00:01:8D",
        "AA:BB:CC:00:01:8E",
        "AA:BB:CC:00:01:8F",
        "AA:BB:CC:00:01:90",
        "AA:BB:CC:00:01:91",
        "AA:BB:CC:00:01:92",
        "AA:BB:CC:00:01:93",
        "AA:BB:CC:00:01:94",
        "AA:BB:CC:00:01:95",
        "AA:BB:CC:00:01:96",
        "AA:BB:CC:00:01:97",
        "AA:BB:CC:00:01:98",
        "AA:BB:CC:00:01:99",
        "AA:BB:CC:00:01:9A",
        "AA:BB:CC:00:01:9B",
        "AA:BB:CC:00:01:9C",
        "AA:BB:CC:00:01:9D",
        "AA:BB:CC:00:01:9E",
        "AA:BB:CC:00:01:9F",
        "AA:BB:CC:00:01:A0",
        "AA:BB:CC:00:01:A1",
        "AA:BB:CC:00:01:A2",
        "AA:BB:CC:00:01:A3",
        "AA:BB:CC:00:01:A4",
        "AA:BB:CC:00:01:A5",
        "AA:BB:CC:00:01:A6",
        "AA:BB:CC:00:01:A7",
        "AA:BB:CC:00:01:A8",
        "AA:BB:CC:00:01:A9",
        "AA:BB:CC:00:01:AA",
        "AA:BB:CC:00:01:AB",
        "AA:BB:CC:00:01:AC",
        "AA:BB:CC:00:01:AD",
        "AA:BB:CC:00:01:AE",
        "AA:BB:CC:00:01:AF",
        "AA:BB:CC:00:01:B0",
        "AA:BB:CC:00:01:B1",
        "AA:BB:CC:00:01:B2",
        "AA:BB:CC:00:01:B3",
        "AA:BB:CC:00:01:B4",
        "AA:BB:CC:00:01:B5",
        "AA:BB:CC:00:01:B6",
        "AA:BB:CC:00:01:B7",
        "AA:BB:CC:00:01:B8",
        "AA:BB:CC:00:01:B9",
        "AA:BB:CC:00:01:BA",
        "AA:BB:CC:00:01:BB",
        "AA:BB:CC:00:01:BC",
        "AA:BB:CC:00:01:BD",
        "AA:BB:CC:00:01:BE",
        "AA:BB:CC:00:01:BF",
        "AA:BB:CC:00:01:C0",
        "AA:BB:CC:00:01:C1",
        "AA:BB:CC:00:01:C2",
        "AA:BB:CC:00:01:C3",
        "AA:BB:CC:00:01:C4",
        "AA:BB:CC:00:01:C5",
        "AA:BB:CC:00:01:C6",
        "AA:BB:CC:00:01:C7",
        "AA:BB:CC:00:01:C8",
        "AA:BB:CC:00:01:C9",
        "AA:BB:CC:00:01:CA",
        "AA:BB:CC:00:01:CB",
        "AA:BB:CC:00:01:CC",
        "AA:BB:CC:00:01:CD",
        "AA:BB:CC:00:01:CE",
        "AA:BB:CC:00:01:CF",
        "AA:BB:CC:00:01:D0",
        "AA:BB:CC:00:01:D1",
        "AA:BB:CC:00:01:D2",
        "AA:BB:CC:00:01:D3",
        "AA:BB:CC:00:01:D4",
        "AA:BB:CC:00:01:D5",
        "AA:BB:CC:00:01:D6",
        "AA:BB:CC:00:01:D7",
        "AA:BB:CC:00:01:D8",
        "AA:BB:CC:00:01:D9",
        "AA:BB:CC:00:01:DA",
        "AA:BB:CC:00:01:DB",
        "AA:BB:CC:00:01:DC",
        "AA:BB:CC:00:01:DD",
        "AA:BB:CC:00:01:DE",
        "AA:BB:CC:00:01:DF",
        "AA:BB:CC:00:01:E0",
        "AA:BB:CC:00:01:E1",
        "AA:BB:CC:00:01:E2",
        "AA:BB:CC:00:01:E3",
        "AA:BB:CC:00:01:E4",
        "AA:BB:CC:00:01:E5",
        "AA:BB:CC:00:01:E6",
        "AA:BB:CC:00:01:E7",
        "AA:BB:CC:00:01:E8",
        "AA:BB:CC:00:01:E9",
        "AA:BB:CC:00:01:EA",
        "AA:BB:CC:00:01:EB",
        "AA:BB:CC:00:01:EC",
        "AA:BB:CC:00:01:ED",
        "AA:BB:CC:00:01:EE",
        "AA:BB:CC:00:01:EF",
        "AA:BB:CC:00:01:F0",
        "AA:BB:CC:00:01:F1",
        "AA:BB:CC:00:01:F2",
        "AA:BB:CC:00:01:F3",
        "AA:BB:CC:00:01:F4",
        "AA:BB:CC:00:01:F5",
        "AA:BB:CC:00:01:F6",
        "AA:BB:CC:00:01:F7",
        "AA:BB:CC:00:01:F8",
        "AA:BB:CC:00:01:F9",
        "AA:BB:CC:00:01:FA",
        "AA:BB:CC:00:01:FB",
        "AA:BB:CC:00:01:FC",
        "AA:BB:CC:00:01:FD",
        "AA:BB:CC:00:01:FE",
        "AA:BB:CC:00:01:FF",
        "AA:BB:CC:00:02:00",
        "AA:BB:CC:00:02:01",
        "AA:BB:CC:00:02:02",
        "AA:BB:CC:00:02:03",
        "AA:BB:CC:00:02:04",
        "AA:BB:CC:00:02:05",
        "AA:BB:CC:00:02:06",
        "AA:BB:CC:00:02:07",
        "AA:BB:CC:00:02:08",
        "AA:BB:CC:00:02:09",
        "AA:BB:CC:00:02:0A",
        "AA:BB:CC:00:02:0B",
        "AA:BB:CC:00:02:0C",
        "AA:BB:CC:00:02:0D",
        "AA:BB:CC:00:02:0E",
        "AA:BB:CC:00:02:0F",
        "AA:BB:CC:00:02:10",
        "AA:BB:CC:00:02:11",
        "AA:BB:CC:00:02:12",
        "AA:BB:CC:00:02:13",
        "AA:BB:CC:00:02:14",
        "AA:BB:CC:00:02:15",
        "AA:BB:CC:00:02:16",
        "AA:BB:CC:00:02:17",
        "AA:BB:CC:00:02:18",
        "AA:BB:CC:00:02:19",
        "AA:BB:CC:00:02:1A",
        "AA:BB:CC:00:02:1B",
        "AA:BB:CC:00:02:1C",
        "AA:BB:CC:00:02:1D",
        "AA:BB:CC:00:02:1E",
        "AA:BB:CC:00:02:1F",
        "AA:BB:CC:00:02:20",
        "AA:BB:CC:00:02:21",
        "AA:BB:CC:00:02:22",
        "AA:BB:CC:00:02:23",
        "AA:BB:CC:00:02:24",
        "AA:BB:CC:00:02:25",
        "AA:BB:CC:00:02:26",
        "AA:BB:CC:00:02:27",
        "AA:BB:CC:00:02:28",
        "AA:BB:CC:00:02:29",
        "AA:BB:CC:00:02:2A",
        "AA:BB:CC:00:02:2B",
        "AA:BB:CC:00:02:2C",
        "AA:BB:CC:00:02:2D",
        "AA:BB:CC:00:02:2E",
        "AA:BB:CC:00:02:2F",
        "AA:BB:CC:00:02:30",
        "AA:BB:CC:00:02:31",
        "AA:BB:CC:00:02:32",
        "AA:BB:CC:00:02:33",
        "AA:BB:CC:00:02:34",
        "AA:BB:CC:00:02:35",
        "AA:BB:CC:00:02:36",
        "AA:BB:CC:00:02:37",
        "AA:BB:CC:00:02:38",
        "AA:BB:CC:00:02:39",
        "AA:BB:CC:00:02:3A",
        "AA:BB:CC:00:02:3B",
        "AA:BB:CC:00:02:3C",
        "AA:BB:CC:00:02:3D",
        "AA:BB:CC:00:02:3E",
        "AA:BB:CC:00:02:3F",
        "AA:BB:CC:00:02:40",
        "AA:BB:CC:00:02:41",
        "AA:BB:CC:00:02:42",
        "AA:BB:CC:00:02:43",
        "AA:BB:CC:00:02:44",
        "AA:BB:CC:00:02:45",
        "AA:BB:CC:00:02:46",
        "AA:BB:CC:00:02:47",
        "AA:BB:CC:00:02:48",
        "AA:BB:CC:00:02:49",
        "AA:BB:CC:00:02:4A",
        "AA:BB:CC:00:02:4B",
        "AA:BB:CC:00:02:4C",
        "AA:BB:CC:00:02:4D",
        "AA:BB:CC:00:02:4E",
        "AA:BB:CC:00:02:4F",
        "AA:BB:CC:00:02:50",
        "AA:BB:CC:00:02:51",
        "AA:BB:CC:00:02:52",
        "AA:BB:CC:00:02:53",
        "AA:BB:CC:00:02:54",
        "AA:BB:CC:00:02:55",
        "AA:BB:CC:00:02:56",
        "AA:BB:CC:00:02:57",
        "AA:BB:CC:00:02:58",
        "AA:BB:CC:00:02:59",
        "AA:BB:CC:00:02:5A",
        "AA:BB:CC:00:02:5B",
        "AA:BB:CC:00:02:5C",
        "AA:BB:CC:00:02:5D",
        "AA:BB:CC:00:02:5E",
        "AA:BB:CC:00:02:5F",
        "AA:BB:CC:00:02:60",
        "AA:BB:CC:00:02:61",
        "AA:BB:CC:00:02:62",
        "AA:BB:CC:00:02:63",
        "AA:BB:CC:00:02:64",
        "AA:BB:CC:00:02:65",
        "AA:BB:CC:00:02:66",
        "AA:BB:CC:00:02:67",
        "AA:BB:CC:00:02:68",
        "AA:BB:CC:00:02:69",
        "AA:BB:CC:00:02:6A",
        "AA:BB:CC:00:02:6B",
        "AA:BB:CC:00:02:6C",
        "AA:BB:CC:00:02:6D",
        "AA:BB:CC:00:02:6E",
        "AA:BB:CC:00:02:6F",
        "AA:BB:CC:00:02:70",
        "AA:BB:CC:00:02:71",
        "AA:BB:CC:00:02:72",
        "AA:BB:CC:00:02:73",
        "AA:BB:CC:00:02:74",
        "AA:BB:CC:00:02:75",
        "AA:BB:CC:00:02:76",
        "AA:BB:CC:00:02:77",
        "AA:BB:CC:00:02:78",
        "AA:BB:CC:00:02:79",
        "AA:BB:CC:00:02:7A",
        "AA:BB:CC:00:02:7B",
        "AA:BB:CC:00:02:7C",
        "AA:BB:CC:00:02:7D",
        "AA:BB:CC:00:02:7E",
        "AA:BB:CC:00:02:7F",
        "AA:BB:CC:00:02:80",
        "AA:BB:CC:00:02:81",
        "AA:BB:CC:00:02:82",
        "AA:BB:CC:00:02:83",
        "AA:BB:CC:00:02:84",
        "AA:BB:CC:00:02:85",
        "AA:BB:CC:00:02:86",
        "AA:BB:CC:00:02:87",
        "AA:BB:CC:00:02:88",
        "AA:BB:CC:00:02:89",
        "AA:BB:CC:00:02:8A",
        "AA:BB:CC:00:02:8B",
        "AA:BB:CC:00:02:8C",
        "AA:BB:CC:00:02:8D",
        "AA:BB:CC:00:02:8E",
        "AA:BB:CC:00:02:8F",
        "AA:BB:CC:00:02:90",
        "AA:BB:CC:00:02:91",
        "AA:BB:CC:00:02:92",
        "AA:BB:CC:00:02:93",
        "AA:BB:CC:00:02:94",
        "AA:BB:CC:00:02:95",
        "AA:BB:CC:00:02:96",
        "AA:BB:CC:00:02:97",
        "AA:BB:CC:00:02:98",
        "AA:BB:CC:00:02:99",
        "AA:BB:CC:00:02:9A",
        "AA:BB:CC:00:02:9B",
        "AA:BB:CC:00:02:9C",
        "AA:BB:CC:00:02:9D",
        "AA:BB:CC:00:02:9E",
        "AA:BB:CC:00:02:9F",
        "AA:BB:CC:00:02:A0",
        "AA:BB:CC:00:02:A1",
        "AA:BB:CC:00:02:A2",
        "AA:BB:CC:00:02:A3",
        "AA:BB:CC:00:02:A4",
        "AA:BB:CC:00:02:A5",
        "AA:BB:CC:00:02:A6",
        "AA:BB:CC:00:02:A7",
        "AA:BB:CC:00:02:A8",
        "AA:BB:CC:00:02:A9",
        "AA:BB:CC:00:02:AA",
        "AA:BB:CC:00:02:AB",
        "AA:BB:CC:00:02:AC",
        "AA:BB:CC:00:02:AD",
        "AA:BB:CC:00:02:AE",
        "AA:BB:CC:00:02:AF",
        "AA:BB:CC:00:02:B0",
        "AA:BB:CC:00:02:B1",
        "AA:BB:CC:00:02:B2",
        "AA:BB:CC:00:02:B3",
        "AA:BB:CC:00:02:B4",
        "AA:BB:CC:00:02:B5",
        "AA:BB:CC:00:02:B6",
        "AA:BB:CC:00:02:B7",
        "AA:BB:CC:00:02:B8",
        "AA:BB:CC:00:02:B9",
        "AA:BB:CC:00:02:BA",
        "AA:BB:CC:00:02:BB",
        "AA:BB:CC:00:02:BC",
        "AA:BB:CC:00:02:BD",
        "AA:BB:CC:00:02:BE",
        "AA:BB:CC:00:02:BF",
        "AA:BB:CC:00:02:C0",
        "AA:BB:CC:00:02:C1",
        "AA:BB:CC:00:02:C2",
        "AA:BB:CC:00:02:C3",
        "AA:BB:CC:00:02:C4",
        "AA:BB:CC:00:02:C5",
        "AA:BB:CC:00:02:C6",
        "AA:BB:CC:00:02:C7",
        "AA:BB:CC:00:02:C8",
        "AA:BB:CC:00:02:C9",
        "AA:BB:CC:00:02:CA",
        "AA:BB:CC:00:02:CB",
        "AA:BB:CC:00:02:CC",
        "AA:BB:CC:00:02:CD",
        "AA:BB:CC:00:02:CE",
        "AA:BB:CC:00:02:CF",
        "AA:BB:CC:00:02:D0",
        "AA:BB:CC:00:02:D1",
        "AA:BB:CC:00:02:D2",
        "AA:BB:CC:00:02:D3",
        "AA:BB:CC:00:02:D4",
        "AA:BB:CC:00:02:D5",
        "AA:BB:CC:00:02:D6",
        "AA:BB:CC:00:02:D7",
        "AA:BB:CC:00:02:D8",
        "AA:BB:CC:00:02:D9",
        "AA:BB:CC:00:02:DA",
        "AA:BB:CC:00:02:DB",
        "AA:BB:CC:00:02:DC",
        "AA:BB:CC:00:02:DD",
        "AA:BB:CC:00:02:DE",
        "AA:BB:CC:00:02:DF",
        "AA:BB:CC:00:02:E0",
        "AA:BB:CC:00:02:E1",
        "AA:BB:CC:00:02:E2",
        "AA:BB:CC:00:02:E3",
        "AA:BB:CC:00:02:E4",
        "AA:BB:CC:00:02:E5",
        "AA:BB:CC:00:02:E6",
        "AA:BB:CC:00:02:E7",
        "AA:BB:CC:00:02:E8",
        "AA:BB:CC:00:02:E9",
        "AA:BB:CC:00:02:EA",
        "AA:BB:CC:00:02:EB",
        "AA:BB:CC:00:02:EC",
        "AA:BB:CC:00:02:ED",
        "AA:BB:CC:00:02:EE",
        "AA:BB:CC:00:02:EF",
        "AA:BB:CC:00:02:F0",
        "AA:BB:CC:00:02:F1",
        "AA:BB:CC:00:02:F2",
        "AA:BB:CC:00:02:F3",
        "AA:BB:CC:00:02:F4",
        "AA:BB:CC:00:02:F5",
        "AA:BB:CC:00:02:F6",
        "AA:BB:CC:00:02:F7",
        "AA:BB:CC:00:02:F8",
        "AA:BB:CC:00:02:F9",
        "AA:BB:CC:00:02:FA",
        "AA:BB:CC:00:02:FB",
        "AA:BB:CC:00:02:FC",
        "AA:BB:CC:00:02:FD",
        "AA:BB:CC:00:02:FE",
        "AA:BB:CC:00:02:FF",
        "AA:BB:CC:00:03:00",
        "AA:BB:CC:00:03:01",
        "AA:BB:CC:00:03:02",
        "AA:BB:CC:00:03:03",
        "AA:BB:CC:00:03:04",
        "AA:BB:CC:00:03:05",
        "AA:BB:CC:00:03:06",
        "AA:BB:CC:00:03:07",
        "AA:BB:CC:00:03:08",
        "AA:BB:CC:00:03:09",
        "AA:BB:CC:00:03:0A",
        "AA:BB:CC:00:03:0B",
        "AA:BB:CC:00:03:0C",
        "AA:BB:CC:00:03:0D",
        "AA:BB:CC:00:03:0E",
        "AA:BB:CC:00:03:0F",
        "AA:BB:CC:00:03:10",
        "AA:BB:CC:00:03:11",
        "AA:BB:CC:00:03:12",
        "AA:BB:CC:00:03:13",
        "AA:BB:CC:00:03:14",
        "AA:BB:CC:00:03:15",
        "AA:BB:CC:00:03:16",
        "AA:BB:CC:00:03:17",
        "AA:BB:CC:00:03:18",
        "AA:BB:CC:00:03:19",
        "AA:BB:CC:00:03:1A",
        "AA:BB:CC:00:03:1B",
        "AA:BB:CC:00:03:1C",
        "AA:BB:CC:00:03:1D",
        "AA:BB:CC:00:03:1E",
        "AA:BB:CC:00:03:1F",
        "AA:BB:CC:00:03:20",
        "AA:BB:CC:00:03:21",
        "AA:BB:CC:00:03:22",
        "AA:BB:CC:00:03:23",
        "AA:BB:CC:00:03:24",
        "AA:BB:CC:00:03:25",
        "AA:BB:CC:00:03:26",
        "AA:BB:CC:00:03:27",
        "AA:BB:CC:00:03:28",
        "AA:BB:CC:00:03:29",
        "AA:BB:CC:00:03:2A",
        "AA:BB:CC:00:03:2B",
        "AA:BB:CC:00:03:2C",
        "AA:BB:CC:00:03:2D",
        "AA:BB:CC:00:03:2E",
        "AA:BB:CC:00:03:2F",
        "AA:BB:CC:00:03:30",
        "AA:BB:CC:00:03:31",
        "AA:BB:CC:00:03:32",
        "AA:BB:CC:00:03:33",
        "AA:BB:CC:00:03:34",
        "AA:BB:CC:00:03:35",
        "AA:BB:CC:00:03:36",
        "AA:BB:CC:00:03:37",
        "AA:BB:CC:00:03:38",
        "AA:BB:CC:00:03:39",
        "AA:BB:CC:00:03:3A",
        "AA:BB:CC:00:03:3B",
        "AA:BB:CC:00:03:3C",
        "AA:BB:CC:00:03:3D",
        "AA:BB:CC:00:03:3E",
        "AA:BB:CC:00:03:3F",
        "AA:BB:CC:00:03:40",
        "AA:BB:CC:00:03:41",
        "AA:BB:CC:00:03:42",
        "AA:BB:CC:00:03:43",
        "AA:BB:CC:00:03:44",
        "AA:BB:CC:00:03:45",
        "AA:BB:CC:00:03:46",
        "AA:BB:CC:00:03:47",
        "AA:BB:CC:00:03:48",
        "AA:BB:CC:00:03:49",
        "AA:BB:CC:00:03:4A",
        "AA:BB:CC:00:03:4B",
        "AA:BB:CC:00:03:4C",
        "AA:BB:CC:00:03:4D",
        "AA:BB:CC:00:03:4E",
        "AA:BB:CC:00:03:4F",
        "AA:BB:CC:00:03:50",
        "AA:BB:CC:00:03:51",
        "AA:BB:CC:00:03:52",
        "AA:BB:CC:00:03:53",
        "AA:BB:CC:00:03:54",
        "AA:BB:CC:00:03:55",
        "AA:BB:CC:00:03:56",
        "AA:BB:CC:00:03:57",
        "AA:BB:CC:00:03:58",
        "AA:BB:CC:00:03:59",
        "AA:BB:CC:00:03:5A",
        "AA:BB:CC:00:03:5B",
        "AA:BB:CC:00:03:5C",
        "AA:BB:CC:00:03:5D",
        "AA:BB:CC:00:03:5E",
        "AA:BB:CC:00:03:5F",
        "AA:BB:CC:00:03:60",
        "AA:BB:CC:00:03:61",
        "AA:BB:CC:00:03:62",
        "AA:BB:CC:00:03:63",
        "AA:BB:CC:00:03:64",
        "AA:BB:CC:00:03:65",
        "AA:BB:CC:00:03:66",
        "AA:BB:CC:00:03:67",
        "AA:BB:CC:00:03:68",
        "AA:BB:CC:00:03:69",
        "AA:BB:CC:00:03:6A",
        "AA:BB:CC:00:03:6B",
        "AA:BB:CC:00:03:6C",
        "AA:BB:CC:00:03:6D",
        "AA:BB:CC:00:03:6E",
        "AA:BB:CC:00:03:6F",
        "AA:BB:CC:00:03:70",
        "AA:BB:CC:00:03:71",
        "AA:BB:CC:00:03:72",
        "AA:BB:CC:00:03:73",
        "AA:BB:CC:00:03:74",
        "AA:BB:CC:00:03:75",
        "AA:BB:CC:00:03:76",
        "AA:BB:CC:00:03:77",
        "AA:BB:CC:00:03:78",
        "AA:BB:CC:00:03:79",
        "AA:BB:CC:00:03:7A",
        "AA:BB:CC:00:03:7B",
        "AA:BB:CC:00:03:7C",
        "AA:BB:CC:00:03:7D",
        "AA:BB:CC:00:03:7E",
        "AA:BB:CC:00:03:7F",
        "AA:BB:CC:00:03:80",
        "AA:BB:CC:00:03:81",
        "AA:BB:CC:00:03:82",
        "AA:BB:CC:00:03:83",
        "AA:BB:CC:00:03:84",
        "AA:BB:CC:00:03:85",
        "AA:BB:CC:00:03:86",
        "AA:BB:CC:00:03:87",
        "AA:BB:CC:00:03:88",
        "AA:BB:CC:00:03:89",
        "AA:BB:CC:00:03:8A",
        "AA:BB:CC:00:03:8B",
        "AA:BB:CC:00:03:8C",
        "AA:BB:CC:00:03:8D",
        "AA:BB:CC:00:03:8E",
        "AA:BB:CC:00:03:8F",
        "AA:BB:CC:00:03:90",
        "AA:BB:CC:00:03:91",
        "AA:BB:CC:00:03:92",
        "AA:BB:CC:00:03:93",
        "AA:BB:CC:00:03:94",
        "AA:BB:CC:00:03:95",
        "AA:BB:CC:00:03:96",
        "AA:BB:CC:00:03:97",
        "AA:BB:CC:00:03:98",
        "AA:BB:CC:00:03:99",
        "AA:BB:CC:00:03:9A",
        "AA:BB:CC:00:03:9B",
        "AA:BB:CC:00:03:9C",
        "AA:BB:CC:00:03:9D",
        "AA:BB:CC:00:03:9E",
        "AA:BB:CC:00:03:9F",
        "AA:BB:CC:00:03:A0",
        "AA:BB:CC:00:03:A1",
        "AA:BB:CC:00:03:A2",
        "AA:BB:CC:00:03:A3",
        "AA:BB:CC:00:03:A4",
        "AA:BB:CC:00:03:A5",
        "AA:BB:CC:00:03:A6",
        "AA:BB:CC:00:03:A7",
        "AA:BB:CC:00:03:A8",
        "AA:BB:CC:00:03:A9",
        "AA:BB:CC:00:03:AA",
        "AA:BB:CC:00:03:AB",
        "AA:BB:CC:00:03:AC",
        "AA:BB:CC:00:03:AD",
        "AA:BB:CC:00:03:AE",
        "AA:BB:CC:00:03:AF",
        "AA:BB:CC:00:03:B0",
        "AA:BB:CC:00:03:B1",
        "AA:BB:CC:00:03:B2",
        "AA:BB:CC:00:03:B3",
        "AA:BB:CC:00:03:B4",
        "AA:BB:CC:00:03:B5",
        "AA:BB:CC:00:03:B6",
        "AA:BB:CC:00:03:B7",
        "AA:BB:CC:00:03:B8",
        "AA:BB:CC:00:03:B9",
        "AA:BB:CC:00:03:BA",
        "AA:BB:CC:00:03:BB",
        "AA:BB:CC:00:03:BC",
        "AA:BB:CC:00:03:BD",
        "AA:BB:CC:00:03:BE",
        "AA:BB:CC:00:03:BF",
        "AA:BB:CC:00:03:C0",
        "AA:BB:CC:00:03:C1",
        "AA:BB:CC:00:03:C2",
        "AA:BB:CC:00:03:C3",
        "AA:BB:CC:00:03:C4",
        "AA:BB:CC:00:03:C5",
        "AA:BB:CC:00:03:C6",
        "AA:BB:CC:00:03:C7",
        "AA:BB:CC:00:03:C8",
        "AA:BB:CC:00:03:C9",
        "AA:BB:CC:00:03:CA",
        "AA:BB:CC:00:03:CB",
        "AA:BB:CC:00:03:CC",
        "AA:BB:CC:00:03:CD",
        "AA:BB:CC:00:03:CE",
        "AA:BB:CC:00:03:CF",
        "AA:BB:CC:00:03:D0",
        "AA:BB:CC:00:03:D1",
        "AA:BB:CC:00:03:D2",
        "AA:BB:CC:00:03:D3",
        "AA:BB:CC:00:03:D4",
        "AA:BB:CC:00:03:D5",
        "AA:BB:CC:00:03:D6",
        "AA:BB:CC:00:03:D7",
        "AA:BB:CC:00:03:D8",
        "AA:BB:CC:00:03:D9",
        "AA:BB:CC:00:03:DA",
        "AA:BB:CC:00:03:DB",
        "AA:BB:CC:00:03:DC",
        "AA:BB:CC:00:03:DD",
        "AA:BB:CC:00:03:DE",
        "AA:BB:CC:00:03:DF",
        "AA:BB:CC:00:03:E0",
        "AA:BB:CC:00:03:E1",
        "AA:BB:CC:00:03:E2",
        "AA:BB:CC:00:03:E3",
        "AA:BB:CC:00:03:E4",
        "AA:BB:CC:00:03:E5",
        "AA:BB:CC:00:03:E6",
        "AA:BB:CC:00:03:E7",
        "AA:BB:CC:00:03:E8",
        "AA:BB:CC:00:03:E9",
        "AA:BB:CC:00:03:EA",
        "AA:BB:CC:00:03:EB",
        "AA:BB:CC:00:03:EC",
        "AA:BB:CC:00:03:ED",
        "AA:BB:CC:00:03:EE",
        "AA:BB:CC:00:03:EF",
        "AA:BB:CC:00:03:F0",
        "AA:BB:CC:00:03:F1",
        "AA:BB:CC:00:03:F2",
        "AA:BB:CC:00:03:F3",
        "AA:BB:CC:00:03:F4",
        "AA:BB:CC:00:03:F5",
        "AA:BB:CC:00:03:F6",
        "AA:BB:CC:00:03:F7",
        "AA:BB:CC:00:03:F8",
        "AA:BB:CC:00:03:F9",
        "AA:BB:CC:00:03:FA",
        "AA:BB:CC:00:03:FB",
        "AA:BB:CC:00:03:FC",
        "AA:BB:CC:00:03:FD",
        "AA:BB:CC:00:03:FE",
        "AA:BB:CC:00:03:FF",
        "AA:BB:CC:00:04:00",
        "AA:BB:CC:00:04:01",
        "AA:BB:CC:00:04:02",
        "AA:BB:CC:00:04:03",
        "AA:BB:CC:00:04:04",
        "AA:BB:CC:00:04:05",
        "AA:BB:CC:00:04:06",
        "AA:BB:CC:00:04:07",
        "AA:BB:CC:00:04:08",
        "AA:BB:CC:00:04:09",
        "AA:BB:CC:00:04:0A",
        "AA:BB:CC:00:04:0B",
        "AA:BB:CC:00:04:0C",
        "AA:BB:CC:00:04:0D",
        "AA:BB:CC:00:04:0E",
        "AA:BB:CC:00:04:0F",
        "AA:BB:CC:00:04:10",
        "AA:BB:CC:00:04:11",
        "AA:BB:CC:00:04:12",
        "AA:BB:CC:00:04:13",
        "AA:BB:CC:00:04:14",
        "AA:BB:CC:00:04:15",
        "AA:BB:CC:00:04:16",
        "AA:BB:CC:00:04:17",
        "AA:BB:CC:00:04:18",
        "AA:BB:CC:00:04:19",
        "AA:BB:CC:00:04:1A",
        "AA:BB:CC:00:04:1B",
        "AA:BB:CC:00:04:1C",
        "AA:BB:CC:00:04:1D",
        "AA:BB:CC:00:04:1E",
        "AA:BB:CC:00:04:1F",
        "AA:BB:CC:00:04:20",
        "AA:BB:CC:00:04:21",
        "AA:BB:CC:00:04:22",
        "AA:BB:CC:00:04:23",
        "AA:BB:CC:00:04:24",
        "AA:BB:CC:00:04:25",
        "AA:BB:CC:00:04:26",
        "AA:BB:CC:00:04:27",
        "AA:BB:CC:00:04:28",
        "AA:BB:CC:00:04:29",
        "AA:BB:CC:00:04:2A",
        "AA:BB:CC:00:04:2B",
        "AA:BB:CC:00:04:2C",
        "AA:BB:CC:00:04:2D",
        "AA:BB:CC:00:04:2E",
        "AA:BB:CC:00:04:2F",
        "AA:BB:CC:00:04:30",
        "AA:BB:CC:00:04:31",
        "AA:BB:CC:00:04:32",
        "AA:BB:CC:00:04:33",
        "AA:BB:CC:00:04:34",
        "AA:BB:CC:00:04:35",
        "AA:BB:CC:00:04:36",
        "AA:BB:CC:00:04:37",
        "AA:BB:CC:00:04:38",
        "AA:BB:CC:00:04:39",
        "AA:BB:CC:00:04:3A",
        "AA:BB:CC:00:04:3B",
        "AA:BB:CC:00:04:3C",
        "AA:BB:CC:00:04:3D",
        "AA:BB:CC:00:04:3E",
        "AA:BB:CC:00:04:3F",
        "AA:BB:CC:00:04:40",
        "AA:BB:CC:00:04:41",
        "AA:BB:CC:00:04:42",
        "AA:BB:CC:00:04:43",
        "AA:BB:CC:00:04:44",
        "AA:BB:CC:00:04:45",
        "AA:BB:CC:00:04:46",
        "AA:BB:CC:00:04:47",
        "AA:BB:CC:00:04:48",
        "AA:BB:CC:00:04:49",
        "AA:BB:CC:00:04:4A",
        "AA:BB:CC:00:04:4B",
        "AA:BB:CC:00:04:4C",
        "AA:BB:CC:00:04:4D",
        "AA:BB:CC:00:04:4E",
        "AA:BB:CC:00:04:4F",
        "AA:BB:CC:00:04:50",
        "AA:BB:CC:00:04:51",
        "AA:BB:CC:00:04:52",
        "AA:BB:CC:00:04:53",
        "AA:BB:CC:00:04:54",
        "AA:BB:CC:00:04:55",
        "AA:BB:CC:00:04:56",
        "AA:BB:CC:00:04:57",
        "AA:BB:CC:00:04:58",
        "AA:BB:CC:00:04:59",
        "AA:BB:CC:00:04:5A",
        "AA:BB:CC:00:04:5B",
        "AA:BB:CC:00:04:5C",
        "AA:BB:CC:00:04:5D",
        "AA:BB:CC:00:04:5E",
        "AA:BB:CC:00:04:5F",
        "AA:BB:CC:00:04:60",
        "AA:BB:CC:00:04:61",
        "AA:BB:CC:00:04:62",
        "AA:BB:CC:00:04:63",
        "AA:BB:CC:00:04:64",
        "AA:BB:CC:00:04:65",
        "AA:BB:CC:00:04:66",
        "AA:BB:CC:00:04:67",
        "AA:BB:CC:00:04:68",
        "AA:BB:CC:00:04:69",
        "AA:BB:CC:00:04:6A",
        "AA:BB:CC:00:04:6B",
        "AA:BB:CC:00:04:6C",
        "AA:BB:CC:00:04:6D",
        "AA:BB:CC:00:04:6E",
        "AA:BB:CC:00:04:6F",
        "AA:BB:CC:00:04:70",
        "AA:BB:CC:00:04:71",
        "AA:BB:CC:00:04:72"
    ]
  }'
```

### Test 2: Gateway Heartbeat

```bash
curl -X POST http://localhost:8000/api/gateway/heartbeat \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer your_api_key_here" \
  -d '{
    "gateway_id": "GW001",
    "uptime": 3600,
    "wifi_connected": true,
    "ntp_synced": true,
    "packets_received": 150,
    "packets_sent": 150,
    "api_success": 145,
    "api_fail": 5
  }'
```

### Test 3: Get Dashboard Stats

```bash
curl -X GET http://localhost:8000/api/statistics/dashboard \
  -H "Authorization: Bearer {your_sanctum_token}"
```

---

## 🔧 Gateway Code Update

Update kode Gateway ESP32 untuk menggunakan API key yang benar:

```cpp
// Di gateway code, ganti ini:
const char* API_ENDPOINT = "http://your-server.com/api/gateway/attendance";
const char* API_KEY = "paste_api_key_dari_env_file_disini";
```

---

## 📊 Database Schema Summary

### Tables:

1. **students** - Data siswa
2. **bluetooth_tags** - MAC address BLE tag
3. **classes** - Data kelas (30 kelas)
4. **schedules** - Jadwal pelajaran
5. **attendances** - Record kehadiran (main table)
6. **nodes** - Status 30 node
7. **gateways** - Status gateway

### Key Relationships:

-   Student → has one BluetoothTag
-   Student → has many Attendances
-   Class → has many Students
-   Class → has many Schedules
-   Schedule → has many Attendances
-   Node → linked to Class (via node_id)

---

## 🚨 Error Codes

| Code | Meaning          | Solution                            |
| ---- | ---------------- | ----------------------------------- |
| 401  | Unauthorized     | Cek API key di header               |
| 422  | Validation Error | Cek format request body             |
| 404  | Not Found        | Resource tidak ditemukan            |
| 500  | Server Error     | Cek log: `storage/logs/laravel.log` |

---

## 📝 Logging

Semua activity tercatat di:

```
storage/logs/laravel.log
```

Format log:

```
[2024-10-12 07:00:10] local.INFO: Attendance data received
{"gateway_id":"GW001","node_id":1,"student_count":35}

[2024-10-12 07:00:11] local.INFO: Attendance recorded
{"student":"Siswa 7A-1","class":"7A","time":"07:00:11"}
```

---

## 🔒 Security Best Practices

1. **API Key Management:**

    - Generate strong API key (min 32 characters)
    - Jangan commit API key ke Git
    - Rotate API key setiap 3-6 bulan

2. **Database:**

    - Gunakan prepared statements (sudah by default di Laravel)
    - Backup database setiap hari
    - Enable slow query log

3. **Rate Limiting:**

    - Laravel default: 60 requests/minute per IP
    - Untuk gateway: sesuaikan di `routes/api.php`

4. **HTTPS:**
    - **WAJIB** pakai HTTPS di production
    - Setup SSL certificate (Let's Encrypt gratis)

---

## 🎯 Performance Optimization

### 1. Database Indexing

Sudah diset di migration:

-   Index pada `mac_address`
-   Index pada `date + student_id`
-   Index pada `date + class_id`
-   Composite index untuk prevent duplicate

### 2. Caching (Optional)

```php
// Cache jadwal untuk reduce DB query
$schedule = Cache::remember("schedule_{$classId}_{$date}", 3600, function() {
    return Schedule::where(...)->get();
});
```

### 3. Queue Jobs (Optional)

Untuk processing berat, gunakan queue:

```bash
php artisan queue:table
php artisan migrate
php artisan queue:work
```

---

## 📱 Mobile App Integration

Gunakan endpoints yang sama dengan prefix `/api/`:

**Example (Flutter/React Native):**

```javascript
const API_BASE = "https://your-server.com/api";
const SANCTUM_TOKEN = "user_token_here";

// Get today's attendance
fetch(`${API_BASE}/attendance/today`, {
    headers: {
        Authorization: `Bearer ${SANCTUM_TOKEN}`,
        Accept: "application/json",
    },
})
    .then((res) => res.json())
    .then((data) => console.log(data));
```

---

## ✅ Deployment Checklist

-   [ ] Set `APP_ENV=production` di `.env`
-   [ ] Set `APP_DEBUG=false` di `.env`
-   [ ] Generate strong `APP_KEY`
-   [ ] Generate strong `GATEWAY_API_KEY`
-   [ ] Setup database (MySQL/PostgreSQL recommended)
-   [ ] Run `php artisan migrate --force`
-   [ ] Run `php artisan db:seed --force`
-   [ ] Setup SSL certificate (HTTPS)
-   [ ] Setup cron job untuk Laravel scheduler
-   [ ] Configure firewall (allow only necessary ports)
-   [ ] Setup monitoring (Laravel Telescope/Bugsnag)
-   [ ] Setup automated database backup
-   [ ] Test all endpoints from production domain
-   [ ] Update API_ENDPOINT di Gateway code
-   [ ] Document API for team

---

## 🆘 Troubleshooting

### Problem: "Invalid API key"

**Solution:**

-   Cek API key di `.env` file
-   Pastikan format header: `Authorization: Bearer {api_key}`
-   Jangan ada spasi ekstra

### Problem: "Validation error: mac_addresses"

**Solution:**

-   Format MAC harus: `AA:BB:CC:DD:EE:FF` (uppercase)
-   Harus array, bukan string

### Problem: "No schedule found"

**Solution:**

-   Pastikan ada data di tabel `schedules`
-   Run: `php artisan db:seed --class=ScheduleSeeder`
-   Cek waktu sistem vs waktu schedule

### Problem: Data tidak masuk database

**Solution:**

1. Cek log: `tail -f storage/logs/laravel.log`
2. Cek koneksi database
3. Test manual: `php artisan tinker` → `DB::connection()->getPdo()`

---

## 📞 Support

Jika ada pertanyaan atau issues:

1. Cek log error di `storage/logs/laravel.log`
2. Enable debug mode temporary: `APP_DEBUG=true`
3. Check database dengan: `php artisan tinker`
4. Test API dengan Postman/cURL

---

**Version:** 1.0.0  
**Last Updated:** 2024-10-12  
**Laravel Version:** 12.x
