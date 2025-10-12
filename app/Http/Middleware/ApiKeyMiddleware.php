<?php
// ============================================
// FILE: app/Http/Middleware/ApiKeyMiddleware.php
// ============================================

namespace App\Http\Middleware;

use Closure;
use Illuminate\Http\Request;
use Symfony\Component\HttpFoundation\Response;

class ApiKeyMiddleware
{
    public function handle(Request $request, Closure $next): Response
    {
        $apiKey = $request->header('Authorization');

        // Remove "Bearer " prefix if exists
        if (str_starts_with($apiKey, 'Bearer ')) {
            $apiKey = substr($apiKey, 7);
        }

        // Validate API key
        if ($apiKey !== config('app.gateway_api_key')) {
            return response()->json([
                'success' => false,
                'message' => 'Invalid API key'
            ], 401);
        }

        return $next($request);
    }
}
