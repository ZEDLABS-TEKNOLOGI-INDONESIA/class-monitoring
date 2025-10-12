<?php

// ============================================
// SEEDER: BluetoothTagSeeder.php
// php artisan make:seeder BluetoothTagSeeder
// ============================================

namespace Database\Seeders;

use Carbon\Carbon;
use App\Models\Student;
use App\Models\BluetoothTag;
use Illuminate\Database\Seeder;

class BluetoothTagSeeder extends Seeder
{
    public function run(): void
    {
        $students = Student::all();

        foreach ($students as $index => $student) {
            // Generate fake MAC address
            $mac = sprintf(
                'AA:BB:CC:%02X:%02X:%02X',
                ($index >> 16) & 0xFF,
                ($index >> 8) & 0xFF,
                $index & 0xFF
            );

            BluetoothTag::create([
                'mac_address' => $mac,
                'student_id' => $student->id,
                'device_name' => 'BLE-Tag-' . $student->nis,
                'is_active' => true,
                'registered_at' => Carbon::now()
            ]);
        }

        $this->command->info('✅ Bluetooth tags created for all students');
    }
}
