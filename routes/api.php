<?php

// ============================================
// FILE: routes/api.php
// ============================================

use Illuminate\Support\Facades\Route;
use App\Http\Controllers\Api\AttendanceController;
use App\Http\Controllers\Api\GatewayController;
use App\Http\Controllers\Api\NodeController;

// Public routes (dengan API key di header)
Route::middleware('api.key')->group(function () {

    // Gateway routes
    Route::prefix('gateway')->group(function () {
        Route::post('/attendance', [AttendanceController::class, 'store']);
        Route::post('/heartbeat', [GatewayController::class, 'heartbeat']);
        Route::get('/status', [GatewayController::class, 'status']);
    });

    // Node routes
    Route::prefix('node')->group(function () {
        Route::post('/{nodeId}/status', [NodeController::class, 'updateStatus']);
        Route::get('/{nodeId}/health', [NodeController::class, 'health']);
    });
});

// Protected routes (untuk dashboard/admin)
Route::middleware('auth:sanctum')->group(function () {

    // Attendance data
    Route::get('/attendance/today', [AttendanceController::class, 'today']);
    Route::get('/attendance/date/{date}', [AttendanceController::class, 'byDate']);
    Route::get('/attendance/student/{studentId}', [AttendanceController::class, 'byStudent']);
    Route::get('/attendance/class/{classId}', [AttendanceController::class, 'byClass']);
    Route::get('/attendance/report', [AttendanceController::class, 'report']);

    // Statistics
    Route::get('/statistics/dashboard', [AttendanceController::class, 'dashboard']);
    Route::get('/statistics/nodes', [NodeController::class, 'statistics']);

    // Node management
    Route::get('/nodes', [NodeController::class, 'index']);
    Route::get('/nodes/{nodeId}', [NodeController::class, 'show']);
});
