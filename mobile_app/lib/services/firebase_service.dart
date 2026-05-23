import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_database/firebase_database.dart';
import 'package:firebase_messaging/firebase_messaging.dart';
import 'package:flutter/foundation.dart';

import '../constants/relay_config.dart';
import 'relay_service.dart';

/// Firebase RTDB + FCM — requires google-services.json (see README).
class FirebaseService {
  static bool _initialized = false;
  static String? _initError;

  static Future<bool> initialize() async {
    try {
      await Firebase.initializeApp();
      _initialized = true;
      _initError = null;
      await FirebaseMessaging.instance.requestPermission();
      return true;
    } catch (e) {
      _initialized = false;
      _initError = e.toString();
      if (kDebugMode) {
        debugPrint('[Firebase] init failed — using LAN relay: $e');
      }
      return false;
    }
  }

  static bool get isReady => _initialized;
  static String? get initError => _initError;

  /// Human-readable cloud path for the Live Monitor UI.
  static String get cloudModeLabel {
    if (_initialized) return 'Firebase';
    if (relayEnabled) return 'LAN relay ($relayBaseUrl)';
    return 'BLE only (no cloud)';
  }

  static Future<void> logEvent({
    required String deviceId,
    required String type,
    required Map<String, dynamic> payload,
  }) async {
    if (!_initialized) return;
    try {
      final ref = FirebaseDatabase.instance.ref('devices/$deviceId/events').push();
      await ref.set({
        'type': type,
        'ts': ServerValue.timestamp,
        ...payload,
      });
    } catch (e) {
      if (kDebugMode) debugPrint('[Firebase] logEvent failed: $e');
      await RelayService.pushAlert({'type': type, ...payload});
    }
  }

  static Future<void> logSeizureAlert(Map<String, dynamic> alert) async {
    final payload = {
      ...alert,
      if (alert['valid'] == true) 'locSrc': 'phone',
    };
    if (_initialized) {
      await logEvent(
        deviceId: 'neuroguard-001',
        type: 'SEIZURE_ALERT',
        payload: payload,
      );
    } else {
      await RelayService.pushAlert({
        'type': 'SEIZURE_ALERT',
        ...payload,
      });
    }
  }

  static DateTime? _lastVitalsPush;

  static Map<String, dynamic> _vitalsPayload(Map<String, dynamic> data) => {
        'state': data['state'],
        'hr': data['hr'],
        'conf': data['conf'],
        'ml': data['ml'],
        'gpsOk': data['gpsOk'],
        'lat': data['lat'],
        'lng': data['lng'],
        if (data['locSrc'] != null) 'locSrc': data['locSrc'],
      };

  /// Push live BLE vitals for caretaker dashboard (RTDB: devices/{id}/vitals).
  static Future<void> logVitals(Map<String, dynamic> data) async {
    final now = DateTime.now();
    if (_lastVitalsPush != null &&
        now.difference(_lastVitalsPush!) < const Duration(seconds: 2)) {
      return;
    }
    _lastVitalsPush = now;

    final payload = _vitalsPayload(data);

    if (_initialized) {
      try {
        await FirebaseDatabase.instance.ref('devices/neuroguard-001/vitals').set({
          ...payload,
          'ts': ServerValue.timestamp,
        });
        return;
      } catch (e) {
        if (kDebugMode) debugPrint('[Firebase] vitals failed, trying relay: $e');
      }
    }

    await RelayService.pushVitals({
      ...payload,
      'ts': DateTime.now().millisecondsSinceEpoch,
    });
  }
}
