function mapsUrl(lat, lng) {
  return `https://maps.google.com/?q=${lat},${lng}`;
}

export default function ActiveAlertBanner({ alert, vitalsState, onAcknowledge, onEnableNotify }) {
  const isUrgent =
    vitalsState === "detected" ||
    vitalsState === "alert_sent" ||
    (alert && !alert.acknowledged);

  if (!isUrgent) return null;

  const lat = alert?.lat ?? null;
  const lng = alert?.lng ?? null;
  const hasMap = alert?.valid && lat != null && lng != null;

  return (
    <section className="active-alert" role="alert">
      <div className="active-alert-inner">
        <div>
          <p className="active-alert-kicker">Emergency</p>
          <h2 className="active-alert-title">Seizure alert — action needed</h2>
          <p className="active-alert-meta">
            {alert
              ? `HR ${alert.hr} BPM · Confidence ${alert.conf}% · ${alert.ts.toLocaleTimeString()}`
              : `Device status: ${vitalsState.replace("_", " ")}`}
          </p>
        </div>
        <div className="active-alert-actions">
          {hasMap && (
            <a
              href={mapsUrl(lat, lng)}
              target="_blank"
              rel="noopener noreferrer"
              className="btn-maps btn-maps--urgent"
            >
              Open Google Maps →
            </a>
          )}
          {alert && !alert.acknowledged && (
            <button type="button" className="btn-ack btn-ack--light" onClick={() => onAcknowledge(alert.id)}>
              Mark reviewed
            </button>
          )}
          {typeof Notification !== "undefined" && Notification.permission === "default" && (
            <button type="button" className="btn-notify" onClick={onEnableNotify}>
              Enable phone alerts
            </button>
          )}
        </div>
      </div>
    </section>
  );
}
