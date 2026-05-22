import 'package:flutter/material.dart';

class ConnectionBanner extends StatelessWidget {
  const ConnectionBanner({
    super.key,
    required this.connected,
    required this.gpsOk,
  });

  final bool connected;
  final bool gpsOk;

  @override
  Widget build(BuildContext context) {
    if (connected && gpsOk) return const SizedBox.shrink();
    final msg = !connected
        ? 'Device disconnected — reconnect to sync GPS'
        : 'GPS stale — enable location or move closer';
    return Material(
      color: const Color(0xFFFF4D6D),
      child: Padding(
        padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
        child: Text(msg, style: const TextStyle(color: Colors.white, fontSize: 13)),
      ),
    );
  }
}
