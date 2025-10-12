<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

// ============================================
// MIGRATION 2: create_bluetooth_tags_table
// php artisan make:migration create_bluetooth_tags_table
// ============================================

return new class extends Migration
{
    public function up(): void
    {
        Schema::create('bluetooth_tags', function (Blueprint $table) {
            $table->id();
            $table->string('mac_address', 17)->unique(); // Format: AA:BB:CC:DD:EE:FF
            $table->unsignedBigInteger('student_id');
            $table->string('device_name')->nullable();
            $table->boolean('is_active')->default(true);
            $table->timestamp('registered_at');
            $table->timestamps();

            $table->foreign('student_id')->references('id')->on('students')->onDelete('cascade');
            $table->index('mac_address');
            $table->index('student_id');
        });
    }

    public function down(): void
    {
        Schema::dropIfExists('bluetooth_tags');
    }
};
