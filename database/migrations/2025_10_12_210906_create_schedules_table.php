<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

// ============================================
// MIGRATION 4: create_schedules_table
// php artisan make:migration create_schedules_table
// ============================================

return new class extends Migration
{
    public function up(): void
    {
        Schema::create('schedules', function (Blueprint $table) {
            $table->id();
            $table->unsignedBigInteger('class_id');
            $table->unsignedInteger('node_id');
            $table->string('subject')->nullable(); // Mata pelajaran
            $table->string('teacher')->nullable();
            $table->date('date');
            $table->time('start_time');
            $table->time('end_time');
            $table->string('day_of_week'); // Monday, Tuesday, dst
            $table->boolean('is_active')->default(true);
            $table->timestamps();

            $table->foreign('class_id')->references('id')->on('classes')->onDelete('cascade');
            $table->index(['class_id', 'date', 'start_time']);
            $table->index('node_id');
        });
    }

    public function down(): void
    {
        Schema::dropIfExists('schedules');
    }
};
