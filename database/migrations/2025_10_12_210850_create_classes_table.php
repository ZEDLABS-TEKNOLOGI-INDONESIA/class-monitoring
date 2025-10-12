<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

// ============================================
// MIGRATION 3: create_classes_table
// php artisan make:migration create_classes_table
// ============================================

return new class extends Migration
{
    public function up(): void
    {
        Schema::create('classes', function (Blueprint $table) {
            $table->id();
            $table->string('name'); // e.g., "7A", "8B", "9C"
            $table->string('grade'); // e.g., "7", "8", "9"
            $table->string('section'); // e.g., "A", "B", "C"
            $table->unsignedInteger('node_id')->nullable(); // Node yang assigned
            $table->string('room_number')->nullable();
            $table->integer('capacity')->default(40);
            $table->boolean('is_active')->default(true);
            $table->timestamps();

            $table->index('node_id');
        });
    }

    public function down(): void
    {
        Schema::dropIfExists('classes');
    }
};
