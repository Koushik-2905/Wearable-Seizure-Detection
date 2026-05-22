import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_database/firebase_database.dart';
import 'package:firebase_messaging/firebase_messaging.dart';

import 'relay_service.dart';

/// Firebase RTDB + FCM — enable after adding google-services.json / GoogleService-Info.plist
class FirebaseService {
  static bool _initialized = false;

  static Future<bool> initialize() async {
    try {
      await Firebase.initializeApp();
      _initialized = true;
      await FirebaseMessaging.instance.requestPermission();
      return true;
    } catch (_) {
      _initialized = false;
      return false;
    }
  }

  static bool get isReady => _initialized;

  static Future<void> logEvent({
    required String deviceId,
    required String type,
    required Map<String, dynamic> payload,
  }) async {
    if (!_initialized) return;
    final ref = FirebaseDatabase.instance.ref('devices/$deviceId/events').push();
    await ref.set({
      'type': type,
      'ts': ServerValue.timestamp,
      ...payload,
    });
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

  /// Push live BLE vitals for caretaker dashboard (RTDB: devices/{id}/vitals).
  static Future<void> logVitals(Map<String, dynamic> data) async {
    if (!_initialized) {
      await RelayService.pushVitals(data);
      return;
    }
    final now = DateTime.now();
    if (_lastVitalsPush != null &&
        now.difference(_lastVitalsPush!) < const Duration(seconds: 2)) {
      return;
    }
    _lastVitalsPush = now;
    await FirebaseDatabase.instance.ref('devices/neuroguard-001/vitals').set({
      'state': data['state'],
      'hr': data['hr'],
      'conf': data['conf'],
      'ml': data['ml'],
      'gpsOk': data['gpsOk'],
      'lat': data['lat'],
      'lng': data['lng'],
      'ts': ServerValue.timestamp,
    });
  }
}
