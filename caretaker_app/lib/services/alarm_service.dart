import 'dart:async';

import 'package:flutter_ringtone_player/flutter_ringtone_player.dart';
import 'package:vibration/vibration.dart';
import 'package:wakelock_plus/wakelock_plus.dart';

/// Loud system alarm + vibration until caretaker stops it.
class AlarmService {
  static bool _active = false;
  static Timer? _vibeTimer;

  static bool get isActive => _active;

  static Future<void> start() async {
    if (_active) return;
    _active = true;
    await WakelockPlus.enable();

    try {
      await FlutterRingtonePlayer().play(
        android: AndroidSounds.alarm,
        ios: IosSounds.alarm,
        looping: true,
        volume: 1.0,
      );
    } catch (_) {}

    _vibeTimer?.cancel();
    _vibeTimer = Timer.periodic(const Duration(milliseconds: 800), (_) async {
      if (!_active) return;
      if (await Vibration.hasVibrator()) {
        await Vibration.vibrate(duration: 400, amplitude: 255);
      }
    });
  }

  static Future<void> stop() async {
    _active = false;
    _vibeTimer?.cancel();
    _vibeTimer = null;
    try {
      await FlutterRingtonePlayer().stop();
    } catch (_) {}
    try {
      await Vibration.cancel();
    } catch (_) {}
    await WakelockPlus.disable();
  }
}
