<?php

// ============================================
// MODEL: BluetoothTag.php
// php artisan make:model BluetoothTag
// ============================================

namespace App\Models;

use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Relations\BelongsTo;

class BluetoothTag extends Model
{
    protected $fillable = [
        'mac_address',
        'student_id',
        'device_name',
        'is_active',
        'registered_at'
    ];

    protected $casts = [
        'is_active' => 'boolean',
        'registered_at' => 'datetime'
    ];

    public function student(): BelongsTo
    {
        return $this->belongsTo(Student::class);
    }

    // Mutator untuk uppercase MAC address
    public function setMacAddressAttribute($value)
    {
        $this->attributes['mac_address'] = strtoupper($value);
    }

    public function scopeActive($query)
    {
        return $query->where('is_active', true);
    }
}
