import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_database/firebase_database.dart';
import 'package:firebase_messaging/firebase_messaging.dart';

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
    await logEvent(
      deviceId: 'neuroguard-001',
      type: 'SEIZURE_ALERT',
      payload: alert,
    );
  }
}
