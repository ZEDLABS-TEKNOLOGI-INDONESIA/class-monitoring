<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

// ============================================
// MIGRATION 5: create_attendances_table
// php artisan make:migration create_attendances_table
// ============================================

return new class extends Migration
{
    public function up(): void
    {
        Schema::create('attendances', function (Blueprint $table) {
            $table->id();
            $table->unsignedBigInteger('student_id');
            $table->unsignedBigInteger('schedule_id');
            $table->unsignedBigInteger('class_id');
            $table->unsignedInteger('node_id');
            $table->unsignedBigInteger('bluetooth_tag_id');
            $table->string('mac_address', 17);
            $table->date('date');
            $table->time('time');
            $table->timestamp('timestamp');
            $table->enum('status', ['present', 'late', 'absent'])->default('present');
            $table->integer('rssi')->nullable();
            $table->string('gateway_id')->nullable();
            $table->integer('cycle_number')->nullable();
            $table->text('notes')->nullable();
            $table->timestamps();

            $table->foreign('student_id')->references('id')->on('students')->onDelete('cascade');
            $table->foreign('schedule_id')->references('id')->on('schedules')->onDelete('cascade');
            $table->foreign('class_id')->references('id')->on('classes')->onDelete('cascade');
            $table->foreign('bluetooth_tag_id')->references('id')->on('bluetooth_tags')->onDelete('cascade');

            // Unique constraint: prevent duplicate attendance
            $table->unique(['student_id', 'schedule_id', 'date'], 'unique_attendance');

            $table->index(['date', 'student_id']);
            $table->index(['date', 'class_id']);
            $table->index('node_id');
        });
    }

    public function down(): void
    {
        Schema::dropIfExists('attendances');
    }
};
