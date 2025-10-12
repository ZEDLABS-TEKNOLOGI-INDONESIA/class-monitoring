<?php

// ============================================
// MODEL: Gateway.php
// php artisan make:model Gateway
// ============================================

namespace App\Models;

use Illuminate\Database\Eloquent\Model;
use Carbon\Carbon;

class Gateway extends Model
{
    protected $fillable = [
        'gateway_id',
        'status',
        'last_heartbeat',
        'uptime',
        'wifi_connected',
        'ntp_synced',
        'packets_received',
        'packets_sent',
        'api_success',
        'api_fail',
        'ip_address'
    ];

    protected $casts = [
        'last_heartbeat' => 'datetime',
        'wifi_connected' => 'boolean',
        'ntp_synced' => 'boolean'
    ];

    public function scopeOnline($query)
    {
        return $query->where('last_heartbeat', '>=', Carbon::now()->subMinutes(5));
    }

    public function getIsOnlineAttribute(): bool
    {
        return $this->last_heartbeat && $this->last_heartbeat >= Carbon::now()->subMinutes(5);
    }

    public function getApiSuccessRateAttribute(): float
    {
        $total = $this->api_success + $this->api_fail;
        return $total > 0 ? round(($this->api_success / $total) * 100, 2) : 0;
    }
}
