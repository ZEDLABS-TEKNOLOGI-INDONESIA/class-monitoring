<?php

use Illuminate\Support\Facades\Schema;
use Illuminate\Database\Schema\Blueprint;
use Illuminate\Database\Migrations\Migration;

// ============================================
// MIGRATION 7: create_gateways_table
// php artisan make:migration create_gateways_table
// ============================================

return new class extends Migration
{
    public function up(): void
    {
        Schema::create('gateways', function (Blueprint $table) {
            $table->id();
            $table->string('gateway_id')->unique();
            $table->enum('status', ['online', 'offline'])->default('offline');
            $table->timestamp('last_heartbeat')->nullable();
            $table->integer('uptime')->nullable(); // seconds
            $table->boolean('wifi_connected')->default(false);
            $table->boolean('ntp_synced')->default(false);
            $table->integer('packets_received')->default(0);
            $table->integer('packets_sent')->default(0);
            $table->integer('api_success')->default(0);
            $table->integer('api_fail')->default(0);
            $table->string('ip_address')->nullable();
            $table->timestamps();

            $table->index('gateway_id');
        });
    }

    public function down(): void
    {
        Schema::dropIfExists('gateways');
    }
};
