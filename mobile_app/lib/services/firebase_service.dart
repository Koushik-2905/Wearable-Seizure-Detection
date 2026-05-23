import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_database/firebase_database.dart';
import 'package:firebase_messaging/firebase_messaging.dart';
import 'package:flutter/foundation.dart';

import '../constants/relay_config.dart';
import '../firebase_options.dart';
import 'relay_service.dart';

const String _rtdbUrl = 'https://neuroguard-df097-default-rtdb.firebaseio.com';
const String _deviceId = 'neuroguard-001';

/// Firebase RTDB + FCM — requires google-services.json (see README).
class FirebaseService {
  static bool _initialized = false;
  static String? _initError;

  static DatabaseReference? _vitalsRef;
  static DatabaseReference? _eventsRef;

  static Future<bool> initialize() async {
    try {
      await Firebase.initializeApp(
        options: DefaultFirebaseOptions.currentPlatform,
      );
      final db = FirebaseDatabase.instanceFor(
        app: Firebase.app(),
        databaseURL: _rtdbUrl,
      );
      _vitalsRef = db.ref('devices/$_deviceId/vitals');
      _eventsRef = db.ref('devices/$_deviceId/events');
      _initialized = true;
      _initError = null;
      if (kDebugMode) {
        debugPrint('[Firebase] Connected to $_rtdbUrl');
      }
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
    if (!_initialized || _eventsRef == null) return;
    try {
      final ref = _eventsRef!.push();
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

    if (_initialized && _vitalsRef != null) {
      try {
        await _vitalsRef!.set({
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
