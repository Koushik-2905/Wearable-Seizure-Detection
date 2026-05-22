function formatTime(date) {
  return date.toLocaleString(undefined, {
    month: "short",
    day: "numeric",
    hour: "2-digit",
    minute: "2-digit",
  });
}

export default function AlertList({ alerts, onAcknowledge }) {
  return (
    <section className="card alerts-card">
      <div className="card-head">
        <h2>⚡ Alert history</h2>
        <span className="count">{alerts.length} events</span>
      </div>
      {alerts.length === 0 ? (
        <p className="empty">No alerts recorded yet.</p>
      ) : (
        <ul className="alert-list">
          {alerts.map((a) => (
            <li key={a.id} className={a.acknowledged ? "ack" : "active"}>
              <div className="alert-top">
                <span className="alert-type">Seizure detected</span>
                <time>{formatTime(a.ts)}</time>
              </div>
              <div className="alert-stats">
                <span>HR {a.hr} BPM</span>
                <span>Conf {a.conf}%</span>
                {a.valid && (
                  <a
                    href={`https://maps.google.com/?q=${a.lat},${a.lng}`}
                    target="_blank"
                    rel="noopener noreferrer"
                    className="btn-maps-inline"
                  >
                    Google Maps →
                  </a>
                )}
              </div>
              {!a.acknowledged && (
                <button
                  type="button"
                  className="btn-ack"
                  onClick={() => onAcknowledge(a.id)}
                >
                  Mark reviewed
                </button>
              )}
              {a.acknowledged && <span className="ack-label">Reviewed</span>}
            </li>
          ))}
        </ul>
      )}
    </section>
  );
}
