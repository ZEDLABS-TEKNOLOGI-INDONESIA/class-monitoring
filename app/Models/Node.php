<?php

// ============================================
// MODEL: Node.php
// php artisan make:model Node
// ============================================

namespace App\Models;

use Carbon\Carbon;
use Illuminate\Database\Eloquent\Model;

class Node extends Model
{
    protected $fillable = [
        'node_id',
        'gateway_id',
        'status',
        'last_seen',
        'rssi',
        'snr',
        'battery',
        'last_student_count',
        'cycle_number',
        'cycles_completed',
        'packets_success',
        'packets_failed'
    ];

    protected $casts = [
        'last_seen' => 'datetime'
    ];

    public function scopeOnline($query)
    {
        return $query->where('status', 'online')
                    ->where('last_seen', '>=', Carbon::now()->subMinutes(10));
    }

    public function scopeOffline($query)
    {
        return $query->where(function($q) {
            $q->where('status', 'offline')
              ->orWhere('last_seen', '<', Carbon::now()->subMinutes(10));
        });
    }

    public function getIsOnlineAttribute(): bool
    {
        return $this->last_seen && $this->last_seen >= Carbon::now()->subMinutes(10);
    }

    public function getHealthScoreAttribute(): int
    {
        $score = 0;

        // Online status (40 points)
        if ($this->is_online) {
            $score += 40;
        }

        // RSSI quality (30 points)
        if ($this->rssi !== null) {
            if ($this->rssi > -70) {
                $score += 30;
            } elseif ($this->rssi > -85) {
                $score += 20;
            } elseif ($this->rssi > -100) {
                $score += 10;
            }
        }

        // Battery level (30 points)
        if ($this->battery !== null) {
            if ($this->battery > 50) {
                $score += 30;
            } elseif ($this->battery > 20) {
                $score += 20;
            } elseif ($this->battery > 10) {
                $score += 10;
            }
        }

        return $score;
    }
}
