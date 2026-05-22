import 'package:flutter_local_notifications/flutter_local_notifications.dart';

class NotificationService {
  static final FlutterLocalNotificationsPlugin _plugin =
      FlutterLocalNotificationsPlugin();
  static bool _ready = false;

  static Future<void> init() async {
    const android = AndroidInitializationSettings('@mipmap/ic_launcher');
    const ios = DarwinInitializationSettings();
    await _plugin.initialize(
      const InitializationSettings(android: android, iOS: ios),
    );

    const channel = AndroidNotificationChannel(
      'seizure_alerts',
      'Seizure Alerts',
      description: 'Emergency seizure notifications',
      importance: Importance.max,
      playSound: true,
      enableVibration: true,
    );

    await _plugin
        .resolvePlatformSpecificImplementation<
            AndroidFlutterLocalNotificationsPlugin>()
        ?.createNotificationChannel(channel);

    _ready = true;
  }

  static Future<void> showSeizureAlert({
    required int hr,
    required int conf,
    required bool hasMap,
  }) async {
    if (!_ready) return;

    const android = AndroidNotificationDetails(
      'seizure_alerts',
      'Seizure Alerts',
      channelDescription: 'Emergency seizure notifications',
      importance: Importance.max,
      priority: Priority.max,
      fullScreenIntent: true,
      category: AndroidNotificationCategory.alarm,
      visibility: NotificationVisibility.public,
      playSound: true,
      enableVibration: true,
      ongoing: true,
    );

    const ios = DarwinNotificationDetails(
      presentAlert: true,
      presentSound: true,
      presentBadge: true,
      interruptionLevel: InterruptionLevel.critical,
    );

    await _plugin.show(
      DateTime.now().millisecondsSinceEpoch ~/ 1000,
      'SEIZURE DETECTED',
      'HR $hr BPM · $conf% confidence${hasMap ? ' · Tap for map' : ''}',
      const NotificationDetails(android: android, iOS: ios),
    );
  }

  static Future<void> cancelAll() async {
    await _plugin.cancelAll();
  }
}
