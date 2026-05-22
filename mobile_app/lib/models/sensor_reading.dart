class SensorReading {
  final int state;
  final int hr;
  final double confidence;
  final bool gpsOk;
  final double? lat;
  final double? lng;

  SensorReading({
    required this.state,
    required this.hr,
    required this.confidence,
    required this.gpsOk,
    this.lat,
    this.lng,
  });

  factory SensorReading.fromJson(Map<String, dynamic> json) {
    return SensorReading(
      state: (json['state'] as num?)?.toInt() ?? 0,
      hr: (json['hr'] as num?)?.toInt() ?? 0,
      confidence: (json['conf'] as num?)?.toDouble() ?? 0,
      gpsOk: json['gpsOk'] == true,
      lat: (json['lat'] as num?)?.toDouble(),
      lng: (json['lng'] as num?)?.toDouble(),
    );
  }
}
