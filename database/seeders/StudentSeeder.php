<?php

// ============================================
// SEEDER: StudentSeeder.php (All 30 Classes)
// php artisan make:seeder StudentSeeder
// php artisan db:seed --class=StudentSeeder
// ============================================

namespace Database\Seeders;

use App\Models\Classes;
use App\Models\Student;
use Illuminate\Database\Seeder;

class StudentSeeder extends Seeder
{
    public function run(): void
    {
        // Konfigurasi untuk 30 kelas
        $classesConfig = [
            // KELAS 7
            ['name' => '7A', 'grade' => 7, 'students' => 35],
            ['name' => '7B', 'grade' => 7, 'students' => 38],
            ['name' => '7C', 'grade' => 7, 'students' => 40],
            ['name' => '7D', 'grade' => 7, 'students' => 36],
            ['name' => '7E', 'grade' => 7, 'students' => 39],
            ['name' => '7F', 'grade' => 7, 'students' => 37],
            ['name' => '7G', 'grade' => 7, 'students' => 40],
            ['name' => '7H', 'grade' => 7, 'students' => 38],
            ['name' => '7I', 'grade' => 7, 'students' => 35],
            ['name' => '7J', 'grade' => 7, 'students' => 40],
            // KELAS 8
            ['name' => '8A', 'grade' => 8, 'students' => 38],
            ['name' => '8B', 'grade' => 8, 'students' => 40],
            ['name' => '8C', 'grade' => 8, 'students' => 36],
            ['name' => '8D', 'grade' => 8, 'students' => 39],
            ['name' => '8E', 'grade' => 8, 'students' => 40],
            ['name' => '8F', 'grade' => 8, 'students' => 37],
            ['name' => '8G', 'grade' => 8, 'students' => 38],
            ['name' => '8H', 'grade' => 8, 'students' => 40],
            ['name' => '8I', 'grade' => 8, 'students' => 35],
            ['name' => '8J', 'grade' => 8, 'students' => 39],
            // KELAS 9
            ['name' => '9A', 'grade' => 9, 'students' => 40],
            ['name' => '9B', 'grade' => 9, 'students' => 38],
            ['name' => '9C', 'grade' => 9, 'students' => 36],
            ['name' => '9D', 'grade' => 9, 'students' => 40],
            ['name' => '9E', 'grade' => 9, 'students' => 37],
            ['name' => '9F', 'grade' => 9, 'students' => 39],
            ['name' => '9G', 'grade' => 9, 'students' => 40],
            ['name' => '9H', 'grade' => 9, 'students' => 38],
            ['name' => '9I', 'grade' => 9, 'students' => 35],
            ['name' => '9J', 'grade' => 9, 'students' => 36],
        ];

        $totalStudents = 0;
        $studentCounter = 1;

        foreach ($classesConfig as $config) {
            // Get class by name
            $class = Classes::where('name', $config['name'])->first();

            if (!$class) {
                $this->command->warn("⚠️  Class {$config['name']} not found! Skipping...");
                continue;
            }

            $gradeNumber = $config['grade'];
            $studentCount = $config['students'];
            $classNis = '2024070' . $gradeNumber . str_pad(substr($config['name'], -1), 1, '0', STR_PAD_LEFT);

            // Create students for this class
            for ($i = 1; $i <= $studentCount; $i++) {
                $nisNumber = $classNis . str_pad($i, 3, '0', STR_PAD_LEFT);

                Student::create([
                    'nis' => $nisNumber,
                    'name' => "Siswa {$config['name']}-{$i}",
                    'class_id' => $class->id,
                    'email' => 'siswa' . strtolower($config['name']) . $i . '@sekolah.com',
                    'phone' => '0812345' . str_pad($studentCounter, 4, '0', STR_PAD_LEFT),
                    'is_active' => true
                ]);

                $studentCounter++;
            }

            $totalStudents += $studentCount;
            $this->command->info("✅ {$studentCount} students created for class {$config['name']}");
        }

        $this->command->info("\n");
        $this->command->info("=" . str_repeat("=", 40));
        $this->command->info("📊 SUMMARY");
        $this->command->info("=" . str_repeat("=", 40));
        $this->command->info("Total Classes: 30 (7A-7J, 8A-8J, 9A-9J)");
        $this->command->info("Total Students: {$totalStudents}");
        $this->command->info("=" . str_repeat("=", 40) . "\n");
    }
}
