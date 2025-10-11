## Alur Kerja

### Node

| Step | Uraian                                         | Keterangan                                     |
| :--: | :--------------------------------------------- | :--------------------------------------------- |
|  1   | Booting Node                                   |                                                |
|  2   | Terima Waktu Dari Gateway (Sinkronisasi Waktu) | Bisa menggunakan CMOS                          |
|  3   | Scan Tag Bluetooth pada area                   | Scan MAC Address                               |
|  4   | Kirim data yang terscan secara berkala         | Data berupa JSON (delay selama 50ms per paket) |
|  5   |                                                |                                                |
|  6   |                                                |                                                |
|  7   |                                                |                                                |
|  8   |                                                |                                                |
|  9   |                                                |                                                |
|  10  |                                                |                                                |
|  11  |                                                |                                                |
|  12  |                                                |                                                |
|  13  |                                                |                                                |
|  14  |                                                |                                                |
|  15  |                                                |                                                |

### Transfer Data

1. MAC address tag bluetooth
2. Device ID
3. Waktu ["Y-m-d H:i:s"]

#### Format Data

```json
{
  "device_id": "1",
  "mac_address": [
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
  ],
  "scan_time": "2025-10-25 08:56:35"
}
```

- Estimasi

```markdown
Overhead (keys + brackets) : 85 byte
MAC addresses (36 × 20) : 720 byte
Koma antar MAC (35) : 35 byte
Spasi & newline (format) : ~50 byte
================================
TOTAL : ~890 byte
```
