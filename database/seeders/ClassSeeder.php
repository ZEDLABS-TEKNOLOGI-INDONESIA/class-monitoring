<?php
// ============================================
// SEEDER: ClassSeeder.php
// php artisan make:seeder ClassSeeder
// ============================================

namespace Database\Seeders;

use App\Models\Classes;
use Illuminate\Database\Seeder;

class ClassSeeder extends Seeder
{
    public function run(): void
    {
        $classes = [
            // Kelas 7 (Node 1-10)
            ['name' => '7A', 'grade' => '7', 'section' => 'A', 'node_id' => 1, 'room_number' => 'R701'],
            ['name' => '7B', 'grade' => '7', 'section' => 'B', 'node_id' => 2, 'room_number' => 'R702'],
            ['name' => '7C', 'grade' => '7', 'section' => 'C', 'node_id' => 3, 'room_number' => 'R703'],
            ['name' => '7D', 'grade' => '7', 'section' => 'D', 'node_id' => 4, 'room_number' => 'R704'],
            ['name' => '7E', 'grade' => '7', 'section' => 'E', 'node_id' => 5, 'room_number' => 'R705'],
            ['name' => '7F', 'grade' => '7', 'section' => 'F', 'node_id' => 6, 'room_number' => 'R706'],
            ['name' => '7G', 'grade' => '7', 'section' => 'G', 'node_id' => 7, 'room_number' => 'R707'],
            ['name' => '7H', 'grade' => '7', 'section' => 'H', 'node_id' => 8, 'room_number' => 'R708'],
            ['name' => '7I', 'grade' => '7', 'section' => 'I', 'node_id' => 9, 'room_number' => 'R709'],
            ['name' => '7J', 'grade' => '7', 'section' => 'J', 'node_id' => 10, 'room_number' => 'R710'],

            // Kelas 8 (Node 11-20)
            ['name' => '8A', 'grade' => '8', 'section' => 'A', 'node_id' => 11, 'room_number' => 'R801'],
            ['name' => '8B', 'grade' => '8', 'section' => 'B', 'node_id' => 12, 'room_number' => 'R802'],
            ['name' => '8C', 'grade' => '8', 'section' => 'C', 'node_id' => 13, 'room_number' => 'R803'],
            ['name' => '8D', 'grade' => '8', 'section' => 'D', 'node_id' => 14, 'room_number' => 'R804'],
            ['name' => '8E', 'grade' => '8', 'section' => 'E', 'node_id' => 15, 'room_number' => 'R805'],
            ['name' => '8F', 'grade' => '8', 'section' => 'F', 'node_id' => 16, 'room_number' => 'R806'],
            ['name' => '8G', 'grade' => '8', 'section' => 'G', 'node_id' => 17, 'room_number' => 'R807'],
            ['name' => '8H', 'grade' => '8', 'section' => 'H', 'node_id' => 18, 'room_number' => 'R808'],
            ['name' => '8I', 'grade' => '8', 'section' => 'I', 'node_id' => 19, 'room_number' => 'R809'],
            ['name' => '8J', 'grade' => '8', 'section' => 'J', 'node_id' => 20, 'room_number' => 'R810'],

            // Kelas 9 (Node 21-30)
            ['name' => '9A', 'grade' => '9', 'section' => 'A', 'node_id' => 21, 'room_number' => 'R901'],
            ['name' => '9B', 'grade' => '9', 'section' => 'B', 'node_id' => 22, 'room_number' => 'R902'],
            ['name' => '9C', 'grade' => '9', 'section' => 'C', 'node_id' => 23, 'room_number' => 'R903'],
            ['name' => '9D', 'grade' => '9', 'section' => 'D', 'node_id' => 24, 'room_number' => 'R904'],
            ['name' => '9E', 'grade' => '9', 'section' => 'E', 'node_id' => 25, 'room_number' => 'R905'],
            ['name' => '9F', 'grade' => '9', 'section' => 'F', 'node_id' => 26, 'room_number' => 'R906'],
            ['name' => '9G', 'grade' => '9', 'section' => 'G', 'node_id' => 27, 'room_number' => 'R907'],
            ['name' => '9H', 'grade' => '9', 'section' => 'H', 'node_id' => 28, 'room_number' => 'R908'],
            ['name' => '9I', 'grade' => '9', 'section' => 'I', 'node_id' => 29, 'room_number' => 'R909'],
            ['name' => '9J', 'grade' => '9', 'section' => 'J', 'node_id' => 30, 'room_number' => 'R910'],
        ];

        foreach ($classes as $class) {
            Classes::create($class);
        }

        $this->command->info('✅ 30 classes created (Node 1-30 mapped)');
    }
}
