import 'package:flutter/material.dart';
import 'package:flutter_blue_plus/flutter_blue_plus.dart';

import '../constants/ble_uuids.dart';
import '../services/permissions_service.dart';
import 'home_screen.dart';

class ScanScreen extends StatefulWidget {
  const ScanScreen({super.key});

  @override
  State<ScanScreen> createState() => _ScanScreenState();
}

class _ScanScreenState extends State<ScanScreen> {
  List<ScanResult> _results = [];
  bool _scanning = false;
  String? _error;

  @override
  void initState() {
    super.initState();
    _startScan();
  }

  Future<void> _startScan() async {
    setState(() {
      _error = null;
      _scanning = true;
      _results = [];
    });

    final ok = await PermissionsService.ensureBleAndLocation();
    if (!ok) {
      setState(() {
        _error = 'Bluetooth and location permissions required';
        _scanning = false;
      });
      return;
    }

    await FlutterBluePlus.startScan(timeout: const Duration(seconds: 8));
    FlutterBluePlus.scanResults.listen((list) {
      if (!mounted) return;
      setState(() {
        _results = list
            .where((r) =>
                r.device.platformName.contains(BleUuids.deviceName) ||
                r.advertisementData.advName.contains(BleUuids.deviceName))
            .toList();
      });
    });

    await Future.delayed(const Duration(seconds: 8));
    await FlutterBluePlus.stopScan();
    if (mounted) setState(() => _scanning = false);
  }

  Future<void> _connect(BluetoothDevice device) async {
    if (!mounted) return;
    await Navigator.push(
      context,
      MaterialPageRoute(builder: (_) => HomeScreen(device: device)),
    );
    _startScan();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFF050A07),
      appBar: AppBar(
        backgroundColor: Colors.black,
        title: const Text('NeuroGuard', style: TextStyle(color: Color(0xFF00FF9D))),
        actions: [
          IconButton(
            icon: _scanning
                ? const SizedBox(
                    width: 20,
                    height: 20,
                    child: CircularProgressIndicator(strokeWidth: 2, color: Color(0xFF00FF9D)),
                  )
                : const Icon(Icons.refresh, color: Color(0xFF00FF9D)),
            onPressed: _scanning ? null : _startScan,
          ),
        ],
      ),
      body: Column(
        children: [
          if (_error != null)
            Padding(
              padding: const EdgeInsets.all(16),
              child: Text(_error!, style: const TextStyle(color: Color(0xFFFF4D6D))),
            ),
          Expanded(
            child: _results.isEmpty
                ? Center(
                    child: Text(
                      _scanning ? 'Scanning for NeuroGuard…' : 'No device found. Power on ESP32.',
                      style: const TextStyle(color: Colors.white54),
                    ),
                  )
                : ListView.builder(
                    itemCount: _results.length,
                    itemBuilder: (_, i) {
                      final r = _results[i];
                      return ListTile(
                        title: Text(
                          r.device.platformName.isNotEmpty
                              ? r.device.platformName
                              : BleUuids.deviceName,
                          style: const TextStyle(color: Colors.white),
                        ),
                        subtitle: Text(r.device.remoteId.str, style: const TextStyle(color: Colors.white38)),
                        trailing: const Icon(Icons.chevron_right, color: Color(0xFF00FF9D)),
                        onTap: () => _connect(r.device),
                      );
                    },
                  ),
          ),
        ],
      ),
    );
  }
}
