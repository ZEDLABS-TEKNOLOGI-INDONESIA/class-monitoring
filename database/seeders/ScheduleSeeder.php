<?php

// ============================================
// SEEDER: ScheduleSeeder.php
// php artisan make:seeder ScheduleSeeder
// ============================================

namespace Database\Seeders;

use Carbon\Carbon;
use App\Models\Classes;
use App\Models\Schedule;
use Illuminate\Database\Seeder;

class ScheduleSeeder extends Seeder
{
    public function run(): void
    {
        $classes = Classes::all();
        $today = Carbon::today();

        $timeSlots = [
            ['start' => '07:00', 'end' => '08:30', 'subject' => 'Matematika'],
            ['start' => '08:31', 'end' => '10:00', 'subject' => 'Bahasa Indonesia'],
            ['start' => '10:15', 'end' => '11:45', 'subject' => 'IPA'],
            ['start' => '12:30', 'end' => '14:00', 'subject' => 'IPS'],
            ['start' => '14:01', 'end' => '15:30', 'subject' => 'Bahasa Inggris'],
        ];

        // Create schedule for today and next 7 days
        for ($day = 0; $day < 7; $day++) {
            $date = $today->copy()->addDays($day);

            // Skip weekends
            if ($date->isWeekend()) continue;

            foreach ($classes as $class) {
                foreach ($timeSlots as $slot) {
                    Schedule::create([
                        'class_id' => $class->id,
                        'node_id' => $class->node_id,
                        'subject' => $slot['subject'],
                        'teacher' => 'Guru ' . $slot['subject'],
                        'date' => $date,
                        'start_time' => $slot['start'],
                        'end_time' => $slot['end'],
                        'day_of_week' => $date->format('l'),
                        'is_active' => true
                    ]);
                }
            }
        }

        $this->command->info('✅ Schedules created for 7 days (weekdays only)');
    }
}
