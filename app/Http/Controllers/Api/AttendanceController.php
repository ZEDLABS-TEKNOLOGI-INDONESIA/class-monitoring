<?php
// ============================================
// FILE: app/Http/Controllers/Api/AttendanceController.php
// ============================================

namespace App\Http\Controllers\Api;

use Carbon\Carbon;
use App\Models\Node;
use App\Models\Student;
use App\Models\Schedule;
use App\Models\Attendance;
use App\Models\BluetoothTag;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\DB;
use Illuminate\Support\Facades\Log;
use App\Http\Controllers\Controller;
use Illuminate\Support\Facades\Validator;

class AttendanceController extends Controller
{
    /**
     * Store attendance data from gateway
     * POST /api/gateway/attendance
     */
    public function store(Request $request)
    {
        // Validation
        $validator = Validator::make($request->all(), [
            'gateway_id' => 'required|string',
            'node_id' => 'required|integer|min:1|max:30',
            'timestamp' => 'required|integer',
            'student_count' => 'required|integer|min:0',
            'mac_addresses' => 'required|array',
            'mac_addresses.*' => 'required|string|regex:/^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$/',
            'rssi' => 'nullable|integer',
            'snr' => 'nullable|numeric',
            'cycle' => 'nullable|integer'
        ]);

        if ($validator->fails()) {
            return response()->json([
                'success' => false,
                'message' => 'Validation error',
                'errors' => $validator->errors()
            ], 422);
        }

        $data = $validator->validated();

        Log::info('Attendance data received', [
            'gateway_id' => $data['gateway_id'],
            'node_id' => $data['node_id'],
            'student_count' => $data['student_count']
        ]);

        try {
            DB::beginTransaction();

            $processedCount = 0;
            $skippedCount = 0;
            $errors = [];
            $timestamp = Carbon::createFromTimestamp($data['timestamp']);

            // Update node status
            Node::updateOrCreate(
                ['node_id' => $data['node_id']],
                [
                    'gateway_id' => $data['gateway_id'],
                    'last_seen' => $timestamp,
                    'rssi' => $data['rssi'] ?? null,
                    'snr' => $data['snr'] ?? null,
                    'status' => 'online',
                    'last_student_count' => $data['student_count'],
                    'cycle_number' => $data['cycle'] ?? null
                ]
            );

            // Process each MAC address
            foreach ($data['mac_addresses'] as $macAddress) {
                try {
                    // Step 1: Find bluetooth tag
                    $tag = BluetoothTag::where('mac_address', strtoupper($macAddress))
                                      ->where('is_active', true)
                                      ->first();

                    if (!$tag) {
                        $skippedCount++;
                        $errors[] = "MAC $macAddress not registered";
                        continue;
                    }

                    // Step 2: Get student
                    $student = Student::where('id', $tag->student_id)
                                      ->where('is_active', true)
                                      ->first();

                    if (!$student) {
                        $skippedCount++;
                        $errors[] = "Student not found for MAC $macAddress";
                        continue;
                    }

                    // Step 3: Check schedule
                    $schedule = Schedule::where('class_id', $student->class_id)
                                       ->where('node_id', $data['node_id'])
                                       ->whereDate('date', $timestamp->toDateString())
                                       ->whereTime('start_time', '<=', $timestamp->toTimeString())
                                       ->whereTime('end_time', '>=', $timestamp->toTimeString())
                                       ->first();

                    if (!$schedule) {
                        $skippedCount++;
                        $errors[] = "No schedule for student {$student->name} at this time";
                        continue;
                    }

                    // Step 4: Check duplicate (prevent double attendance)
                    $exists = Attendance::where('student_id', $student->id)
                                       ->where('schedule_id', $schedule->id)
                                       ->where('date', $timestamp->toDateString())
                                       ->exists();

                    if ($exists) {
                        $skippedCount++;
                        continue; // Already recorded, skip silently
                    }

                    // Step 5: Save attendance
                    $attendance = Attendance::create([
                        'student_id' => $student->id,
                        'schedule_id' => $schedule->id,
                        'class_id' => $student->class_id,
                        'node_id' => $data['node_id'],
                        'bluetooth_tag_id' => $tag->id,
                        'mac_address' => strtoupper($macAddress),
                        'date' => $timestamp->toDateString(),
                        'time' => $timestamp->toTimeString(),
                        'timestamp' => $timestamp,
                        'status' => 'present',
                        'rssi' => $data['rssi'] ?? null,
                        'gateway_id' => $data['gateway_id'],
                        'cycle_number' => $data['cycle'] ?? null
                    ]);

                    $processedCount++;

                    Log::info('Attendance recorded', [
                        'student' => $student->name,
                        'class' => $student->class_id,
                        'time' => $timestamp->toTimeString()
                    ]);

                } catch (\Exception $e) {
                    $errors[] = "Error processing MAC $macAddress: " . $e->getMessage();
                    Log::error('Error processing attendance', [
                        'mac' => $macAddress,
                        'error' => $e->getMessage()
                    ]);
                }
            }

            DB::commit();

            return response()->json([
                'success' => true,
                'message' => 'Attendance data processed',
                'data' => [
                    'processed' => $processedCount,
                    'skipped' => $skippedCount,
                    'total' => count($data['mac_addresses']),
                    'timestamp' => $timestamp->toDateTimeString(),
                    'errors' => $errors
                ]
            ], 200);

        } catch (\Exception $e) {
            DB::rollBack();

            Log::error('Failed to process attendance', [
                'error' => $e->getMessage(),
                'trace' => $e->getTraceAsString()
            ]);

            return response()->json([
                'success' => false,
                'message' => 'Failed to process attendance data',
                'error' => $e->getMessage()
            ], 500);
        }
    }

