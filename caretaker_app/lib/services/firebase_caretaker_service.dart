import 'dart:async';

import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_database/firebase_database.dart';
import 'package:firebase_messaging/firebase_messaging.dart';

import '../constants/device_config.dart';
import '../models/patient_vitals.dart';
import '../models/seizure_event.dart';

typedef VitalsCallback = void Function(PatientVitals vitals);
typedef AlertCallback = void Function(SeizureEvent event);

class FirebaseCaretakerService {
  static bool _initialized = false;
  static final Set<String> _seenEventIds = {};
  static StreamSubscription<DatabaseEvent>? _vitalsSub;
  static StreamSubscription<DatabaseEvent>? _eventsSub;

  static Future<bool> initialize() async {
    try {
      await Firebase.initializeApp();
      _initialized = true;
      await FirebaseMessaging.instance.requestPermission(
        alert: true,
        badge: true,
        sound: true,
        criticalAlert: true,
      );
      await FirebaseMessaging.instance.subscribeToTopic('caretaker_$deviceId');
      return true;
    } catch (e) {
      _initialized = false;
      return false;
    }
  }

  static bool get isReady => _initialized;

  static void listen({
    required VitalsCallback onVitals,
    required AlertCallback onNewAlert,
  }) {
    if (!_initialized) return;

    _vitalsSub?.cancel();
    _eventsSub?.cancel();

    final vitalsRef = FirebaseDatabase.instance.ref(vitalsPath);
    _vitalsSub = vitalsRef.onValue.listen((e) {
      onVitals(PatientVitals.fromMap(e.snapshot.value as Map<dynamic, dynamic>?));
    });

    final eventsRef = FirebaseDatabase.instance.ref(eventsPath);

    eventsRef.get().then((snap) {
      for (final child in snap.children) {
        if (child.key != null) _seenEventIds.add(child.key!);
      }
      _eventsSub = eventsRef.onChildAdded.listen((e) {
        final id = e.snapshot.key;
        final val = e.snapshot.value;
        if (id == null || val is! Map) return;
        if (_seenEventIds.contains(id)) return;
        _seenEventIds.add(id);

        final event = SeizureEvent.fromSnapshot(id, val);
        onNewAlert(event);
      });
    });
  }

  static void dispose() {
    _vitalsSub?.cancel();
    _eventsSub?.cancel();
  }
}

/// Background FCM handler (register in main before runApp).
@pragma('vm:entry-point')
Future<void> firebaseMessagingBackgroundHandler(RemoteMessage message) async {
  await Firebase.initializeApp();
}
