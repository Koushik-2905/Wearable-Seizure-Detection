import 'package:flutter/material.dart';

/// Shown only when BLE link is down or firmware setup is wrong — no relay/GPS hints.
class ConnectionBanner extends StatelessWidget {
  const ConnectionBanner({
    super.key,
    required this.connected,
    this.setupWarning,
  });

  final bool connected;
  final String? setupWarning;

  @override
  Widget build(BuildContext context) {
    if (connected && setupWarning == null) {
      return const SizedBox.shrink();
    }

    final msg = setupWarning ??
        'Wearable disconnected — reconnect to NeuroGuard from the scan screen';

    return Material(
      color: const Color(0xFFFF4D6D),
      child: Padding(
        padding: const EdgeInsets.symmetric(horizontal: 16, vertical: 8),
        child: Text(msg, style: const TextStyle(color: Colors.white, fontSize: 12)),
      ),
    );
  }
}
