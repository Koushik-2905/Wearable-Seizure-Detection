import 'package:flutter/material.dart';

class ConnectionBanner extends StatelessWidget {
  const ConnectionBanner({
    super.key,
    required this.connected,
    required this.gpsOk,
    this.bleReceiving = true,
    this.setupWarning,
    this.cloudLabel,
  });

  final bool connected;
  final bool gpsOk;
  final bool bleReceiving;
  final String? setupWarning;
  final String? cloudLabel;

  @override
  Widget build(BuildContext context) {
    final issues = <String>[];
    if (setupWarning != null) issues.add(setupWarning!);
    if (!connected) {
      issues.add('Wearable disconnected — open app and reconnect to NeuroGuard');
    } else if (!bleReceiving) {
      issues.add(
          'No live data from ESP32 — check BLE link (Serial: "Phone connected")');
    } else if (!gpsOk) {
      issues.add('GPS stale — enable location on this phone');
    }
    if (cloudLabel != null && cloudLabel!.contains('relay')) {
      issues.add('Caretaker PC: run dashboard + scripts/set-relay-ip.ps1, same Wi‑Fi');
    }

    if (issues.isEmpty) return const SizedBox.shrink();

    return Material(
      color: const Color(0xFFB8860B),
      child: Padding(
        padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: issues
              .map((m) => Text(m, style: const TextStyle(color: Colors.white, fontSize: 12)))
              .toList(),
        ),
      ),
    );
  }
}
