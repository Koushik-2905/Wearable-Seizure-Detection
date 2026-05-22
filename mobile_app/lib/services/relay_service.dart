import 'dart:convert';

import 'package:http/http.dart' as http;

import '../constants/relay_config.dart';

/// Forwards BLE vitals to the caretaker dashboard Vite relay (no Firebase needed).
class RelayService {
  static DateTime? _lastVitalsPush;

  static Future<void> pushVitals(Map<String, dynamic> data) async {
    if (!relayEnabled) return;
    final now = DateTime.now();
    if (_lastVitalsPush != null &&
        now.difference(_lastVitalsPush!) < const Duration(seconds: 2)) {
      return;
    }
    _lastVitalsPush = now;
    try {
      await http
          .post(
            Uri.parse('$relayBaseUrl/api/vitals'),
            headers: {'Content-Type': 'application/json'},
            body: jsonEncode(data),
          )
          .timeout(const Duration(seconds: 3));
    } catch (_) {}
  }

  static Future<void> pushAlert(Map<String, dynamic> data) async {
    if (!relayEnabled) return;
    try {
      await http
          .post(
            Uri.parse('$relayBaseUrl/api/events'),
            headers: {'Content-Type': 'application/json'},
            body: jsonEncode(data),
          )
          .timeout(const Duration(seconds: 3));
    } catch (_) {}
  }
}
