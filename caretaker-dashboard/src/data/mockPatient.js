export const PATIENT = {
  id: "neuroguard-001",
  name: "Patient — Demo",
  deviceLabel: "NeuroGuard Wristband",
};

export const INITIAL_VITALS = {
  heartRate: 72,
  confidence: 0.08,
  gpsOk: true,
  lat: 12.9716,
  lng: 77.5946,
  battery: 82,
  bleConnected: true,
  state: "monitoring",
  lastUpdate: new Date(),
};

export function seedAlerts() {
  const now = Date.now();
  return [
    {
      id: "a1",
      type: "SEIZURE_ALERT",
      conf: 94,
      hr: 118,
      lat: 12.9716,
      lng: 77.5946,
      valid: true,
      ts: new Date(now - 86400000 * 2),
      acknowledged: true,
    },
    {
      id: "a2",
      type: "SEIZURE_ALERT",
      conf: 88,
      hr: 112,
      lat: 12.9721,
      lng: 77.5938,
      valid: true,
      ts: new Date(now - 86400000 * 5),
      acknowledged: true,
    },
  ];
}

export function randomVitals(prev) {
  const jitter = () => (Math.random() - 0.5) * 0.04;
  return {
    ...prev,
    heartRate: Math.round(68 + Math.random() * 12),
    confidence: Math.max(0.02, Math.min(0.25, prev.confidence + jitter())),
    lastUpdate: new Date(),
  };
}
