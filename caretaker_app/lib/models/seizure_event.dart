class SeizureEvent {
  final String id;
  final int conf;
  final int hr;
  final double lat;
  final double lng;
  final bool valid;
  final DateTime when;
  final String locSrc;

  SeizureEvent({
    required this.id,
    required this.conf,
    required this.hr,
    required this.lat,
    required this.lng,
    required this.valid,
    required this.when,
    this.locSrc = 'none',
  });

  factory SeizureEvent.fromSnapshot(String id, Map<dynamic, dynamic> m) {
    final ts = m['ts'];
    DateTime when = DateTime.now();
    if (ts is int) when = DateTime.fromMillisecondsSinceEpoch(ts);

    return SeizureEvent(
      id: id,
      conf: (m['conf'] as num?)?.toInt() ?? 0,
      hr: (m['hr'] as num?)?.toInt() ?? 0,
      lat: (m['lat'] as num?)?.toDouble() ?? 0,
      lng: (m['lng'] as num?)?.toDouble() ?? 0,
      valid: m['valid'] == true,
      when: when,
      locSrc: m['locSrc']?.toString() ?? 'none',
    );
  }

  String get mapsUrl => 'https://maps.google.com/?q=$lat,$lng';
}