    /**
     * Get today's attendance
     * GET /api/attendance/today
     */
    public function today(Request $request)
    {
        $date = Carbon::today();

        $attendances = Attendance::with(['student', 'schedule', 'class'])
            ->whereDate('date', $date)
            ->orderBy('time', 'desc')
            ->paginate(50);

        $statistics = [
            'total_present' => Attendance::whereDate('date', $date)->count(),
            'total_students' => Student::where('is_active', true)->count(),
            'attendance_rate' => 0
        ];

        if ($statistics['total_students'] > 0) {
            $statistics['attendance_rate'] = round(
                ($statistics['total_present'] / $statistics['total_students']) * 100,
                2
            );
        }

        return response()->json([
            'success' => true,
            'data' => $attendances,
            'statistics' => $statistics
        ]);
    }

    /**
     * Get attendance by date
     * GET /api/attendance/date/{date}
     */
    public function byDate(Request $request, string $date)
    {
        try {
            $dateObj = Carbon::parse($date);
        } catch (\Exception $e) {
            return response()->json([
                'success' => false,
                'message' => 'Invalid date format. Use YYYY-MM-DD'
            ], 422);
        }

        $attendances = Attendance::with(['student', 'schedule', 'class'])
            ->whereDate('date', $dateObj)
            ->orderBy('time', 'desc')
            ->paginate(50);

        return response()->json([
            'success' => true,
            'data' => $attendances
        ]);
    }

    /**
     * Get attendance by student
     * GET /api/attendance/student/{studentId}
     */
    public function byStudent(Request $request, int $studentId)
    {
        $startDate = $request->input('start_date', Carbon::now()->startOfMonth());
        $endDate = $request->input('end_date', Carbon::now()->endOfMonth());

        $attendances = Attendance::with(['schedule', 'class'])
            ->where('student_id', $studentId)
            ->whereBetween('date', [$startDate, $endDate])
            ->orderBy('date', 'desc')
            ->orderBy('time', 'desc')
            ->get();

        $student = Student::find($studentId);

        return response()->json([
            'success' => true,
            'data' => [
                'student' => $student,
                'attendances' => $attendances,
                'total_days' => $attendances->groupBy('date')->count()
            ]
        ]);
    }

