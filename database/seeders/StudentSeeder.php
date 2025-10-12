<?php

// ============================================
// SEEDER: StudentSeeder.php (Example for 1 class)
// php artisan make:seeder StudentSeeder
// ============================================

namespace Database\Seeders;

use App\Models\Classes;
use App\Models\Student;
use Illuminate\Database\Seeder;

class StudentSeeder extends Seeder
{
    public function run(): void
    {
        // Get first class (7A) for demo
        $class7A = Classes::where('name', '7A')->first();

        if (!$class7A) {
            $this->command->error('Class 7A not found! Run ClassSeeder first.');
            return;
        }

        // Create 40 dummy students for class 7A
        for ($i = 1; $i <= 40; $i++) {
            Student::create([
                'nis' => '2024070' . str_pad($i, 3, '0', STR_PAD_LEFT),
                'name' => 'Siswa 7A-' . $i,
                'class_id' => $class7A->id,
                'email' => 'siswa7a' . $i . '@sekolah.com',
                'phone' => '08123456' . str_pad($i, 4, '0', STR_PAD_LEFT),
                'is_active' => true
            ]);
        }

        $this->command->info('✅ 40 students created for class 7A');
        $this->command->info('💡 Tip: Modify this seeder to create students for all classes');
    }
}
