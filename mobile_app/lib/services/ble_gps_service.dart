import 'dart:async';
import 'dart:convert';

import 'package:flutter_blue_plus/flutter_blue_plus.dart';
import 'package:geolocator/geolocator.dart';

import '../constants/ble_uuids.dart';

typedef AlertCallback = void Function(Map<String, dynamic> data);
typedef SensorCallback = void Function(Map<String, dynamic> data);

class BleGpsService {
  BleGpsService({required this.onAlert, required this.onSensorData});

  final AlertCallback onAlert;
  final SensorCallback onSensorData;

  BluetoothDevice? _device;
  BluetoothCharacteristic? _gpsChar;
  BluetoothCharacteristic? _alertChar;
  BluetoothCharacteristic? _dataChar;
  Timer? _gpsTimer;
  bool connected = false;

  StreamSubscription<List<int>>? _alertSub;
  StreamSubscription<List<int>>? _dataSub;

  Future<void> connect(BluetoothDevice device) async {
    _device = device;
    await device.connect(autoConnect: false, timeout: const Duration(seconds: 15));
    connected = true;

    final services = await device.discoverServices();
    for (final svc in services) {
      if (!svc.uuid.toString().toLowerCase().contains("789abc")) continue;
      for (final char in svc.characteristics) {
        final u = char.uuid.toString().toLowerCase();
        if (u.contains("9c9c")) _gpsChar = char;
        if (u.contains("9abd")) _alertChar = char;
        if (u.contains("9abe")) _dataChar = char;
      }
    }

    if (_alertChar != null) {
      await _alertChar!.setNotifyValue(true);
      _alertSub = _alertChar!.lastValueStream.listen(_handleAlert);
    }

    if (_dataChar != null) {
      await _dataChar!.setNotifyValue(true);
      _dataSub = _dataChar!.lastValueStream.listen(_handleSensor);
    }

    _startGpsLoop();
  }

  void _handleAlert(List<int> value) {
    if (value.isEmpty) return;
    try {
      onAlert(jsonDecode(utf8.decode(value)) as Map<String, dynamic>);
    } catch (_) {}
  }

  void _handleSensor(List<int> value) {
    if (value.isEmpty) return;
    try {
      onSensorData(jsonDecode(utf8.decode(value)) as Map<String, dynamic>);
    } catch (_) {}
  }

  void _startGpsLoop() {
    _gpsTimer?.cancel();
    _sendGpsToEsp32();
    _gpsTimer = Timer.periodic(const Duration(seconds: 10), (_) => _sendGpsToEsp32());
  }

  Future<void> _sendGpsToEsp32() async {
    if (_gpsChar == null) return;

    LocationPermission perm = await Geolocator.checkPermission();
    if (perm == LocationPermission.denied) {
      perm = await Geolocator.requestPermission();
      if (perm == LocationPermission.denied) return;
    }

    try {
      final pos = await Geolocator.getCurrentPosition(
        locationSettings: const LocationSettings(
          accuracy: LocationAccuracy.high,
          timeLimit: Duration(seconds: 5),
        ),
      );
      final payload =
          "${pos.latitude.toStringAsFixed(6)},${pos.longitude.toStringAsFixed(6)}";
      await _gpsChar!.write(utf8.encode(payload), withoutResponse: true);
    } catch (e) {
      // GPS off or timeout — ESP32 uses cached coords
    }
  }

  Future<void> sendGpsNow() => _sendGpsToEsp32();

  Future<void> disconnect() async {
    _gpsTimer?.cancel();
    await _alertSub?.cancel();
    await _dataSub?.cancel();
    connected = false;
    await _device?.disconnect();
    _device = null;
  }
}
