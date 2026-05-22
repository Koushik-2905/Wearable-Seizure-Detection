class PatientVitals {
  final int state;
  final int hr;
  final double confidence;
  final bool gpsOk;
  final double? lat;
  final double? lng;
  final String locSrc;
  final DateTime? updatedAt;

  PatientVitals({
    required this.state,
    required this.hr,
    required this.confidence,
    required this.gpsOk,
    this.lat,
    this.lng,
    this.locSrc = 'none',
    this.updatedAt,
  });

  factory PatientVitals.fromMap(Map<dynamic, dynamic>? m) {
    if (m == null) {
      return PatientVitals(state: 0, hr: 0, confidence: 0, gpsOk: false);
    }
    final ts = m['ts'];
    return PatientVitals(
      state: (m['state'] as num?)?.toInt() ?? 0,
      hr: (m['hr'] as num?)?.toInt() ?? 0,
      confidence: (m['conf'] as num?)?.toDouble() ?? 0,
      gpsOk: m['gpsOk'] == true,
      lat: (m['lat'] as num?)?.toDouble(),
      lng: (m['lng'] as num?)?.toDouble(),
      locSrc: m['locSrc']?.toString() ?? 'none',
      updatedAt: ts is int
          ? DateTime.fromMillisecondsSinceEpoch(ts)
          : null,
    );
  }

  String get stateLabel {
    const labels = ['Monitoring', 'Possible seizure', 'Alert sent', 'Cancelled'];
    if (state >= 0 && state < labels.length) return labels[state];
    return 'Unknown';
  }
}
