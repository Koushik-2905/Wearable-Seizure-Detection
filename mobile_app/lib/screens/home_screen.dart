import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:google_maps_flutter/google_maps_flutter.dart';

import '../models/sensor_reading.dart';
import '../services/ble_gps_service.dart';
import '../services/firebase_service.dart';
import '../widgets/connection_banner.dart';
import '../widgets/seizure_alert_dialog.dart';
import '../widgets/sensor_chart.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key, required this.device});

  final BluetoothDevice device;

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  late BleGpsService _ble;
  SensorReading? _reading;
  final List<double> _confHistory = [];
  bool _connecting = true;
  String? _status;

  @override
  void initState() {
    super.initState();
    _ble = BleGpsService(
      onAlert: _onAlert,
      onSensorData: _onSensor,
    );
    _connect();
  }

  Future<void> _connect() async {
    try {
      await _ble.connect(widget.device);
      setState(() {
        _connecting = false;
        _status = 'Connected — GPS syncing every 10s';
      });
    } catch (e) {
      setState(() {
        _connecting = false;
        _status = 'Connection failed: $e';
      });
    }
  }

  void _onAlert(Map<String, dynamic> data) {
    if (!mounted) return;
    FirebaseService.logSeizureAlert(data);
    showSeizureAlertDialog(context, data);
  }

  void _onSensor(Map<String, dynamic> data) {
    if (!mounted) return;
    FirebaseService.logVitals(data);
    final r = SensorReading.fromJson(data);
    setState(() {
      _reading = r;
      _confHistory.add(r.confidence);
      if (_confHistory.length > 60) _confHistory.removeAt(0);
    });
  }

  @override
  void dispose() {
    _ble.disconnect();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final r = _reading;
    final lat = r?.lat;
    final lng = r?.lng;
    final hasMap = lat != null && lng != null;

    return Scaffold(
      backgroundColor: const Color(0xFF050A07),
      appBar: AppBar(
        backgroundColor: Colors.black,
        title: const Text('Live Monitor', style: TextStyle(color: Color(0xFF00FF9D), fontSize: 16)),
        leading: IconButton(
          icon: const Icon(Icons.arrow_back, color: Color(0xFF00FF9D)),
          onPressed: () => Navigator.pop(context),
        ),
        actions: [
          IconButton(
            icon: const Icon(Icons.my_location, color: Color(0xFF00FF9D)),
            onPressed: () => _ble.sendGpsNow(),
            tooltip: 'Send GPS now',
          ),
        ],
      ),
      body: Column(
        crossAxisAlignment: CrossAxisAlignment.stretch,
        children: [
          ConnectionBanner(connected: _ble.connected, gpsOk: r?.gpsOk ?? false),
          if (_connecting)
            const Expanded(
              child: Center(
                child: CircularProgressIndicator(color: Color(0xFF00FF9D)),
              ),
            )
          else ...[
            Padding(
              padding: const EdgeInsets.all(16),
              child: Text(_status ?? '', style: const TextStyle(color: Colors.white54, fontSize: 12)),
            ),
            _metricRow('Heart Rate', '${r?.hr ?? '--'} BPM'),
            _metricRow('Confidence', '${((r?.confidence ?? 0) * 100).toStringAsFixed(0)}%'),
            _metricRow('GPS', r?.gpsOk == true ? '${lat?.toStringAsFixed(5)}, ${lng?.toStringAsFixed(5)}' : 'Waiting…'),
            const Padding(
              padding: EdgeInsets.fromLTRB(16, 8, 16, 0),
              child: Text('Confidence (live)', style: TextStyle(color: Color(0xFF00FF9D), fontSize: 12)),
            ),
            Padding(padding: const EdgeInsets.all(8), child: SensorChart(confidenceHistory: _confHistory)),
            Expanded(
              child: hasMap
                  ? GoogleMap(
                      initialCameraPosition: CameraPosition(target: LatLng(lat, lng), zoom: 16),
                      markers: {
                        Marker(
                          markerId: const MarkerId('neuroguard'),
                          position: LatLng(lat, lng),
                        ),
                      },
                      myLocationEnabled: true,
                    )
                  : const Center(
                      child: Text('Map appears when GPS syncs', style: TextStyle(color: Colors.white38)),
                    ),
            ),
          ],
        ],
      ),
    );
  }

  Widget _metricRow(String label, String value) {
    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 6),
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceBetween,
        children: [
          Text(label, style: const TextStyle(color: Colors.white70)),
          Text(value, style: const TextStyle(color: Color(0xFFE8F5E8), fontWeight: FontWeight.w600)),
        ],
      ),
    );
  }
}
