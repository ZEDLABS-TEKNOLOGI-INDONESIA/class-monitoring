<?php
// ============================================
// FILE: app/Http/Controllers/Api/GatewayController.php
// ============================================

namespace App\Http\Controllers\Api;

use App\Http\Controllers\Controller;
use App\Models\Gateway;
use Illuminate\Http\Request;
use Carbon\Carbon;

class GatewayController extends Controller
{
    /**
     * Gateway heartbeat
     * POST /api/gateway/heartbeat
     */
    public function heartbeat(Request $request)
    {
        $validated = $request->validate([
            'gateway_id' => 'required|string',
            'uptime' => 'nullable|integer',
            'wifi_connected' => 'nullable|boolean',
            'ntp_synced' => 'nullable|boolean',
            'packets_received' => 'nullable|integer',
            'packets_sent' => 'nullable|integer',
            'api_success' => 'nullable|integer',
            'api_fail' => 'nullable|integer'
        ]);

        Gateway::updateOrCreate(
            ['gateway_id' => $validated['gateway_id']],
            array_merge($validated, [
                'last_heartbeat' => Carbon::now(),
                'status' => 'online'
            ])
        );

        return response()->json([
            'success' => true,
            'message' => 'Heartbeat received',
            'timestamp' => Carbon::now()->timestamp
        ]);
    }

    /**
     * Get gateway status
     * GET /api/gateway/status
     */
    public function status(Request $request)
    {
        $gatewayId = $request->input('gateway_id');

        if ($gatewayId) {
            $gateway = Gateway::where('gateway_id', $gatewayId)->first();
            return response()->json([
                'success' => true,
                'data' => $gateway
            ]);
        }

        $gateways = Gateway::all();
        return response()->json([
            'success' => true,
            'data' => $gateways
        ]);
    }
}
