<?php
// ============================================
// FILE: app/Http/Controllers/Api/NodeController.php
// ============================================

namespace App\Http\Controllers\Api;

use Carbon\Carbon;
use App\Models\Node;
use Illuminate\Http\Request;
use App\Http\Controllers\Controller;

class NodeController extends Controller
{
    /**
     * Get all nodes
     * GET /api/nodes
     */
    public function index(Request $request)
    {
        $nodes = Node::orderBy('node_id')->get();

        return response()->json([
            'success' => true,
            'data' => $nodes
        ]);
    }

    /**
     * Get single node
     * GET /api/nodes/{nodeId}
     */
    public function show(int $nodeId)
    {
        $node = Node::where('node_id', $nodeId)->first();

        if (!$node) {
            return response()->json([
                'success' => false,
                'message' => 'Node not found'
            ], 404);
        }

        return response()->json([
            'success' => true,
            'data' => $node
        ]);
    }

    /**
     * Update node status
     * POST /api/node/{nodeId}/status
     */
    public function updateStatus(Request $request, int $nodeId)
    {
        $validated = $request->validate([
            'status' => 'required|string|in:online,offline,error',
            'rssi' => 'nullable|integer',
            'snr' => 'nullable|numeric',
            'battery' => 'nullable|integer|min:0|max:100',
            'cycles_completed' => 'nullable|integer'
        ]);

        $node = Node::updateOrCreate(
            ['node_id' => $nodeId],
            array_merge($validated, [
                'last_seen' => Carbon::now()
            ])
        );

        return response()->json([
            'success' => true,
            'data' => $node
        ]);
    }

    /**
     * Get node health
     * GET /api/node/{nodeId}/health
     */
    public function health(int $nodeId)
    {
        $node = Node::where('node_id', $nodeId)->first();

        if (!$node) {
            return response()->json([
                'success' => false,
                'message' => 'Node not found'
            ], 404);
        }

        $isOnline = $node->last_seen >= Carbon::now()->subMinutes(10);

        $health = [
            'node_id' => $node->node_id,
            'status' => $node->status,
            'is_online' => $isOnline,
            'last_seen' => $node->last_seen,
            'last_seen_human' => $node->last_seen?->diffForHumans(),
            'rssi' => $node->rssi,
            'snr' => $node->snr,
            'battery' => $node->battery,
            'health_score' => $this->calculateHealthScore($node, $isOnline)
        ];

        return response()->json([
            'success' => true,
            'data' => $health
        ]);
    }

    /**
     * Get node statistics
     * GET /api/statistics/nodes
     */
    public function statistics(Request $request)
    {
        $stats = [
            'total_nodes' => Node::count(),
            'online_nodes' => Node::where('status', 'online')
                ->where('last_seen', '>=', Carbon::now()->subMinutes(10))
                ->count(),
            'offline_nodes' => Node::where('last_seen', '<', Carbon::now()->subMinutes(10))
                ->count(),
            'average_rssi' => Node::where('rssi', '!=', null)->avg('rssi'),
            'nodes_with_low_battery' => Node::where('battery', '<', 20)->count(),
            'nodes_detail' => Node::select('node_id', 'status', 'rssi', 'battery', 'last_seen')
                ->orderBy('node_id')
                ->get()
        ];

        return response()->json([
            'success' => true,
            'data' => $stats
        ]);
    }

    /**
     * Calculate health score (0-100)
     */
    private function calculateHealthScore($node, $isOnline): int
    {
        $score = 0;

        // Online status (40 points)
        if ($isOnline) {
            $score += 40;
        }

        // RSSI quality (30 points)
        if ($node->rssi !== null) {
            if ($node->rssi > -70) {
                $score += 30;
            } elseif ($node->rssi > -85) {
                $score += 20;
            } elseif ($node->rssi > -100) {
                $score += 10;
            }
        }

        // Battery level (30 points)
        if ($node->battery !== null) {
            if ($node->battery > 50) {
                $score += 30;
            } elseif ($node->battery > 20) {
                $score += 20;
            } elseif ($node->battery > 10) {
                $score += 10;
            }
        }

        return $score;
    }
}
