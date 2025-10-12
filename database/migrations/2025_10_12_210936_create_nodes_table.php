<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

// ============================================
// MIGRATION 6: create_nodes_table
// php artisan make:migration create_nodes_table
// ============================================

return new class extends Migration
{
    public function up(): void
    {
        Schema::create('nodes', function (Blueprint $table) {
            $table->id();
            $table->unsignedInteger('node_id')->unique(); // 1-30
            $table->string('gateway_id')->nullable();
            $table->enum('status', ['online', 'offline', 'error'])->default('offline');
            $table->timestamp('last_seen')->nullable();
            $table->integer('rssi')->nullable();
            $table->decimal('snr', 5, 2)->nullable();
            $table->integer('battery')->nullable(); // 0-100%
            $table->integer('last_student_count')->nullable();
            $table->integer('cycle_number')->nullable();
            $table->integer('cycles_completed')->default(0);
            $table->integer('packets_success')->default(0);
            $table->integer('packets_failed')->default(0);
            $table->timestamps();

            $table->index('node_id');
            $table->index('status');
        });
    }

    public function down(): void
    {
        Schema::dropIfExists('nodes');
    }
};
