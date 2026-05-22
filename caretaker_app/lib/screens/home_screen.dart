import 'package:flutter/material.dart';
import 'package:intl/intl.dart';
import 'package:url_launcher/url_launcher.dart';

import '../constants/device_config.dart';
import '../models/patient_vitals.dart';
import '../models/seizure_event.dart';
import '../services/alarm_service.dart';
import '../services/firebase_caretaker_service.dart';
import '../services/notification_service.dart';
import 'alarm_screen.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  bool _firebaseOk = false;
  PatientVitals? _vitals;
  final List<SeizureEvent> _events = [];
  final Set<String> _acked = {};
  int _lastVitalsState = 0;

  @override
  void initState() {
    super.initState();
    _boot();
  }

  Future<void> _boot() async {
    final ok = await FirebaseCaretakerService.initialize();
    setState(() => _firebaseOk = ok);

    if (!ok) return;

    FirebaseCaretakerService.listen(
      onVitals: (v) {
        setState(() => _vitals = v);
        final urgent = v.state == 1 || v.state == 2;
        if (urgent && _lastVitalsState < 1) {
          _lastVitalsState = v.state;
          _triggerAlarm(
            SeizureEvent(
              id: 'vitals-${DateTime.now().millisecondsSinceEpoch}',
              conf: (v.confidence * 100).round(),
              hr: v.hr,
              lat: v.lat ?? 0,
              lng: v.lng ?? 0,
              valid: v.gpsOk && v.lat != null && v.lng != null,
              when: DateTime.now(),
              locSrc: v.locSrc,
            ),
          );
        } else if (!urgent) {
          _lastVitalsState = v.state;
        }
      },
      onNewAlert: (e) {
        setState(() => _events.insert(0, e));
        _triggerAlarm(e);
      },
    );
  }

  Future<void> _triggerAlarm(SeizureEvent e) async {
    if (AlarmService.isActive) return;

    await NotificationService.showSeizureAlert(
      hr: e.hr,
      conf: e.conf,
      hasMap: e.valid,
    );

    if (!mounted) return;
    await Navigator.of(context).push(
      MaterialPageRoute(
        builder: (_) => AlarmScreen(event: e),
        fullscreenDialog: true,
      ),
    );
  }

  @override
  void dispose() {
    FirebaseCaretakerService.dispose();
    AlarmService.stop();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    final v = _vitals;
    final fmt = DateFormat.Hm();

    return Scaffold(
      backgroundColor: const Color(0xFF050A07),
      appBar: AppBar(
        backgroundColor: Colors.black,
        title: const Text('NeuroGuard Caretaker', style: TextStyle(color: Color(0xFF00FF9D), fontSize: 16)),
      ),
      body: ListView(
        padding: const EdgeInsets.all(16),
        children: [
          _banner(
            _firebaseOk
                ? 'Live · Firebase $deviceId'
                : 'Firebase not configured — add google-services.json (see README)',
            ok: _firebaseOk,
          ),
          const SizedBox(height: 12),
          _card(
            title: 'Live vitals',
            child: v == null
                ? const Text('Waiting for patient app data…', style: TextStyle(color: Colors.white54))
                : Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      _row('Heart rate', '${v.hr} BPM'),
                      _row('Status', v.stateLabel),
                      _row('Confidence', '${(v.confidence * 100).toStringAsFixed(0)}%'),
                      _row('Location', v.gpsOk ? '${v.lat?.toStringAsFixed(5)}, ${v.lng?.toStringAsFixed(5)}' : 'Unavailable'),
                      if (v.locSrc == 'sim')
                        const Text('Approx (SIM cell)', style: TextStyle(color: Colors.amber, fontSize: 12)),
                      if (v.updatedAt != null)
                        Text('Updated ${fmt.format(v.updatedAt!)}', style: const TextStyle(color: Colors.white38, fontSize: 11)),
                      if (v.gpsOk && v.lat != null && v.lng != null) ...[
                        const SizedBox(height: 8),
                        TextButton.icon(
                          onPressed: () => launchUrl(
                            Uri.parse('https://maps.google.com/?q=${v.lat},${v.lng}'),
                            mode: LaunchMode.externalApplication,
                          ),
                          icon: const Icon(Icons.map, color: Color(0xFF00FF9D)),
                          label: const Text('Open Google Maps'),
                        ),
                      ],
                    ],
                  ),
          ),
          const SizedBox(height: 12),
          _card(
            title: 'Alert history',
            child: _events.isEmpty
                ? const Text('No alerts yet', style: TextStyle(color: Colors.white54))
                : Column(
                    children: _events.take(10).map((e) {
                      final acked = _acked.contains(e.id);
                      return ListTile(
                        contentPadding: EdgeInsets.zero,
                        title: Text(
                          'Seizure · ${fmt.format(e.when)}',
                          style: TextStyle(color: acked ? Colors.white54 : Colors.white),
                        ),
                        subtitle: Text('HR ${e.hr} · ${e.conf}%'),
                        trailing: e.valid
                            ? IconButton(
                                icon: const Icon(Icons.map, color: Color(0xFF00B4D8)),
                                onPressed: () => launchUrl(Uri.parse(e.mapsUrl), mode: LaunchMode.externalApplication),
                              )
                            : null,
                        onTap: acked ? null : () => _triggerAlarm(e),
                      );
                    }).toList(),
                  ),
          ),
          const SizedBox(height: 24),
          const Text(
            'Patient must run NeuroGuard app (BLE connected). Alerts arrive instantly when Firebase receives events.',
            style: TextStyle(color: Colors.white38, fontSize: 11),
            textAlign: TextAlign.center,
          ),
        ],
      ),
    );
  }

  Widget _banner(String text, {required bool ok}) {
    return Container(
      padding: const EdgeInsets.all(12),
      decoration: BoxDecoration(
        color: ok ? const Color(0xFF0D2818) : const Color(0xFF2A1A05),
        border: Border.all(color: ok ? const Color(0xFF00FF9D) : Colors.orange),
        borderRadius: BorderRadius.circular(8),
      ),
      child: Text(text, style: TextStyle(color: ok ? const Color(0xFF00FF9D) : Colors.orange, fontSize: 12)),
    );
  }

  Widget _card({required String title, required Widget child}) {
    return Container(
      padding: const EdgeInsets.all(16),
      decoration: BoxDecoration(
        color: const Color(0xFF0F1410),
        borderRadius: BorderRadius.circular(12),
        border: Border.all(color: const Color(0xFF1E2A22)),
      ),
      child: Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(title, style: const TextStyle(color: Color(0xFF00FF9D), fontWeight: FontWeight.w600)),
          const SizedBox(height: 12),
          child,
        ],
      ),
    );
  }

  Widget _row(String k, String v) => Padding(
        padding: const EdgeInsets.symmetric(vertical: 4),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceBetween,
          children: [
            Text(k, style: const TextStyle(color: Colors.white70)),
            Text(v, style: const TextStyle(color: Colors.white, fontWeight: FontWeight.w600)),
          ],
        ),
      );
}
