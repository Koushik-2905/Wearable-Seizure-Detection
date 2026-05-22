import 'package:flutter/material.dart';
import 'package:url_launcher/url_launcher.dart';

import '../models/seizure_event.dart';
import '../services/alarm_service.dart';
import '../services/notification_service.dart';

class AlarmScreen extends StatefulWidget {
  const AlarmScreen({super.key, required this.event});

  final SeizureEvent event;

  @override
  State<AlarmScreen> createState() => _AlarmScreenState();
}

class _AlarmScreenState extends State<AlarmScreen> with SingleTickerProviderStateMixin {
  late final AnimationController _pulse;

  @override
  void initState() {
    super.initState();
    _pulse = AnimationController(vsync: this, duration: const Duration(milliseconds: 600))
      ..repeat(reverse: true);
    AlarmService.start();
  }

  @override
  void dispose() {
    _pulse.dispose();
    super.dispose();
  }

  Future<void> _silence() async {
    await AlarmService.stop();
    await NotificationService.cancelAll();
    if (mounted) Navigator.of(context).pop();
  }

  Future<void> _openMaps() async {
    if (!widget.event.valid) return;
    final uri = Uri.parse(widget.event.mapsUrl);
    if (await canLaunchUrl(uri)) {
      await launchUrl(uri, mode: LaunchMode.externalApplication);
    }
  }

  @override
  Widget build(BuildContext context) {
    final e = widget.event;
    return PopScope(
      canPop: false,
      child: Scaffold(
        backgroundColor: const Color(0xFF5C0000),
        body: AnimatedBuilder(
          animation: _pulse,
          builder: (context, child) {
            final t = 0.55 + _pulse.value * 0.45;
            return Container(
              decoration: BoxDecoration(
                gradient: LinearGradient(
                  colors: [
                    Color.lerp(const Color(0xFF8B0000), const Color(0xFFB91C1C), t)!,
                    const Color(0xFF450A0A),
                  ],
                  begin: Alignment.topCenter,
                  end: Alignment.bottomCenter,
                ),
              ),
              child: child,
            );
          },
          child: SafeArea(
            child: Padding(
              padding: const EdgeInsets.all(24),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.stretch,
                children: [
                  const Icon(Icons.warning_amber_rounded, size: 72, color: Colors.white),
                  const SizedBox(height: 16),
                  const Text(
                    'SEIZURE ALERT',
                    textAlign: TextAlign.center,
                    style: TextStyle(
                      color: Colors.white,
                      fontSize: 28,
                      fontWeight: FontWeight.bold,
                      letterSpacing: 2,
                    ),
                  ),
                  const SizedBox(height: 12),
                  Text(
                    'HR ${e.hr} BPM · Confidence ${e.conf}%',
                    textAlign: TextAlign.center,
                    style: const TextStyle(color: Colors.white70, fontSize: 16),
                  ),
                  if (e.locSrc == 'sim')
                    const Padding(
                      padding: EdgeInsets.only(top: 8),
                      child: Text(
                        'Approximate location (cell network)',
                        textAlign: TextAlign.center,
                        style: TextStyle(color: Colors.white54, fontSize: 12),
                      ),
                    ),
                  const Spacer(),
                  if (e.valid)
                    FilledButton.icon(
                      onPressed: _openMaps,
                      icon: const Icon(Icons.map),
                      label: const Text('Open Google Maps'),
                      style: FilledButton.styleFrom(
                        backgroundColor: const Color(0xFF00FF9D),
                        foregroundColor: Colors.black,
                        padding: const EdgeInsets.symmetric(vertical: 16),
                      ),
                    ),
                  const SizedBox(height: 12),
                  FilledButton.icon(
                    onPressed: _silence,
                    icon: const Icon(Icons.volume_off),
                    label: const Text('Stop alarm & dismiss'),
                    style: FilledButton.styleFrom(
                      backgroundColor: Colors.white,
                      foregroundColor: Colors.black,
                      padding: const EdgeInsets.symmetric(vertical: 16),
                    ),
                  ),
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }
}
