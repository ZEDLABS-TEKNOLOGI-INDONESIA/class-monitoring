<?php

// ============================================
// SEEDER: NodeSeeder.php
// php artisan make:seeder NodeSeeder
// ============================================

namespace Database\Seeders;

use App\Models\Node;
use Illuminate\Database\Seeder;

class NodeSeeder extends Seeder
{
    public function run(): void
    {
        // Create 30 nodes
        for ($i = 1; $i <= 30; $i++) {
            Node::create([
                'node_id' => $i,
                'gateway_id' => 'GW001',
                'status' => 'offline',
                'last_seen' => null,
                'rssi' => null,
                'snr' => null,
                'battery' => 100,
                'cycles_completed' => 0,
                'packets_success' => 0,
                'packets_failed' => 0
            ]);
        }

        $this->command->info('✅ 30 nodes initialized');
    }
}
