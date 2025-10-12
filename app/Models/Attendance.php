<?php

// ============================================
// MODEL: Attendance.php
// php artisan make:model Attendance
// ============================================

namespace App\Models;

use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Relations\BelongsTo;

class Attendance extends Model
{
    protected $fillable = [
        'student_id',
        'schedule_id',
        'class_id',
        'node_id',
        'bluetooth_tag_id',
        'mac_address',
        'date',
        'time',
        'timestamp',
        'status',
        'rssi',
        'gateway_id',
        'cycle_number',
        'notes'
    ];

    protected $casts = [
        'date' => 'date',
        'time' => 'datetime:H:i:s',
        'timestamp' => 'datetime'
    ];

    public function student(): BelongsTo
    {
        return $this->belongsTo(Student::class);
    }

    public function schedule(): BelongsTo
    {
        return $this->belongsTo(Schedule::class);
    }

    public function class(): BelongsTo
    {
        return $this->belongsTo(Classes::class, 'class_id');
    }

    public function bluetoothTag(): BelongsTo
    {
        return $this->belongsTo(BluetoothTag::class);
    }

    public function scopeToday($query)
    {
        return $query->whereDate('date', today());
    }

    public function scopePresent($query)
    {
        return $query->where('status', 'present');
    }

    public function scopeByClass($query, $classId)
    {
        return $query->where('class_id', $classId);
    }
}
