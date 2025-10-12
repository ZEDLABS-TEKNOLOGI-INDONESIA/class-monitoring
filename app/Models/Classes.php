<?php

// ============================================
// MODEL: Classes.php
// php artisan make:model Classes
// ============================================

namespace App\Models;

use Illuminate\Database\Eloquent\Model;
use Illuminate\Database\Eloquent\Relations\HasMany;

class Classes extends Model
{
    protected $fillable = [
        'name',
        'grade',
        'section',
        'node_id',
        'room_number',
        'capacity',
        'is_active'
    ];

    protected $casts = [
        'is_active' => 'boolean'
    ];

    public function students(): HasMany
    {
        return $this->hasMany(Student::class, 'class_id');
    }

    public function schedules(): HasMany
    {
        return $this->hasMany(Schedule::class, 'class_id');
    }

    public function attendances(): HasMany
    {
        return $this->hasMany(Attendance::class, 'class_id');
    }

    public function scopeActive($query)
    {
        return $query->where('is_active', true);
    }
}