    /**
     * Get attendance by class
     * GET /api/attendance/class/{classId}
     */
    public function byClass(Request $request, int $classId)
    {
        $date = $request->input('date', Carbon::today()->toDateString());

        $attendances = Attendance::with(['student', 'schedule'])
            ->where('class_id', $classId)
            ->whereDate('date', $date)
            ->orderBy('time', 'desc')
            ->get();

        $totalStudents = Student::where('class_id', $classId)
                               ->where('is_active', true)
                               ->count();

        return response()->json([
            'success' => true,
            'data' => [
                'attendances' => $attendances,
                'total_present' => $attendances->count(),
                'total_students' => $totalStudents,
                'attendance_rate' => $totalStudents > 0
                    ? round(($attendances->count() / $totalStudents) * 100, 2)
                    : 0
            ]
        ]);
    }

    /**
     * Get dashboard statistics
     * GET /api/statistics/dashboard
     */
    public function dashboard(Request $request)
    {
        $today = Carbon::today();

        $stats = [
            'today' => [
                'total_present' => Attendance::whereDate('date', $today)->count(),
                'total_students' => Student::where('is_active', true)->count(),
                'total_classes' => DB::table('attendances')
                    ->whereDate('date', $today)
                    ->distinct('class_id')
                    ->count('class_id'),
                'nodes_online' => Node::where('status', 'online')
                    ->where('last_seen', '>=', Carbon::now()->subMinutes(10))
                    ->count()
            ],
            'this_week' => [
                'total_present' => Attendance::whereBetween('date', [
                    Carbon::now()->startOfWeek(),
                    Carbon::now()->endOfWeek()
                ])->count(),
                'average_attendance_rate' => 0 // Calculate later
            ],
            'nodes_status' => Node::select('status', DB::raw('count(*) as count'))
                ->groupBy('status')
                ->get(),
            'recent_activities' => Attendance::with(['student', 'class'])
                ->whereDate('date', $today)
                ->orderBy('time', 'desc')
                ->limit(10)
                ->get()
        ];

        // Calculate attendance rate
        if ($stats['today']['total_students'] > 0) {
            $stats['today']['attendance_rate'] = round(
                ($stats['today']['total_present'] / $stats['today']['total_students']) * 100,
                2
            );
        } else {
            $stats['today']['attendance_rate'] = 0;
        }

        return response()->json([
            'success' => true,
            'data' => $stats
        ]);
    }

    /**
     * Get attendance report
     * GET /api/attendance/report
     */
    public function report(Request $request)
    {
        $validator = Validator::make($request->all(), [
            'start_date' => 'required|date',
            'end_date' => 'required|date|after_or_equal:start_date',
            'class_id' => 'nullable|integer',
            'student_id' => 'nullable|integer'
        ]);

        if ($validator->fails()) {
            return response()->json([
                'success' => false,
                'errors' => $validator->errors()
            ], 422);
        }

        $query = Attendance::with(['student', 'class', 'schedule'])
            ->whereBetween('date', [
                $request->start_date,
                $request->end_date
            ]);

        if ($request->has('class_id')) {
            $query->where('class_id', $request->class_id);
        }

        if ($request->has('student_id')) {
            $query->where('student_id', $request->student_id);
        }

        $attendances = $query->orderBy('date', 'desc')
                            ->orderBy('time', 'desc')
                            ->get();

        $summary = [
            'total_records' => $attendances->count(),
            'unique_students' => $attendances->pluck('student_id')->unique()->count(),
            'date_range' => [
                'start' => $request->start_date,
                'end' => $request->end_date
            ],
            'daily_stats' => $attendances->groupBy('date')->map(function ($day) {
                return [
                    'count' => $day->count(),
                    'unique_students' => $day->pluck('student_id')->unique()->count()
                ];
            })
        ];

        return response()->json([
            'success' => true,
            'data' => [
                'attendances' => $attendances,
                'summary' => $summary
            ]
        ]);
    }
}
